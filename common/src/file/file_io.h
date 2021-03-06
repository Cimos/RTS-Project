
/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     file_io.h
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

#include <unistd.h>
#include <string>
//#include "../../public/debug.h"

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
bool checkIfFileExists(const char *fileName, int mode = F_OK | R_OK | W_OK);


/* ----------------------------------------------------	*
 *	@read_pid_chid_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
int read_pid_chid_FromFile(int *pid, int *chid, const char *node, const char *file2Read);


/* ----------------------------------------------------	*
 *	@write_pid_chid_ToFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_pid_chid_ToFile(int pid, int chid, char *file2Write2, char *mode = "w");


/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(std::string *buf, int size2Read, std::string *file2Read, char *mode = "r");


/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(char *buf, int sizeOfBuf, char *file2Read, char *mode = "r");


/* ----------------------------------------------------	*
 *	@write_string_ToFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(std::string *buf, std::string *file2Write, char *mode = "w");


/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(char *buf, int sizeOfBuf, char *file2Write, char *mode = "w");


/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(std::string *buf, char *file2Write, char *mode = "w");

#endif /* SRC_FILE_FILE_IO_H_ */



