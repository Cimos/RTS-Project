/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     projectTrafficLight.c
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
#include <iostream>

#include "workerThread.h"
#include "file_io.h"
#include "ipc_dataTypes.h"
#include <unistd.h>
#include <String>
#include <sys/iofunc.h>
#include <sys/netmgr.h>
/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/




/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
int client(int serverPID,  int serverChID);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	puts("RTS - Traffic Light Project");

	int pid = 0;
	int chid = 0;


	read_pid_chid_FromFile(&pid, &chid, CONTROLHUB,CONTROLHUB_SERVER);

	std::cout << "pid = " << pid << std::endl;
	std::cout << "chid = " << chid << std::endl;



	client(pid, chid);






	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/

/*** Client code ***/
int client(int serverPID,  int serverChID)
{
	_data msg_;
    _reply reply_;

    msg_.ClientID = 509;

    int server_coid;
    int index = 0;

	printf("   --> Trying to connect (server) process which has a PID: %d\n",   serverPID);
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
    msg_.hdr.type = 0x00;
    msg_.hdr.subtype = 0x00;

    // Do whatever work you wanted with server connection
    for (index=0; index < 5; index++) // send data packets
    {
    	// set up data packet
    	//msg_.data=10+index;

    	// the data we are sending is in msg.data
        printf("Client (ID:%d), sending data packet with the integer value: %d \n", msg_.ClientID, msg_.data);
        fflush(stdout);

        if (MsgSend(server_coid, &msg_, sizeof(msg_), &reply_, sizeof(reply_)) == -1)
        {
            printf(" Error data '%d' NOT sent to server\n", msg_.data);
            	// maybe we did not get a reply from the server
            break;
        }
        else
        { // now process the reply
//            printf("   -->Reply is: '%s'\n", reply_.buf);
        }

        sleep(5);	// wait a few seconds before sending the next data packet
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    ConnectDetach(server_coid);

    return EXIT_SUCCESS;
}

