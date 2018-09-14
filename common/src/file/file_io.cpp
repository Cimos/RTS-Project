/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     file_io.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <string.h>	// used for memcopy

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

/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/




/* ----------------------------------------------------	*
 *	@getMsgQueId checkIfFileExists:						*
 *	@breif: 											*
 *	@return:returns true or false for success			*
 * ---------------------------------------------------	*/
bool checkIfFileExists(const char *fileName, int mode)
{
	// error code returned from access
	int error = -1;

	error = access( fileName, F_OK );
	DEBUGF("Checking If File Exists:\n");

	if (error == 0)
		return true;

	return false;
}



/* ----------------------------------------------------	*
 *	@getMsgQueId write_pid_chid_ToFile:					*
 *	@breif: 											*
 *	@return:returns true or false for success			*
 * ---------------------------------------------------	*/
bool write_pid_chid_ToFile(int pid, int chid, char *file2Write2, char mode)
{
	FILE *fp;
	char line[255] = {};
	int ret = 0, errorVal;

	// creating csv string to write to file
	std::string str("PID=" + std::to_string(pid) +",CID=" + std::to_string(chid) + ";");
	//str = "PID=" + std::to_string(pid) +",CID=" + std::to_string(chid) + ";";

	fp = fopen(file2Write2, &mode);
	DEBUGF("File Open:\n");

	// setting errno to know val
	errno = EOK;

	// writing to file
	ret = fwrite(str.c_str(), sizeof(char), str.length(), fp);

	// getting errno val
	errorVal = errno;

	// error if write didnt work properly
	if (errorVal != EOK)
	{
		DEBUGF("Error: %d\n", strerror( errorVal));
		return false;
	} else if (ret <= 0)
	{
		DEBUGF("Error with opening file info\n");
		return false;
	}

	DEBUGF("Written %d characters to \"%s\":\n",ret, file2Write2);

	ret = fclose(fp);
	DEBUGF("File Close:\n");

	return true;
}




/* ----------------------------------------------------	*
 *	@getMsgQueId read_pid_chid_FromFile:				*
 *	@breif: 											*
 *	@return:returns true or false for success			*
 * ---------------------------------------------------	*/
bool read_pid_chid_FromFile(int *pid, int *chid, char *file2Read)
{
	FILE *fp;
	char line[255] = {};
	int pos1=0,pos2=0,pos3=0,pos4=0;

	// Open file with read only
	fp = fopen(file2Read, "r" );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		DEBUGF("File Reading:\n");
		while( fread( &line, sizeof(char), sizeof(line), fp ) != 0 )
		{
			DEBUGF("id=%s\n", line );
		}
	}
	else
	{
		return false;
	}

	// close file
	fclose(fp);
	DEBUGF("File Close:\n");

	// using strings to easily find pid cid
	std::string str(line);

	pos1 = str.find("PID=");
	pos2 = str.find(",CID");
	pos3 = str.find("CID=");
	pos4 = str.find(";");

	DEBUGF("pos1: %d\n",pos1);
	DEBUGF("pos2: %d\n",pos2);
	DEBUGF("pos3: %d\n",pos3);
	DEBUGF("pos4: %d\n",pos4);

	// Checking that all str->find() returned a position
	if (pos1 == -1 || pos2 == -1 || pos3 == -1 || pos4 == -1)
	{
		*pid = -1;
		*chid = -1;
		return false;;
	}

	DEBUGF("SubString1: %s\n",str.substr(pos1+sizeof("PID=")-1, pos2).c_str());
	DEBUGF("SubString2: %s\n",str.substr(pos3+sizeof("CID=")-1, pos4).c_str());

	*pid = atoi(str.substr(pos1+sizeof("PID=")-1, pos2).c_str());
	*chid = atoi(str.substr(pos3+sizeof("CID=")-1, pos4).c_str());

	// TODO: check for errors in atoi() function call;

	return true;
}



/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(std::string *buf, int size2Read, char *file2Read)
{
	FILE *fp;
	char *line = (char *)malloc(size2Read);
	int pos1=0,pos2=0,pos3=0,pos4=0;

	// Open file with read only
	fp = fopen(file2Read, "r" );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		DEBUGF("File Reading:\n");
		while( fread(line, sizeof(char), size2Read, fp ) != 0 )
		{
			DEBUGF("%s\n", *line );
		}
	}
	else
	{
		// return failure
		return false;
	}

	// close file
	fclose(fp);
	DEBUGF("File Close:\n");

	// copying read data into string
	(*buf) = line;

	// freeing malloc after use
	free(line);

	// return success
	return true;
}



/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(char *buf, int sizeOfBuf, char *file2Read)
{
	FILE *fp;
	char *line = (char *)malloc(sizeOfBuf);
	int pos1=0,pos2=0,pos3=0,pos4=0;

	// Open file with read only
	fp = fopen(file2Read, "r" );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		DEBUGF("File Reading:\n");
		while( fread(line, sizeof(char), sizeOfBuf, fp ) != 0 )
		{
			DEBUGF("%s\n", *line );
		}
	}
	else
	{
		// return failure
		return false;
	}

	// close file
	fclose(fp);
	DEBUGF("File Close:\n");

	// copying read data into buffer is sizeOfBud
	memcpy((void*)buf, (void*)line, (size_t)sizeOfBuf);

	// freeing mem after use
	free((void*)line);

	// return success
	return true;
}




