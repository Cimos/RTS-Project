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
#include <fcntl.h>
#include <sys/netmgr.h>

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
bool write_pid_chid_ToFile(int pid, int chid, char *file2Write2, char *mode)
{
	FILE *fp;
	int ret = 0, errorVal;

	// creating csv string to write to file
	std::string str("PID=" + std::to_string(pid) +",CID=" + std::to_string(chid) + ";");
	//str = "PID=" + std::to_string(pid) +",CID=" + std::to_string(chid) + ";";

	fp = fopen(file2Write2, mode);
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
		DEBUGF("Error: %s\n", strerror( errorVal));
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
int read_pid_chid_FromFile(int *pid, int *chid, const char *node, const char *file2Read)
{
	int fd;
	int nd;
	char line[255] = {};
	int pos1=0,pos2=0,pos3=0,pos4=0;
	std::string _file2Read(node);
	_file2Read.append(file2Read);

	// Open file with read only
	fd = open(_file2Read.c_str(), O_RDWR );	// mode = read only
	DEBUGF("File Open:\n");


	if( fd != NULL )
	{
		DEBUGF("File Reading:\n");
		while(read(fd,  &line,  sizeof(line)) != 0 )
		{
			DEBUGF("id=%s\n", line );
		}
	}
	else
	{
		return -1;
	}

	// close file
	close(fd);
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
		return -1;
	}

	DEBUGF("SubString1: %s\n",str.substr(pos1+sizeof("PID=")-1, pos2).c_str());
	DEBUGF("SubString2: %s\n",str.substr(pos3+sizeof("CID=")-1, pos4).c_str());

	*pid = atoi(str.substr(pos1+sizeof("PID=")-1, pos2).c_str());
	*chid = atoi(str.substr(pos3+sizeof("CID=")-1, pos4).c_str());

	nd = netmgr_strtond( node, NULL);
	if (nd == -1) {
		DEBUGF("Filed to get ND");
		return -1;
	}
	else {
		DEBUGF ("Node id for %s is %d.\n", node, nd);
	}

	return nd;
}



