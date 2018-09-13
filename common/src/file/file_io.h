
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


#ifndef SRC_FILE_FILE_IO_H_
#define SRC_FILE_FILE_IO_H_


/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/





/* ----------------------------------------------------	*
 *	@getMsgQueId checkIfFileExists:						*
 *	@breif: use to check if file exists 				*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool checkIfFileExists(const char *fileName, int mode);


/* ----------------------------------------------------	*
 *	@read_pid_chid_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_pid_chid_FromFile(int *pid, int *chid, char *file2Read);


/* ----------------------------------------------------	*
 *	@write_pid_chid_ToFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_pid_chid_ToFile(int pid, int chid, char *file2Write2, char mode = 'w');


#endif /* SRC_FILE_FILE_IO_H_ */



