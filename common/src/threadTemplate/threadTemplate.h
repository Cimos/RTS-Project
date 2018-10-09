/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     threadTemplate.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
*/



#ifndef SRC_THREADTEMPLATE_THREADTEMPLATE_H_
#define SRC_THREADTEMPLATE_THREADTEMPLATE_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

//#include "../../public/debug.h"
#include <string>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/


#define DATA_MODE 1
#define CMD_MODE 2
/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

struct workBuf{
  std::string *data = NULL;
  int mode = 0;
  int size = 0;
};

/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/
class noName // Choose Name
{

private:
  	class Private;
  	Private *_pimpl;

public:
	int doWork(char *buf, int size, int mode);
	noName();
	~noName();
};




#endif /* SRC_THREADTEMPLATE_THREADTEMPLATE_H_ */

