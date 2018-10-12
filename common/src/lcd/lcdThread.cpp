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








void Screen_animations(int i)   // increment i
{
  dli = 0; // start writing the display list
  dl_cmd(CLEAR(1, 1, 1)); // clear screen
  dl_cmd(BEGIN(BITMAPS)); // start drawing bitmaps
  dl_cmd(VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
  dl_cmd(VERTEX2II(244, 110, 31, 'T')); // ascii T
  dl_cmd(VERTEX2II(270, 110, 31, 'D')); // ascii D
  dl_cmd(VERTEX2II(299, 110, 31, 'I')); // ascii I
  dl_cmd(END());
  dl_cmd(COLOR_RGB(255, 0, 0)); // change color to red
  dl_cmd(POINT_SIZE(320)); // set point size
  dl_cmd(BEGIN(POINTS)); // start drawing points
  dl_cmd(VERTEX2II(i%480, i%272, 0, 0)); // red point
  dl_cmd(END());
  dl_cmd(DISPLAY());
  usleep(1);
  wr8(REG_DLSWAP,DLSWAP_FRAME);
}



// ---------------------------------------------------------------------------
//							Local Functions Implementation
// ---------------------------------------------------------------------------





