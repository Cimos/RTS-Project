/*
 *
 *
 * @Breif:	I2C LCD code
 * @Author:  Simon Maddison
 * @Date:	16/08/2018
 *
 */


#include "../../public/lcdThread.h"



// ---------------------------------------------------------------------------
//							Global Definitions
// ---------------------------------------------------------------------------
#define DATA_SEND 0x40  // sets the Rs value high
#define Co_Ctrl   0x00  // mode to tell LCD we are sending a single command





// ---------------------------------------------------------------------------
//							Local Functions Definitions
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]);
int  I2cWrite_(int fd, uint8_t addr, uint8_t cmd, uint8_t *pBuffer, uint32_t NbData);
void SetCursor(int fd, uint8_t LCDi2cAdd, uint8_t row, uint8_t column);
void Initialise_LCD(int fd, _Uint32t LCDi2cAdd);
void readIPaddress(char * IPaddress);

int i2cOpen(_self *self);





// ---------------------------------------------------------------------------
//							Global Function Implementation
// ---------------------------------------------------------------------------


int i2cInit(_self *self)
{
	int error = 0;
	int file;

	_Uint32t speed = 10000; // nice and slow (will work with 200000)

	error = devctl(file,DCMD_I2C_SET_BUS_SPEED,&(speed),sizeof(speed),NULL);  // Set Bus speed
	if (error)
	{
		fprintf(stderr, "->Error setting the bus speed: %d\n",strerror ( error ));
		return error;
	}
	else
		printf("->Bus speed set: %d\n", speed);
}

int lcdInit(_self *self)
{
	volatile uint8_t LCDi2cAdd = 0x3C;	// i2c address
}



// ---------------------------------------------------------------------------
//							Local Functions Implementation
// ---------------------------------------------------------------------------


// Use to Upen I2C communications to a device
int i2cOpen(_self *self)
{
	// File == handle
	int file;

	// Open I2C resource and set it up
	if ((file = open("/dev/i2c1",O_RDWR)) < 0)	  // OPEN I2C1
	{
		printf("->Error while opening Device File.!!\n");
		return -1;
	}
	else
		printf("->I2C1 Opened Successfully:\n");

	return 0;
}


/*
 * 	@breif: writes i2c messages over a i2c bus
 * 	@params: 'addr' of the i2c device, 'cmd' prior to data
 * 	@return:
 */
int  I2cWrite_(int fd, uint8_t addr, uint8_t cmd, uint8_t *pBuffer, uint32_t NbData)
{
	i2c_send_t hdr;
    iov_t sv[2];
    int status, i;

    uint8_t packet[21] = {};  // limited to 21 characters  (1 control bit + 20 bytes)

    // set the mode for the write (control or data)
    packet[0] = cmd;  // set the cmd (data or control)
    					// read or write


    // check ret code
    memcpy(&packet[1], pBuffer, NbData);

	// copy data to send to buffer (after the mode bit)
    //for (i=0;i<NbData+1;i++)
	//	packet[i+1] = *pBuffer++;

    hdr.slave.addr = addr;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = NbData + 1;  // 1 extra for control (mode) bit
    hdr.stop = 1;

    SETIOV(&sv[0], &hdr, sizeof(hdr));
    SETIOV(&sv[1], &packet[0], NbData + 1); // 1 extra for control (mode) bit

      // int devctlv(int filedes, int dcmd,     int sparts, int rparts, const iov_t *sv, const iov_t *rv, int *dev_info_ptr);
    status = devctlv(fd, 		  DCMD_I2C_SEND, 2,          0,          sv,              NULL,           NULL);

    if (status != EOK)
    	printf("status = %s\n", strerror ( status ));

    return status;
}











void readIPaddress(char * IPaddress)
{
	char *command = "ifconfig dm0";
	FILE *fpipe;
	char line[256] = {};
	char *e=NULL;
	char *p=NULL;

	fpipe = (FILE*)popen(command,"r");
	if( fpipe != NULL )
	{
	 while( fgets(line, sizeof(line), fpipe) )
	 {
		 // Find the line with 'inet ' in it
		 //printf("%s",line);
         if (p == strstr(line, "inet "))
         {
    		 //printf("%s",line);
    		 //printf("%c",*p);
        	 p = strchr(p, ' ');  	// find the position of the first space after 'inet'
        	 e = &IPaddress[0];		// set the buffer pp
        	 p++;
    		 while( (*p) != '\0') 	// pointer arithmetic, iterates through memory
    		 {
    			 //printf("%c",*p);
    			 if((*p)!=' ')  	// need to stop at the first white space
    			 {
    				 *e = *p;
    				 e++;
    				 p++;
    			 }
    			 else break;		// don't copy the rest of the string
    		 }
    		 *e++ = '\0';  // make sure there is a null terminator at the end of the IPaddress
    		 //printf("%s",IPaddress);
         }
	 }
	 pclose(fpipe);
	}
	else printf("Error, problems with pipe to ifconfig dm0!\n");
}

