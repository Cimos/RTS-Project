#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>
#include <string.h>
#include <unistd.h>
#include <iostream>


//#include "ipc_dataTypes.h"

#define BUF_SIZE 100
#define TRAINSTATION "/net/BBB_CimosDirect"
#define TRAIN_SERVER "/fs/TrainServer.info"


using namespace std;

typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	char data[2];     // our data
} my_data;

typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	char buf[BUF_SIZE];// Message we send back to clients to tell them the messages was processed correctly.
} my_reply;

typedef struct
{
	int PID;
	int CHID;
} file_params;

size_t read_data(FILE *fp, file_params *p)
{
	return(fread(p, sizeof(file_params), 1, fp));
}

enum clients
{
	DEFAULT_CLT = 0,
	TRAFFIC_L1,
	TRAFFIC_L2,
	TRAIN_I1,
	CONTROL_H1
};

// prototypes
int client(int serverPID, int serverCHID);

int main(int argc, char *argv[])
{
	printf("Client running\n");

	// connection data (you may need to edit this)
	int serverPID = 0;	// CHANGE THIS Value to PID of the server process
	int	serverCHID = 0;			// CHANGE THIS Value to Channel ID of the server process (typically 1)

	/*
	*   Your code goes here.
	*
	*   You need to find the process ID (PID), and a channel ID (CHID) of the server to make a connection to it...
	*
	*   The data should be read from file on the server (for example it could be located at):
	*
	*	 serverPID  (first line of file)
	*	 Channel ID (second line of file)
	*
	*   Initially you may NEED TO START THE SERVER and set the serverPID = Process ID of the RUNNING SERVER
	*
	*/
	//********************************************************************************************
	FILE *fp;
	file_params my_file;

	//open file
	//fp = fopen("/net/BBB_CimosDirect/fs/shawn_file", "r");

	string fileName = TRAINSTATION;
	fileName.append(TRAIN_SERVER);
	fp = fopen(fileName.c_str(), "r");

	//read in PID and CHID from file
	if (fp != NULL){
		while (read_data(fp, &my_file) != 0){
			printf("\nPID = %d", my_file.PID);
			printf("\nCHAN ID = %d\n\n", my_file.CHID);
			serverPID = my_file.PID;
			serverCHID = my_file.CHID;
		}

		fclose(fp);
	}

	//********************************************************************************************


	int ret = 0;
	ret = client(serverPID, serverCHID);

	printf("Main (client) Terminated....\n");
	return ret;
}

/*** Client code ***/
int client(int serverPID, int serverChID)
{
	my_data msg;
	my_reply reply;

	msg.ClientID = clients::TRAIN_I1;

	int server_coid;
	int index = 0;

	printf("   --> Trying to connect (server) process which has a PID: %d\n", serverPID);
	printf("   --> on channel: %d\n\n", serverChID);

	// set up message passing channel
	server_coid = ConnectAttach(ND_LOCAL_NODE, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		printf("\n    ERROR, could not connect to server!\n\n");
		return EXIT_FAILURE;
	}


	printf("Connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);

	// We would have pre-defined data to stuff here
	msg.hdr.type = 0x00;
	msg.hdr.subtype = 0x00;

	int done = 0;
	char keyboardInput = '0';

	// Do whatever work you wanted with server connection
	//for (index=0; index < 5; index++) // send data packets
	//{
	// set up data packet
	//msg.data=10+index;

	while (!done){
		while (1){
			scanf("%c", &keyboardInput);					//wait for keyboard input
			if (keyboardInput == 'i'){
				msg.data[0] = keyboardInput;
				//sprintf(buf, "%c", keyboardInput);			//put the message in a char[] so it can be sent
				break;
			}
			else if (keyboardInput == 'o'){
				msg.data[0] = keyboardInput;
				//sprintf(buf, "%c", keyboardInput);			//put the message in a char[] so it can be sent
				break;
			}
			else if (keyboardInput == 'q'){					// press q to close queue and quit
				//sprintf(buf, "%c", keyboardInput);
				msg.data[0] = keyboardInput;
				done = 1;
				break;
			}
		}
		keyboardInput = '0';

		// the data we are sending is in msg.data
		printf("Client (ID:%d), sending data packet with the  value: %c \n", msg.ClientID, msg.data[0]);
		fflush(stdout);

		if (MsgSend(server_coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		{
			printf(" Error data '%d' NOT sent to server\n", msg.data[0]);
			// maybe we did not get a reply from the server
			break;
		}
		else
		{ // now process the reply
			printf("   -->Reply is: '%s'\n", reply.buf);
		}

	}//end while
	//sleep(5);	// wait a few seconds before sending the next data packet
	//}

	// Close the connection
	printf("\n Sending message to server to tell it to close the connection\n");
	ConnectDetach(server_coid);

	return EXIT_SUCCESS;
}
