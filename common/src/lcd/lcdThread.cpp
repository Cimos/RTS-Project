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




// ---------------------------------------------------------------------------
//							Local Functions Definitions
// ---------------------------------------------------------------------------
//int main(int argc, char *argv[]);
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
	int file = 0;

	_Uint32t speed = 10000; // nice and slow (will work with 200000)

	error = devctl(self->I2C_handle.fd,DCMD_I2C_SET_BUS_SPEED,&(self->I2C_handle.bus_speed),sizeof(self->I2C_handle.bus_speed),NULL);  // Set Bus speed
	if (error)
	{
		//fprintf(stderr, "->Error setting the bus speed: %d\n",strerror ( error ));
		return error;
	}
	else
	{
		//printf("->Bus speed set: %d\n", speed);
	}

	return 0;
}

int lcdInit(_self *self)
{

	return 0;
}



// ---------------------------------------------------------------------------
//							Local Functions Implementation
// ---------------------------------------------------------------------------


