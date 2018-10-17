/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     vehicle_intersection_2.c
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>
#include <fcntl.h>
#include <String>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>

#include "keyPad.h"
#include "DelayTimer.h"
#include "file_io.h"
#include "ipc_dataTypes.h"
#include <time.h>
#include "trafficTime.h"

#include "debug.h"
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <workerThread.h>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
//#define SHAWN
//#define SIMON
#define MICHAEL

#define TIMER_STRAIGHT	10
#define TIMER_TURN		5
#define TIMER_YELLOW	3

#define SEM_FLAGS O_CREAT | O_EXCL

// Use to lock mutex
#define Lock(_Mutex) {		 											\
	do { 																\
		pthread_mutex_lock(&_Mutex); 									\
	} while(0); 														\
}

// Use to unlock mutex
#define Unlock(_Mutex) { 												\
	do { 																\
		pthread_mutex_unlock(&_Mutex);									\
	} while(0); 														\
}

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum LIGHT_COLORS
{
	LED_OFF = 0,
	LED_GREEN,
	LED_YELLOW,
	LED_RED,
};

struct TRAFFIC_LIGHTS
{
	LIGHT_COLORS north_s;
	LIGHT_COLORS north_t;
	LIGHT_COLORS east_s;
	LIGHT_COLORS east_t;
	LIGHT_COLORS south_s;
	LIGHT_COLORS south_t;
	LIGHT_COLORS west_s;
	LIGHT_COLORS west_t;
};

struct TRAFFIC_SENSORS
{
	bool ns_straight;
	bool ns_turn;
	bool ns_pedestrian;
	bool ew_straight;
	bool ew_turn;
	bool ew_pedestrian;
	bool train;
};

struct traffic_data
{
	trafficLightStates current_state;
	trafficLightStates next_state;
	trafficLightStates prev_state;

	trafficLightTiming timing;
	TRAFFIC_LIGHTS lights;
	TRAFFIC_SENSORS sensors;

	bool keep_running;
	bool change_state;

	sem_t sem;
	char sem_name[10];

	int pidt;
	int chidt;
	int ndt;

	int pidc;
	int chidc;
	int ndc;
};

typedef struct
{
	int priority;
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param sch_prm;
}_thread;

// ******** Client Struct ********
struct
{
	WorkerThread client;
	pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
	_thread clientWorkThread = {0};
	_thread clientInitThread = {0};
	char *workingthreadName = "Intersection 2 Work";
	char *servicethreadName = "Intersection 2 Service";
	int living = 1;
	int serverPID = 0;
	int serverCHID = 0;
	int *server_coid = 0;
	int nodeDescriptor = 0;
	trainStationStates Train1 = trainStationStates::DEFAULT_TSS;
	_data* msg;
	_data *reply;
}client;

struct
{
	WorkerThread client;
	pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
	_thread clientWorkThread = {0};
	_thread clientInitThread = {0};
	char *workingthreadName = "Intersection 2 Work Train";
	char *servicethreadName = "Intersection 2 Service Train";
	int living = 1;
	int serverPID = 0;
	int serverCHID = 0;
	int *server_coid = 0;
	int nodeDescriptor = 0;
	trainStationStates Train1 = trainStationStates::DEFAULT_TSS;
	_data* msg;
	_data *reply;
}client2;

WorkerThread pingpong;
keyPad kp;
TRAFFIC_SENSORS _sensor;
trafficLightStates _current_state;

bool _peakhour;
bool _train_fault;


/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/
void *th_statemachine(void *Data);
void *th_sensors(void *Data);
void *th_ipc_controlhub_client(void *Data);
void *th_ipc_train_client(void *Data);

void *clientService(void *Data);
void *clientServiceTrain(void *Data);

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
int clientConnect(int serverPID,  int serverChID, int nodeDescriptor);
void clientDisconnect(int server_coid);
//void Client_Start(int prio, void *Data);
int _client(int serverPID, int serverChID, int nd, void *Data);

int clientConnectTrain(int serverPID,  int serverChID, int nodeDescriptor);
void clientDisconnectTrain(int server_coid);
//void Client_StartTrain(int prio, void *Data);
int _clientTrain(int serverPID, int serverChID, int nd, void *Data);

void threadInit(_thread *th);

void state_display(traffic_data *data);
void state_perform(traffic_data *data);
void state_transition(traffic_data *data);
void state_transition_peak(traffic_data *data);

void init_traffic_data(traffic_data *data);

