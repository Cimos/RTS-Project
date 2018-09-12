
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



/*
 * @breif: use to check if file exists
 * @param: 	pointer to file name.
 * 			mode: includes F_OK, R_OK , W_OK , X_OK
 * @ret: returns true or false for file
 *
 */
bool checkIfFileExists(const char *fileName, int mode);

void write_pid_chid_ToFile(void);

void read_pid_chid_FromFile(void);


#endif /* SRC_FILE_FILE_IO_H_ */



