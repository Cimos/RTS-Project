///*
// * clientService.cpp
// *
// *  Created on: 16Oct.,2018
// *      Author: MadMan
// */
//
//
//
//
//
///* ----------------------------------------------------	*
// *	@trainClient_Start Implementation:					*
// *	@brief:												*
// *	@return:											*
// * ---------------------------------------------------	*/
//void trainClient_Start(int prio)
//{
//	Lock(self.client.Mtx);
//	self.client.clientInitThread.priority = prio;
//	self.client.clientWorkThread.priority = prio;
//	threadInit(&self.client.clientInitThread);
//	pthread_create(&self.client.clientInitThread.thread, &self.client.clientInitThread.attr, clientService, NULL);
//
//	Unlock(self.client.Mtx);
//}
//
//
///* ----------------------------------------------------	*
// *	@clientService Implementation:						*
// *	@brief:												*
// *	@return:	server_coid								*
// * ---------------------------------------------------	*/
//void *clientService(void *notUsed)
//{
//	bool fileExists = false;
//	int nD = -1;
//	DelayTimer timout(false, 0, 3, 0, 0);
//	int pid = 0;
//	int chid = 0;
//
//	Lock(self.client.Mtx);
//	// naming thread
//	pthread_setname_np(pthread_self(),self.client.servicethreadName);
//
//	Unlock(self.client.Mtx);
//
//	std::string fullFilePath(TRAINSTATION);
//	fullFilePath.append(TRAIN_SERVER);
//
//
//	while(true)
//	{
//		//checking if file for server exists
//		do
//		{
//			DEBUGF("clientService->checking for file with train server details\n");
//
//			fileExists = checkIfFileExists(fullFilePath.c_str());
//			timout.createTimer();
//
//		}while(!fileExists);
//
//		nD = read_pid_chid_FromFile(&pid, &chid, TRAINSTATION, TRAIN_SERVER);
//
//		if (nD != 0)
//		{
//			break;
//			DEBUGF("clientService->Server file found with a valid node descriptor\n");
//		}
//		timout.createTimer();
//	}
//
//	Lock(self.client.Mtx);
//	self.client.serverPID = pid;
//	self.client.serverCHID = chid;
//	self.client.nodeDescriptor = nD;
//	Unlock(self.client.Mtx);
//
//	// start client service for train station
//	self.client.clientWorkThread.priority = 10;
//	threadInit(&self.client.clientWorkThread);
//
//	pthread_create(&self.client.clientWorkThread.thread, &self.client.clientWorkThread.attr, train_client, NULL);
//
//	// wait for working thread to finish
//	pthread_join(self.client.clientWorkThread.thread, NULL);
//
//
//	//locking mutex
//	Lock(self.client.Mtx);
//
//	// Check if living and if node has failed.. i.e. a drop.
//	if (self.client.living == 0)
//	{
//		Unlock(self.client.Mtx);
//		return NULL;
//	}
//	// create a thread that is this function and then exit this thread.
//	pthread_create(&self.client.clientInitThread.thread, &self.client.clientInitThread.attr, train_client, NULL);
//
//	// reconnection counter
//	// create longer delay?
//	Unlock(self.client.Mtx);
//
//
//	return NULL;
//}
//
//
//
///* ----------------------------------------------------	*
// *	@clientConnect Implementation:						*
// *	@brief:												*
// *	@return:	int										*
// * ---------------------------------------------------	*/
//int clientConnect(int serverPID,  int serverChID, int nodeDescriptor)
//{
//    int server_coid;
//
//    DEBUGF("clientConnect->trying to connect (server) process which has a PID: %d\n", serverPID);
//    DEBUGF("clientConnect->on channel: %d\n", serverChID);
//
//	// set up message passing channel
//    server_coid = ConnectAttach(nodeDescriptor, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
//	if (server_coid == -1)
//	{
//		DEBUGF("clientConnect->ERROR, could not connect to server!\n\n");
//        return server_coid;
//	}
//
//	DEBUGF("clientConnect->connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);
//    return server_coid;
//}
//
//
///* ----------------------------------------------------	*
// *	@clientDisconnect Implementation:					*
// *	@brief:												*
// *	@return:	server_coid								*
// * ---------------------------------------------------	*/
//void clientDisconnect(int server_coid)
//{
//    DEBUGF("\n Sending message to server to tell it to close the connection\n");
//    ConnectDetach(server_coid);
//    return;
//}
//
//
//
//
//
//void *train_client(void *appData)
//{
//	DEBUGF("trainClient->Thread started\n");
//	//  Timer
//	DelayTimer pingInterval(false, 0, 1, 0, 0);
//	DelayTimer channelCreate(false,0,1,0,0);
//	int server_coid = 0;
//	// set up data packet
//	bool message = false;
//	bool reply_train = false;
//
//	// mutex locking
//	Lock(self.client.Mtx);
//
//    int pid = self.client.serverPID;
//    int chid = self.client.serverCHID;
//    int nodeDescriptor = self.client.nodeDescriptor;
//    self.client.server_coid = &server_coid;
//	// naming thread
//	pthread_setname_np(pthread_self(),self.client.workingthreadName);
//
//	// mutex unlocking
//	Unlock(self.client.Mtx);
//
//	// Print debug
//	DEBUGF("trainClient->Connecting to Train Server: PID: %d \tCHID %d\n", pid,chid);
//
//	// Set up message channel
//	do
//	{
//	DEBUGF("trainClient->Creating channel to server");
//	server_coid = clientConnect(pid, chid,nodeDescriptor);
//	channelCreate.createTimer();
//	}while(server_coid < 0);
//
//
//	while (self.client.living)
//	{
//
//
//		// Try sending the message
//		if (MsgSend(server_coid, &message, sizeof(message), &reply_train, sizeof(reply_train)) == -1)
//		{
//			// Reply not received from server
//			DEBUGF("trainClient->Error data NOT sent to train server\n", message);
//		}
//		else
//		{
//			// Process the reply
//			DEBUGF("trainClient->train sServer reply is: '%d'\n", reply_train);
//
//			// Update train server
//			if (reply_train)
//			{
//				Lock(self.client.Mtx);
//				self.client.Train1 = trainStationStates::BOOM_GATE_UP;
//				Unlock(self.client.Mtx);
//				DEBUGF("trainClient->train boom gates are up\n");
//			}
//			else
//			{
//				Lock(self.client.Mtx);
//				self.client.Train1 = trainStationStates::BOOM_GATE_UP;
//				Unlock(self.client.Mtx);
//				DEBUGF("trainClient->train boom gates are down\n");
//			}
//		}
//
//		// ping delay
//		pingInterval.createTimer();
//	}
//
//	// Close  connection
//	clientDisconnect(server_coid);
//
//	DEBUGF("trainClient->Thread Terminated\n");
//	return EXIT_SUCCESS;
//}
//