void keypad_cb(char keypress);
void *work_cb(workBuf *work);
void reset_sensors(void);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	printf("TLI-2 - Main started.\n");

	pingpong.setWorkFunction(work_cb);

	_peakhour = false;
	_train_fault = false;

	// Setup Keypad
 	pthread_attr_t keyPad_attr;
	struct sched_param keyPad_param;
    pthread_attr_init(&keyPad_attr);
    pthread_attr_setschedpolicy(&keyPad_attr, SCHED_RR);
    keyPad_param.sched_priority = 20;
    pthread_attr_setschedparam (&keyPad_attr, &keyPad_param);
    pthread_attr_setinheritsched (&keyPad_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&keyPad_attr, 8000);

    // Start the Keypad Callback
	kp.registerCallback(keypad_cb);
	kp.start(&keyPad_attr);

	// Setup state machine
	traffic_data traffic;
	init_traffic_data(&traffic);
	_current_state = traffic.current_state;

	// Clear any old semaphores with the same name
	sem_unlink(traffic.sem_name);

	// Create the named Semaphore
	traffic.sem = *sem_open(traffic.sem_name, SEM_FLAGS, S_IRWXG, 1); // Group access, 1 = unlocked.

	// Setup Train Server Connection
//	traffic.ndt = read_pid_chid_FromFile( &traffic.pidt, &traffic.chidt, "/net/BBB_CimosDirect", "/fs/TrainServer.info");

	// Confirm Control Hub Server Connection
//	DEBUGF("Train Server PID = %d\tCID = %d\n", traffic.pidt, traffic.chidt);

	// Declare Threads
	pthread_t th_traffic_sm;
	pthread_t th_sensor;
//	pthread_t th_client_train;

	void *retval;

	// Create Threads
	pthread_create(&th_traffic_sm, NULL, th_statemachine, &traffic);
	pthread_create(&th_sensor, NULL, th_sensors, &traffic);
//	pthread_create(&th_client_train, NULL, th_ipc_train_client, &traffic);

	// Create Thread for control hub client
	Lock(client.Mtx);
	client.clientInitThread.priority = 10;
	client.clientWorkThread.priority = 10;
	threadInit(&client.clientInitThread);
	pthread_create(&client.clientInitThread.thread, &client.clientInitThread.attr, clientService, &traffic);

	// This needs to be a usleep(1) to start the client properly
	usleep(1);
	Unlock(client.Mtx);

	// Create Thread for train client
	Lock(client2.Mtx);
	client2.clientInitThread.priority = 10;
	client2.clientWorkThread.priority = 10;
	threadInit(&client2.clientInitThread);
	pthread_create(&client2.clientInitThread.thread, &client.clientInitThread.attr, clientServiceTrain, &traffic);

	// This needs to be a usleep(1) to start the client properly
	usleep(1);
	Unlock(client2.Mtx);

	// Join Threads
	pthread_join(th_traffic_sm, &retval);
	pthread_join(th_sensor, &retval);
