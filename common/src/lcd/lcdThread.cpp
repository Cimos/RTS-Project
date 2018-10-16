/*
 *
 *
 * @Breif:	I2C LCD code
 * @Author:  Simon Maddison
 * @Date:	16/08/2018
 *
 */


#include "../../public/lcdThread.h"
#include <unistd.h>



// ---------------------------------------------------------------------------
//							Global Definitions
// ---------------------------------------------------------------------------

#define MAX_BRIGHTNESS_LCD        128
#define MIN_BRIGHTNESS_LCD        0



// ---------------------------------------------------------------------------
//							Local Functions Definitions
// ---------------------------------------------------------------------------
//int main(int argc, char *argv[]);
int  I2cWrite_(int fd, uint8_t addr, uint8_t cmd, uint8_t *pBuffer, uint32_t NbData);
void SetCursor(int fd, uint8_t LCDi2cAdd, uint8_t row, uint8_t column);
void Initialise_LCD(int fd, _Uint32t LCDi2cAdd);
void readIPaddress(char * IPaddress);

int i2cOpen(_self_i2c *self);


// ---------------------------------------------------------------------------
//							Global Function Implementation
// ---------------------------------------------------------------------------

int i2cInit(_self_i2c *self)
{
	int error = 0;

	error = devctl(self->I2C_handle.fd,DCMD_I2C_SET_BUS_SPEED,&(self->I2C_handle.bus_speed),sizeof(self->I2C_handle.bus_speed),NULL);  // Set Bus speed
	if (error)
	{
		DEBUGF("lcd->Error setting the bus speed: %d\n",strerror ( error ));
		return error;
	}
	else
	{
		DEBUGF("lcd->Bus speed set: %d\n", self->I2C_handle.bus_speed);
	}

	return 0;
}

int lcdInit(_self_i2c *self)
{
	return 0;
}



// 240 *2
// 136 * 2
void splash_screen()
{
		start_screen(CLEAR_COLOR_RGB(0,0,0));
		make_string(240,106,31,OPT_CENTER,COLOR_RGB(255,255,255), "RTS Project 2018");
		make_string(240,166,22,OPT_CENTER,COLOR_RGB(255,255,255), "Simon Maddison, Shawn Buschmann, Michael Stekla");
		end_screen();
}




// ---------------------------------------------------------------------------
//							Local Functions Implementation
// ---------------------------------------------------------------------------