/*
int main(int argc, char *argv[])
{
	int file;
	int error;
	volatile uint8_t LCDi2cAdd = 0x3C;	// i2c address
	_Uint32t speed = 10000; // nice and slow (will work with 200000)
	uint8_t	LCDdata[21] = {};


	// Open I2C resource and set it up
	if ((file = open("/dev/i2c1",O_RDWR)) < 0)	  // OPEN I2C1
	{
		printf("Error while opening Device File.!!\n");
		exit(EXIT_FAILURE);
	}
	else
		//printf("I2C1 Opened Successfully\n");

	error = devctl(file,DCMD_I2C_SET_BUS_SPEED,&(speed),sizeof(speed),NULL);  // Set Bus speed
	if (error)
	{
		fprintf(stderr, "Error setting the bus speed: %d\n",strerror ( error ));
		exit(EXIT_FAILURE);
	}
	else
		//printf("Bus speed set = %d\n", speed);

	Initialise_LCD(file, LCDi2cAdd);

	usleep(1);

	// write hostname to LCD screen at position 0,0
	char hostnm[250] = {};   // #define BUF_SIZE 256
	memset(hostnm, '\0', 250);
	hostnm[250 - 1] = '\n';
	gethostname(hostnm, sizeof(hostnm));
	//printf("  --> Machine hostname is: '%s'\n", hostnm);
	SetCursor(file, LCDi2cAdd,0,0); // set cursor on LCD to first position first line
	I2cWrite_(file, LCDi2cAdd, DATA_SEND, &hostnm[0], 20);		// write new data to I2C


	char * BadIP = "0.0.0.0";
//	char * BadIP = "192.168.0.12";  // for debug only
	char IPaddress[256] = {};
	readIPaddress(IPaddress);
	//printf("%s",IPaddress);
	if(strcmp(IPaddress,BadIP))
	{
		// Got valid IP address, so we can display it and exit
		SetCursor(file, LCDi2cAdd,1,0); // set cursor on LCD to first position first line
		I2cWrite_(file, LCDi2cAdd, DATA_SEND, &IPaddress[0], 20);		// write new data to I2C
		printf("Node hostname: %s\n",hostnm);
		printf("Node IP address is: %s\n",IPaddress);
		pclose(file);
		return EXIT_SUCCESS;
	}
	else
	{
		// no IP yet, so we need to wait for DHCP server
		printf("Node hostname: %s\n",hostnm);
		printf("Waiting for IP address from DHCP server....\n");
		int i;
		for(i=120;i>=0;i--)
		{
			// check if IP has been assigned
			readIPaddress(IPaddress);
			if(strcmp(IPaddress,BadIP))
			{
				// Got valid IP address, so we can display it and exit
				SetCursor(file, LCDi2cAdd,1,0); // set cursor on LCD to first position first line
				I2cWrite_(file, LCDi2cAdd, DATA_SEND, &IPaddress[0], 20);		// write new data to I2C
				printf("Node hostname: %s\n",hostnm);
				printf("Node IP address is: %s\n",IPaddress);
				pclose(file);
				return EXIT_SUCCESS;
			}

			sprintf(LCDdata,"waiting DHCP: %d",i);
			SetCursor(file, LCDi2cAdd,1,0); // set cursor on LCD to first position first line
			I2cWrite_(file, LCDi2cAdd, DATA_SEND, &LCDdata[0], 20);		// write new data to I2C
			//printf("Debug: %s",IPaddress);
			sleep(1);
		}
	}

	printf("Failed to get IP address from DHCP server: %s\n",IPaddress);
	sprintf(LCDdata,"Error: DHCP Failed !");
	SetCursor(file, LCDi2cAdd,1,0); // set cursor on LCD to first position first line
	I2cWrite_(file, LCDi2cAdd, DATA_SEND, &LCDdata[0], 20);		// write new data to I2C
	pclose(file);
	//printf("\n complete");
	return EXIT_SUCCESS;
}
*/



void SetCursor(int fd, uint8_t LCDi2cAdd, uint8_t row, uint8_t column)
{
	uint8_t position = 0x80; // SET_DDRAM_CMD (control bit)
	uint8_t rowValue = 0;
	uint8_t	LCDcontrol = 0;
	if (row == 1)
		rowValue = 0x40;     // memory location offset for row 1
	position = (uint8_t)(position + rowValue + column);
	LCDcontrol = position;
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C
}


void Initialise_LCD (int fd, _Uint32t LCDi2cAdd)
{
	uint8_t	LCDcontrol = 0x00;

	//   Initialise the LCD display via the I2C bus
	LCDcontrol = 0x38;  // data byte for FUNC_SET_TBL1
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x39; // data byte for FUNC_SET_TBL2
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x14; // data byte for Internal OSC frequency
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x79; // data byte for contrast setting
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x50; // data byte for Power/ICON control Contrast set
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x6C; // data byte for Follower control
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x0C; // data byte for Display ON
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

	LCDcontrol = 0x01; // data byte for Clear display
	I2cWrite_(fd, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C
}




















// FT800 I2C code


// --------------------------------------- READ/WRITE ---------------------------------------