//	pthread_join(th_client_train, &retval);

	// Close the named semaphore
	sem_close(&traffic.sem);

	// Destroy the named semaphore
	sem_unlink(traffic.sem_name);

	printf("TLI-2 - Main terminated.\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/
void *th_statemachine(void *Data)
{
	printf("TLI-2 - State Machine - Thread Started\n");

	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	// Default Transitions to setup intersection
	state_display( data );
	state_perform( data );
	state_transition( data );

	// Run traffic continuously
	while(data->keep_running)
	{
		// Display state data
		state_display( data );
		state_perform( data );

		// Check if a state transition is needed
		if (_peakhour)
		{
			state_transition_peak( data );
		}
		else if (data->change_state == 1)
		{
			// Change state if one of the sensors has been tripped
			state_transition( data );
		}

		if (data->prev_state == RED)
		{
			// Reset change state variable
			data->change_state = 0;
		}
	}

	printf("TLI-2 - State Machine - Thread Terminating\n");
	return EXIT_SUCCESS;
}

void *th_sensors(void *Data)
{
	printf("TLI-2 - Sensors - Thread Started\n");

	// Cast Pointer
	traffic_data *data = (traffic_data*) Data;

	// Create Timer
	DelayTimer timer(false, 1, 1000, 0, 0); 	// 1us second delay

	// Read Sensors Continuously
	while(data->keep_running)
	{
		sem_wait(&data->sem);

		// Read Sensor Data and ignore transitions to the same state
		// North South Checks
		if (_sensor.ns_turn && data->current_state != NSTG)
		{
			data->sensors.ns_turn = _sensor.ns_turn;
			data->change_state = true;
		}
		else if (_sensor.ns_straight && data->current_state != NSG)
		{
			data->sensors.ns_straight = _sensor.ns_straight;
			data->change_state = true;
		}
		else if (_sensor.ns_pedestrian && data->current_state != NSG)
		{
			data->sensors.ns_pedestrian = _sensor.ns_pedestrian;
			data->change_state = true;
		}

		// East West Checks
//		if (_sensor.ew_turn && data->current_state != EWTG)
//		{
//			data->sensors.ew_turn = _sensor.ew_turn;
//			data->change_state = true;
//		}
		if (_sensor.ew_straight && data->current_state != EWG)
		{
			data->sensors.ew_straight = _sensor.ew_straight;
			data->change_state = true;
		}
		else if (_sensor.ew_pedestrian && data->current_state != EWG)
		{
			data->sensors.ew_pedestrian = _sensor.ew_pedestrian;
			data->change_state = true;
		}

		if (_sensor.train)
		{
			data->sensors.train = _sensor.train;
			data->change_state = true;
		}

//		data->current_state = _state;

		sem_post(&data->sem);

		// Set update rate
		timer.createTimer(); // 1 us timer
	}

	printf("TLI-2 - Sensors - Thread Terminating\n");
	return EXIT_SUCCESS;
}

void *clientService(void *Data)
{
	// Cast the pointer
	traffic_data *data = (traffic_data*) Data;

	bool fileExists = false;
	int nD = -1;
	DelayTimer timout(false, 0, 3, 0, 0);
	int pid = 0;
	int chid = 0;

	Lock(client.Mtx);
	// naming thread
	pthread_setname_np(pthread_self(),client.servicethreadName);
	Unlock(client.Mtx);

	std::string fullFilePath(CONTROLHUB);
	fullFilePath.append(CONTROLHUB_SERVER);

	while(1)
	{
		while(true)
		{
			//checking if file for server exists
			do
			{
				DEBUGF("clientService->checking for file with train server details\n");

				fileExists = checkIfFileExists(fullFilePath.c_str());
				timout.createTimer();

			}while(!fileExists);

			nD = read_pid_chid_FromFile(&pid, &chid, CONTROLHUB, CONTROLHUB_SERVER);

			if (nD != 0)
			{
				break;
				DEBUGF("clientService->Server file found with a valid node descriptor\n");
			}
			timout.createTimer();
		}

		Lock(client.Mtx);
		client.serverPID = pid;
		client.serverCHID = chid;
		client.nodeDescriptor = nD;
		Unlock(client.Mtx);

		// start client service for train station
		client.clientWorkThread.priority = 10;
		threadInit(&client.clientWorkThread);

		//pthread_create(&client.clientWorkThread.thread, &client.clientWorkThread.attr, client_ex, NULL);
		_client(client.serverPID, client.serverCHID, client.nodeDescriptor, &data);
		// wait for working thread to finish
		//pthread_join(client.clientWorkThread.thread, NULL);


		//locking mutex
		Lock(client.Mtx);

		// Check if living and if node has failed.. i.e. a drop.
		if (client.living == 0)
		{
			Unlock(client.Mtx);
			return NULL;
		}
		// create a thread that is this function and then exit this thread.
	//	pthread_create(&client.clientInitThread.thread, &client.clientInitThread.attr, clientService, &data);

		// reconnection counter
		// create longer delay?
		Unlock(client.Mtx);
	}
	return NULL;
}


void *clientServiceTrain(void *Data)
{
	// Cast the pointer
	traffic_data *data = (traffic_data*) Data;

	bool fileExists = false;
	int nD = -1;
	DelayTimer timout(false, 0, 3, 0, 0);
	int pid = 0;
	int chid = 0;

	Lock(client2.Mtx);
	// naming thread
	pthread_setname_np(pthread_self(),client2.servicethreadName);
	Unlock(client2.Mtx);

	std::string fullFilePath(TRAINSTATION);
	fullFilePath.append(TRAIN_SERVER);

	while(1)
	{
		while(true)
		{
			//checking if file for server exists
			do
			{
				DEBUGF("clientService->checking for file with train server details\n");

				fileExists = checkIfFileExists(fullFilePath.c_str());
				timout.createTimer();

			}while(!fileExists);

			nD = read_pid_chid_FromFile(&pid, &chid, TRAINSTATION, TRAIN_SERVER);

			if (nD != 0)
			{
				break;
				DEBUGF("clientService->Server file found with a valid node descriptor\n");
			}
			timout.createTimer();
		}

		Lock(client2.Mtx);
		client2.serverPID = pid;
		client2.serverCHID = chid;
		client2.nodeDescriptor = nD;
		Unlock(client2.Mtx);

		// start client service for train station
		client2.clientWorkThread.priority = 10;
		threadInit(&client2.clientWorkThread);

		//pthread_create(&client.clientWorkThread.thread, &client.clientWorkThread.attr, client_ex, NULL);
		_client(client2.serverPID, client2.serverCHID, client2.nodeDescriptor, &data);
		// wait for working thread to finish
		//pthread_join(client.clientWorkThread.thread, NULL);


		//locking mutex
		Lock(client2.Mtx);

		// Check if living and if node has failed.. i.e. a drop.
		if (client2.living == 0)
		{
			Unlock(client2.Mtx);
			return NULL;
		}
		// create a thread that is this function and then exit this thread.
	//	pthread_create(&client.clientInitThread.thread, &client.clientInitThread.attr, clientService, &data);

		// reconnection counter
		// create longer delay?
		Unlock(client2.Mtx);
	}
	return NULL;
}

void *th_ipc_train_client(void *Data)
{
	printf("TLI-2 - Train Client - Thread Started\n");

	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	// Create Timer
	DelayTimer server_rate(false, 0, 1, 0, 0);

	// Print debug
	DEBUGF("\t--> Connecting to Train Server: PID: %d \tCHID %d\n", data->pidt, data->chidt);

	// Set up message passing channel
	int server_coid = ConnectAttach(data->ndt, data->pidt, data->chidt, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		printf("\n    ERROR, could not connect to server!\n\n");
		return 0;
	}

	// Confirm connection
	DEBUGF("Connection established to process with PID:%d, CHID:%d\n", data->pidt, data->chidt);

	while (data->keep_running)
	{
		// set up data packet
		bool message = false;
		bool reply_train = false;

		// Try sending the message
		if (MsgSend(server_coid, &message, sizeof(message), &reply_train, sizeof(reply_train)) == -1)
		{
			// Reply not received from server
			DEBUGF("\tError data '%d' NOT sent to server\n", message);

//			_sensor.train = true;
			_train_fault = true;

		}
		else
		{
			// Process the reply
			DEBUGF("\t-->Server reply is: '%d'\n", reply_train);

			// Update train server
			_sensor.train = reply_train;

			//
			_train_fault = false;
		}

		// Slow down the message rate
		server_rate.createTimer();
	}

	// Close the connection
	ConnectDetach(server_coid);

	printf("TLI-2 - Train Client - Thread Terminated\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/

/*** Client code ***/
int _client(int serverPID, int serverChID, int nd, void *Data)
{
	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	DEBUGF("Client running\n");
	//client_data msg;
	//_reply reply;

	_data reply;
	_data msg;

	msg.ClientID = TRAFFIC_L2;

	int server_coid;
	int index = 0;

	DEBUGF("   --> Trying to connect (server) process which has a PID: %d\n", serverPID);
	DEBUGF("   --> on channel: %d\n\n", serverChID);

	// set up message passing channel
	server_coid = ConnectAttach(nd, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("\n    ERROR, could not connect to server!\n\n");
		return EXIT_FAILURE;
	}

	DEBUGF("Connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);

	msg.hdr.type = 0x00;
	msg.hdr.subtype = 0x00;

	int done = 0;

	// Create Timer
	DelayTimer server_rate(false, 0, 1, 0, 0);

	while (!done)
	{
		// set up data packet
		msg.inter_data.currentState = _current_state;
		msg.inter_data.trainFault_int2 = _train_fault;

		DEBUGF("\tClient (ID:%d) current_state = %d\n", msg.ClientID, msg.inter_data.currentState);

		fflush(stdout);
		int error = 0;
		if (MsgSend(server_coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		{
			error = errno;
			DEBUGF("Error was: %s\n", strerror(error));
			DEBUGF(" Error data NOT sent to server\n");
			// maybe we did not get a reply from the server
			break;
		}
		else
		{
			// Process the reply
			DEBUGF("\t-->Server reply is: '%d'\n", reply.inter_data.currentState);

			// Handle state requests
			switch (reply.inter_data.currentState)
			{
			case NSG:
				_sensor.ns_straight = true;
				break;
			case NSTG:
				_sensor.ns_turn = true;
				break;
			case EWG:
				_sensor.ew_straight = true;
				break;
//			case EWTG:
//				_sensor.ew_turn = true;
			case TIMEING_UPDATE:
				printf("Timing Update Received\n");
				setTimeDate( localtime(&reply.timing.time) );
				_peakhour = checkIfpeak(NULL, &reply.timing);
			default:
				break;
			}

			// Update traffic light timing
			data->timing = reply.inter_data.lightTiming; // TODO - check if wrong struct implementation, doesn't update variable
		}

		if (_peakhour)
		{
			printf("Peak Hour ON\n");
		}

		// Slow down the message rate
		server_rate.createTimer();

	}//end while

	// Close the connection
	DEBUGF("\n Sending message to server to tell it to close the connection\n");
	ConnectDetach(server_coid);

	return EXIT_SUCCESS;
}

/* ----------------------------------------------------	*
 *	@threadInit Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void threadInit(_thread *th)
{
	// setting up thread attributes
    pthread_attr_init (&th->attr);
    pthread_attr_setschedpolicy(&th->attr, SCHED_RR);
    th->sch_prm.sched_priority = th->priority;
    pthread_attr_setschedparam (&th->attr, &th->sch_prm);
    pthread_attr_setinheritsched (&th->attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&th->attr, 8000);
}


/* ----------------------------------------------------	*
 *	@clientConnect Implementation:						*
 *	@brief:												*
 *	@return:	int										*
 * ---------------------------------------------------	*/
int clientConnect(int serverPID,  int serverChID, int nodeDescriptor)
{
    int server_coid;

    DEBUGF("clientConnect->trying to connect (server) process which has a PID: %d\n", serverPID);
    DEBUGF("clientConnect->on channel: %d\n", serverChID);

	// set up message passing channel
    server_coid = ConnectAttach(nodeDescriptor, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("clientConnect->ERROR, could not connect to server!\n\n");
        return server_coid;
	}

	DEBUGF("clientConnect->connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);
    return server_coid;
}


/* ----------------------------------------------------	*
 *	@clientDisconnect Implementation:					*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
void clientDisconnect(int server_coid)
{
    DEBUGF("\n Sending message to server to tell it to close the connection\n");
    ConnectDetach(server_coid);
    return;
}


/*** Client code ***/
int _clientTrain(int serverPID, int serverChID, int nd, void *Data)
{
	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	DEBUGF("Client train running\n");
	//client_data msg;
	//_reply reply;

	_data reply;
	_data msg;

	msg.ClientID = TRAFFIC_L1;

	int server_coid;
	int index = 0;

	DEBUGF("   --> Trying to connect (train server) process which has a PID: %d\n", serverPID);
	DEBUGF("   --> on channel: %d\n\n", serverChID);

	// set up message passing channel
	server_coid = ConnectAttach(nd, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("\n    ERROR, could not connect to train server!\n\n");
		return EXIT_FAILURE;
	}

	DEBUGF("Connection established to train process with PID:%d, Ch:%d\n", serverPID, serverChID);

	msg.hdr.type = 0x00;
	msg.hdr.subtype = 0x00;

	int done = 0;

	// Create Timer
	DelayTimer server_rate(false, 0, 1, 0, 0);

	while (!done)
	{
		// set up data packet
		bool message = false;
		bool reply_train = false;

		int error = 0;
		if (MsgSend(server_coid, &msg, sizeof(msg), &reply_train, sizeof(reply_train)) == -1)
		{
			error = errno;
			DEBUGF("Error was: %s\n", strerror(error));
			DEBUGF(" Error data NOT sent to train server\n");
			// maybe we did not get a reply from the server

			_train_fault = true;

			break;
		}
		else
		{
			// Process the reply
			_train_fault = false;
			_sensor.train = reply_train;
		}


		// Slow down the message rate
		server_rate.createTimer();

	}//end while

	// Close the connection
	DEBUGF("\n Sending message to train server to tell it to close the connection\n");
	ConnectDetach(server_coid);

	return EXIT_SUCCESS;
}


/* ----------------------------------------------------	*
 *	@clientConnect Implementation:						*
 *	@brief:												*
 *	@return:	int										*
 * ---------------------------------------------------	*/
int clientConnectTrain(int serverPID,  int serverChID, int nodeDescriptor)
{
    int server_coid;

    DEBUGF("clientConnect->trying to connect (train server) process which has a PID: %d\n", serverPID);
    DEBUGF("clientConnect->on channel: %d\n", serverChID);

	// set up message passing channel
    server_coid = ConnectAttach(nodeDescriptor, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("clientConnect->ERROR, could not connect to train server!\n\n");
        return server_coid;
	}

	DEBUGF("clientConnect->connection established to train process with PID:%d, Ch:%d\n", serverPID, serverChID);
    return server_coid;
}


/* ----------------------------------------------------	*
 *	@clientDisconnect Implementation:					*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
void clientDisconnectTrain(int server_coid)
{
    DEBUGF("\n Sending message to train server to tell it to close the connection\n");
    ConnectDetach(server_coid);
    return;
}

void state_display(traffic_data *data)
{
	sem_wait(&data->sem);

	switch(data->current_state)
	{
	case DEFAULT_TLS:
		// Initial State
		printf("Current state: DEFAULT_TLS\t\n");
		break;
	case NSG:
		// North South Green
		printf("Current state: NSG\t(North South Green)\n");
		break;
	case NSY:
		// North South Yellow
		printf("Current state: NSY\t(North South Yellow)\n");
		break;
	case NSTG:
		// North South Turn Green
		printf("Current state: NSTG\t(North South Turn Green)\n");
		break;
	case NSTY:
		// North South Turn Yellow
		printf("Current state: NSTY\t(North South Turn Yellow)\n");
		break;
	case EWG:
		// East West Green
		printf("Current state: EWG\t(East West Green Green)\n");
		break;
	case EWY:
		// East West Yellow
		printf("Current state: EWY\t(East West Yellow)\n");
		break;
//	case EWTG:
//		// East West Turn Green
//		printf("Current state: EWTG\t(East West Turn Green)\n");
//		break;
//	case EWTY:
//		// East West Turn Yellow
//		printf("Current state: EWTY\t(East West Turn Yellow)\n");
//		break;
	case RED:
		// All Red
		printf("Current state: RED\t(All Red)\n");
		break;
	case ERROR_1:
		// Error State 1
		printf("Current state: ERROR_1\n");
		break;
	case ERROR_2:
		// Error State 2
		printf("Current state: ERROR_2\n");
		break;
	case ERROR_3:
		// Error State 3
		printf("Current state: ERROR_3\n");
		break;
	case ERROR_4:
		// Error State 4
		printf("Current state: ERROR_4\n");
		break;
	default:
		// Unknown State
		printf("state_display: Unknown state\n");
	}
	fflush(stdout);
	sem_post(&data->sem);
}

void state_perform(traffic_data *data)
{
	sem_wait(&data->sem);

	// Create Timers
	DelayTimer timer_ns(false, 0, data->timing.nsStraight, 0, 0);
	DelayTimer timer_nst(false, 0, data->timing.nsTurn, 0, 0);
	DelayTimer timer_ew(false, 0, data->timing.ewStraight, 0, 0);
	DelayTimer timer_ewt(false, 0, data->timing.ewTurn, 0, 0);
	DelayTimer timer_y(false, 0, data->timing.yellow, 0, 0);

	switch(data->current_state)
	{
	case DEFAULT_TLS:
		// Initial State
		data->lights =	{
				LED_OFF, 	// N Straight
				LED_OFF, 	// N Turn
				LED_OFF,	// E Straight
				LED_OFF, 	// E Turn
				LED_OFF,	// S Straight
				LED_OFF, 	// S Turn
				LED_OFF, 	// W Straight
				LED_OFF		// W Turn
		};
		break;
	case NSG:
		// North South Green
		data->lights =	{
				LED_GREEN, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_GREEN,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		timer_ns.createTimer();
		break;
	case NSY:
		// North South Yellow
		data->lights =	{
				LED_YELLOW, // N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_YELLOW,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		timer_y.createTimer();
		break;
	case NSTG:
		// North South Turn Green
		data->lights =	{
				LED_RED, 	// N Straight
				LED_GREEN, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_GREEN, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		timer_nst.createTimer();
		break;
	case NSTY:
		// North South Turn Yellow
		data->lights =	{
				LED_RED, 	// N Straight
				LED_YELLOW, // N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_YELLOW, // S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		timer_y.createTimer();
		break;
	case EWG:
		// East West Green
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_GREEN,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_GREEN, 	// W Straight
				LED_RED		// W Turn
		};
		timer_ew.createTimer();
		break;
	case EWY:
		// East West Yellow
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_YELLOW,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_YELLOW, // W Straight
				LED_RED		// W Turn
		};
		timer_y.createTimer();
		break;
//	case EWTG:
//		// East West Turn Green
//		data->lights =	{
//				LED_RED, 	// N Straight
//				LED_RED, 	// N Turn
//				LED_RED,	// E Straight
//				LED_GREEN, 	// E Turn
//				LED_RED,	// S Straight
//				LED_RED, 	// S Turn
//				LED_RED, 	// W Straight
//				LED_GREEN	// W Turn
//		};
//		timer_ewt.createTimer();
//		break;
//	case EWTY:
//		// East West Turn Yellow
//		data->lights =	{
//				LED_RED, 	// N Straight
//				LED_RED, 	// N Turn
//				LED_RED,	// E Straight
//				LED_YELLOW, 	// E Turn
//				LED_RED,	// S Straight
//				LED_RED, 	// S Turn
//				LED_RED, 	// W Straight
//				LED_YELLOW	// W Turn
//		};
//		timer_y.createTimer();
//		break;
	case RED:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case ERROR_1:
	case ERROR_2:
	case ERROR_3:
	case ERROR_4:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		printf("state_perform: ERROR State %d", (data->current_state - 9) );
		break;
	default:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		printf("state_perform: Unknown State");
	}
	fflush(stdout);
	sem_post(&data->sem);
}

void state_transition(traffic_data *data)
{
	sem_wait(&data->sem);

	// Transition States
	data->prev_state = data->current_state;
	data->current_state = data->next_state;

	// Load Next State
	switch(data->current_state)
	{
	case DEFAULT_TLS:
		// Initial State
		data->next_state = NSG;
		break;
	case NSG:
		// North South Green
		data->next_state = NSY;
		break;
	case NSY:
		// North South Yellow
		data->next_state = RED;
		break;
	case NSTG:
		// North South Turn Green
		data->next_state = NSTY;
		break;
	case NSTY:
		// North South Turn Yellow
		data->next_state = RED;
		break;
	case EWG:
		// East West Green
		data->next_state = EWY;
		break;
	case EWY:
		// East West Yellow
		data->next_state = RED;
		break;
//	case EWTG:
//		// East West Turn Green
//		data->next_state = EWTY;
//		break;
//	case EWTY:
//		// East West Turn Yellow
//		data->next_state = RED;
//		break;
	case RED:
		// All Red State
		if (data->prev_state == NSY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = EWG;
			}
//			else if (data->sensors.ew_turn == 1)
//			{
//				// Car on EW wants to turn
//				data->next_state = EWTG;
//
//				// Reset Sensor
//				data->sensors.ew_turn = 0;
//			}
			else if (data->sensors.ew_straight == 1 || data->sensors.ew_pedestrian == 1)
			{
				data->next_state = EWG;

				// Reset Sensors
				data->sensors.ew_straight = 0;
				data->sensors.ew_pedestrian = 0;
			}
			else if (data->sensors.ns_turn == 1)
			{
				// NS Car wants to turn
				data->next_state = NSTG;

				// Reset Sensors
				data->sensors.ns_turn = 0;
			}
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
			}
		}
//		else if (data->prev_state == EWTY)
//		{
//			if (data->sensors.ew_straight == 1 || data->sensors.train)
//			{
//				// Car on EW wants to go straight, or train is approaching
//				data->next_state = EWG;
//
//				// Reset Sensor
//				data->sensors.ew_straight = 0;
//			}
//			else if (data->sensors.ns_turn == 1)
//			{
//				// NS Car wants to turn
//				data->next_state = NSTG;
//
//				// Reset sensor
//				data->sensors.ns_turn = 0;
//			}
//			else if (data->sensors.ns_straight == 1 || data->sensors.ns_pedestrian == 1)
//			{
//				// NS Car wants to go straight
//				data->next_state = NSG;
//
//				// Reset Sensor
//				data->sensors.ns_straight = 0;
//				data->sensors.ns_pedestrian = 0;
//			}
//			else if (data->sensors.ew_pedestrian == 1)
//			{
//				// Pedestraian wants to cross on EW
//				data->next_state = EWG;
//
//				// Reset sensor
//				data->sensors.ew_pedestrian = 0;
//
//			}
//			else
//			{
//				printf("\tERROR - Transition Triggered without sensor data\n");
//			}
//		}
		else if (data->prev_state == EWY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = NSTG;
			}
			else if (data->sensors.ns_turn == 1)
			{
				// NS Car wants to turn
				data->next_state = NSTG;

				// Reset sensor
				data->sensors.ns_turn = 0;
			}
			else if (data->sensors.ns_pedestrian == 1 || data->sensors.ns_straight == 1)
			{
				// Pedestrian wants to cross or NS car wants to go straight
				data->next_state = NSG;

				// Reset Sensor
				data->sensors.ns_pedestrian = 0;
				data->sensors.ns_straight = 0;
			}
//			else if (data->sensors.ew_turn == 1)
//			{
//				// EW Car wants to turn
//				data->next_state = EWTG;
//
//				// Reset Sensor
//				data->sensors.ew_turn = 0;
//			}
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
			}
		}
		else if (data->prev_state == NSTY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = EWG;
			}
			else if (data->sensors.ns_straight == 1 || data->sensors.ns_pedestrian == 1)
			{
				// NS car wants to go straight or NS pedestrian wants to cross
				data->next_state = NSG;

				// Reset Sensor
				data->sensors.ns_straight = 0;
				data->sensors.ns_pedestrian = 0;
			}
//			else if (data->sensors.ew_turn == 1)
//			{
//				// EW Car wants to turn
//				data->next_state = EWTG;
//
//				// Reset Sensor
//				data->sensors.ew_turn = 0;
//			}
			else if (data->sensors.ew_straight == 1 || data->sensors.ew_pedestrian == 1)
			{
				// EW car wants to go straight
				data->next_state = EWG;

				// Reset Sensor
				data->sensors.ew_straight = 0;
				data->sensors.ew_pedestrian = 0;
			}
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
			}
		}
		else if (data->prev_state == RED)
		{
			printf("\tERROR - Transition Triggered without sensor data\n");
		}
		else
		{
			printf("ERROR - Something really bad has happened with state transitions");
		}

		// Reset sensors after transitions have occurred
		reset_sensors();

		break;
	case ERROR_1:
	case ERROR_2:
	case ERROR_3:
	case ERROR_4:
		// Error State
		printf("state_transition: ERROR State\n");
		break;
	default:
		// Unknown State
		printf("state_transition: Unknown state\n");
	}
	fflush(stdout);

	_current_state = data->current_state;

	sem_post(&data->sem);
}

void state_transition_peak(traffic_data *data)
{
	sem_wait(&data->sem);

	// Transition States
	data->prev_state = data->current_state;
	data->current_state = data->next_state;

	// Load Next State
	switch(data->current_state)
	{
	case DEFAULT_TLS:
		// Initial State
		data->next_state = NSG;
		break;
	case NSG:
		// North South Green
		data->next_state = NSY;
		break;
	case NSY:
		// North South Yellow
		data->next_state = RED;
		break;
	case NSTG:
		// North South Turn Green
		data->next_state = NSTY;
		break;
	case NSTY:
		// North South Turn Yellow
		data->next_state = RED;
		break;
	case EWG:
		// East West Green
		data->next_state = EWY;
		break;
	case EWY:
		// East West Yellow
		data->next_state = RED;
		break;
//	case EWTG:
//		// East West Turn Green
//		data->next_state = EWTY;
//		break;
//	case EWTY:
//		// East West Turn Yellow
//		data->next_state = RED;
//		break;
	case RED:		// TODO - check logic of transitions below
		// All Red State
		if (data->prev_state == NSY)
		{
//			if (data->sensors.train == 1)
//			{
				// Train is present
				data->next_state = EWG;
//			}
//			else
//			{
//				// Car on EW wants to turn
//				data->next_state = EWTG;
//			}
		}
//		else if (data->prev_state == EWTY)
//		{
//			data->next_state = EWG;
//		}
		else if (data->prev_state == EWY)
		{
			data->next_state = NSTG;
		}
		else if (data->prev_state == NSTY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = EWG;
			}
			else
			{
				// NS car wants to go straight or NS pedestrian wants to cross
				data->next_state = NSG;
			}
		}
		else if (data->prev_state == RED)
		{
			printf("\tERROR - Transition Triggered without sensor data\n");
		}
		else
		{
			printf("ERROR - Something really bad has happened with state transitions");
		}

		// Reset sensors after transitions have occurred
		reset_sensors();

		break;
	case ERROR_1:
	case ERROR_2:
	case ERROR_3:
	case ERROR_4:
		// Error State
		printf("state_transition: ERROR State\n");
		break;
	default:
		// Unknown State
		printf("state_transition: Unknown state\n");
	}
	fflush(stdout);

	_current_state = data->current_state;

	sem_post(&data->sem);
}

void init_traffic_data(traffic_data *data)
{
	// Initialize States
	data->current_state = DEFAULT_TLS;
	data->next_state = NSG;
	data->prev_state = DEFAULT_TLS;

	// Initialize LED Lights
	data->lights =	{
			LED_OFF, 	// N Straight
			LED_OFF, 	// N Turn
			LED_OFF,	// E Straight
			LED_OFF, 	// E Turn
			LED_OFF,	// S Straight
			LED_OFF, 	// S Turn
			LED_OFF, 	// W Straight
			LED_OFF		// W Turn
	};

	// Initialize Timing
	data->timing =	{
			TIMER_STRAIGHT*2,	// NS Straight
			TIMER_TURN,		// NS Turn
			TIMER_STRAIGHT,	// EW Straight
			TIMER_TURN,		// EW Turn
			TIMER_YELLOW,	// Yellow
	};

	// Initialize Sensors
	data->sensors = {
			0,	// NS Straight
			0,	// NS Turn
			0,	// NS Pedestrian
			0,	// EW Straight
			0,	// EW Turn
			0,	// EW Pedestrian
			0	// Train
	};

	// Run state machine continuously
	data->keep_running = 1;

	data->change_state = 0;

	// Set the Semaphore name
	strcpy(data->sem_name, "sem_2");
}

void keypad_cb(char keypress)
{
	// Start Worker thread
	pingpong.doWork(&keypress, sizeof(keypress), 0);

	// Create Timer
	DelayTimer timer(false, 1, 1000, 0, 0); 	// 1us second delay
	timer.createTimer();
}

void *work_cb(workBuf *work)
{
	char data = *work->data->c_str();

	printf("\tButton pressed %c\n", data);

	switch ( data )
	{
	case '1':
		// Button 1 Pressed
		// NS Straight
		_sensor.ns_straight = true;
		break;
	case '2':
		// Button 2 Pressed
		// NS Turn
		_sensor.ns_turn = true;
		break;
	case '3':
		// Button 3 Pressed
		// NS Pedestrian
		_sensor.ns_pedestrian = true;
		break;
	case '4':
		// Button 4 Pressed
		// Do nothing
		break;
	case '5':
		// Button 5 Pressed
		// EW Straight
		_sensor.ew_straight = true;
		break;
//	case '6':
//		// Button 6 Pressed
//		// EW Turn
//		_sensor.ew_turn = true;
//		break;
	case '7':
		// Button 5 Pressed
		// EW Pedestrian
		_sensor.ew_pedestrian = true;
		break;
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
		// Do Nothing. Buttons unassigned.
		break;
	default:
		printf("ERROR - Unknown key pressed.\n");
		break;
	}

	return NULL;
}

void reset_sensors(void)
{
	_sensor.ns_straight = false;
	_sensor.ns_turn = false;
	_sensor.ns_pedestrian = false;
	_sensor.ew_straight = false;
	_sensor.ew_turn = false;
	_sensor.ew_pedestrian = false;
}