/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@brief:												*
 *														*
 * 		a	:	Append: create a new file or open		*
 * 				the file for writing at its end.		*
 * 		a+	:	Append: open the file or create it		*
 * 				for update, writing at end-of-file;		*
 * 				use the default file translation.		*
 * 		r	:	Open the file for reading.				*
 * 		r+	:	Open the file for update (reading		*
 * 				and/or writing);						*
 * 				use the default file translation.		*
 * 		w	:	Create the file for writing,			*
 * 				or truncate it to zero length.			*
 * 		w+	:	Create the file for update, or			*
 * 				truncate it to zero length;				*
 * 				use the default file translation		*
 *														*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(std::string *buf, int size2Read, std::string *file2Read, char *mode)
{
	FILE *fp;
	char *line = (char *)malloc(size2Read);

	// Open file with read only
	fp = fopen(file2Read->c_str(), mode );	// mode = read only
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
 *														*
 * 		a	:	Append: create a new file or open		*
 * 				the file for writing at its end.		*
 * 		a+	:	Append: open the file or create it		*
 * 				for update, writing at end-of-file;		*
 * 				use the default file translation.		*
 * 		r	:	Open the file for reading.				*
 * 		r+	:	Open the file for update (reading		*
 * 				and/or writing);						*
 * 				use the default file translation.		*
 * 		w	:	Create the file for writing,			*
 * 				or truncate it to zero length.			*
 * 		w+	:	Create the file for update, or			*
 * 				truncate it to zero length;				*
 * 				use the default file translation		*
 *														*
 *	@return:											*
 * ---------------------------------------------------	*/
bool read_string_FromFile(char *buf, int sizeOfBuf, char *file2Read, char *mode)
{
	FILE *fp;
	char *line = (char *)malloc(sizeOfBuf);

	// Open file with read only
	fp = fopen(file2Read,  mode );	// mode = read only
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



/* ----------------------------------------------------	*
 *	@read_string_FromFile Implementation:				*
 *	@description:										*
 *														*
 * 		a	:	Append: create a new file or open		*
 * 				the file for writing at its end.		*
 * 		a+	:	Append: open the file or create it		*
 * 				for update, writing at end-of-file;		*
 * 				use the default file translation.		*
 * 		r	:	Open the file for reading.				*
 * 		r+	:	Open the file for update (reading		*
 * 				and/or writing);						*
 * 				use the default file translation.		*
 * 		w	:	Create the file for writing,			*
 * 				or truncate it to zero length.			*
 * 		w+	:	Create the file for update, or			*
 * 				truncate it to zero length;				*
 * 				use the default file translation		*
 *														*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(char *buf, int sizeOfBuf, char *file2Write, char *mode)
{
	FILE *fp;
	int ret = 0, errorVal;
	// Open file with read only
	fp = fopen(file2Write, mode );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		// setting errno to know val
		errno = EOK;

		DEBUGF("File Writing:\n");
		ret = fwrite(buf, sizeof(char), sizeOfBuf, fp );

		// getting errno val
		errorVal = errno;

		// error if write didnt work properly
		if (errorVal != EOK)
		{
			DEBUGF("Error: %s\n", strerror(errorVal));
			return false;
		} else if (ret <= 0)
		{
			DEBUGF("Error with write file info\n");
			return false;
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

	// return success
	return true;
}




/* ----------------------------------------------------	*
 *	@write_string_ToFile Implementation:				*
 *	@brief:												*
 *	@description:										*
 *														*
 * 		a	:	Append: create a new file or open		*
 * 				the file for writing at its end.		*
 * 		a+	:	Append: open the file or create it		*
 * 				for update, writing at end-of-file;		*
 * 				use the default file translation.		*
 * 		r	:	Open the file for reading.				*
 * 		r+	:	Open the file for update (reading		*
 * 				and/or writing);						*
 * 				use the default file translation.		*
 * 		w	:	Create the file for writing,			*
 * 				or truncate it to zero length.			*
 * 		w+	:	Create the file for update, or			*
 * 				truncate it to zero length;				*
 * 				use the default file translation		*
 *														*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(std::string *buf, std::string *file2Write, char *mode)
{
	FILE *fp;
	int ret = 0, errorVal;
	// Open file with read only
	fp = fopen(file2Write->c_str(), mode );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		// setting errno to know val
		errno = EOK;

		DEBUGF("File Writing:\n");
		ret = fwrite(buf->c_str(), sizeof(char), buf->length(), fp );

		// getting errno val
		errorVal = errno;

		// error if write didnt work properly
		if (errorVal != EOK)
		{
			DEBUGF("Error: %s\n", strerror(errorVal));
			return false;
		} else if (ret <= 0)
		{
			DEBUGF("Error with write file info\n");
			return false;
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

	// return success
	return true;
}







/* ----------------------------------------------------	*
 *	@write_string_ToFile Implementation:				*
 *	@brief:												*
 *	@description:										*
 *														*
 * 		a	:	Append: create a new file or open		*
 * 				the file for writing at its end.		*
 * 		a+	:	Append: open the file or create it		*
 * 				for update, writing at end-of-file;		*
 * 				use the default file translation.		*
 * 		r	:	Open the file for reading.				*
 * 		r+	:	Open the file for update (reading		*
 * 				and/or writing);						*
 * 				use the default file translation.		*
 * 		w	:	Create the file for writing,			*
 * 				or truncate it to zero length.			*
 * 		w+	:	Create the file for update, or			*
 * 				truncate it to zero length;				*
 * 				use the default file translation		*
 *														*
 *	@return:											*
 * ---------------------------------------------------	*/
bool write_string_ToFile(std::string *buf, char *file2Write, char *mode)
{
	FILE *fp;
	int ret = 0, errorVal;
	// Open file with read only
	fp = fopen(file2Write, mode );	// mode = read only
	DEBUGF("File Open:\n");


	if( fp != NULL )
	{
		// setting errno to know val
		errno = EOK;

		DEBUGF("File Writing:\n");
		ret = fwrite(buf->c_str(), sizeof(char), buf->length(), fp );

		// getting errno val
		errorVal = errno;

		// error if write didnt work properly
		if (errorVal != EOK)
		{
			DEBUGF("Error: %s\n", strerror(errorVal));
			return false;
		} else if (ret <= 0)
		{
			DEBUGF("Error with write file info\n");
			return false;
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

	// return success
	return true;
}




