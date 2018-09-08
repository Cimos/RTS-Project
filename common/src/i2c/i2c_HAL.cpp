/*
 * i2c_HAL.c
 *
 *  Created on: 20Aug.,2018
 *  	Author: MadMan
 *
 *  BBB P9 connections:
 *    - P9_Pin17 - SCL - I2C1	 GPIO3_2
 *    - P9_Pin18 - SDA - I2C1    GPIO3_1
 *
 *
 */

#include "../../public/i2c_HAL.h"
#include <strings.h>
#include <hw/i2c.h>



#define ACTIVE  0x00
#define STANDBY 0x41
#define SLEEP   0x42
#define PWRDOWN 0x50
#define CLKEXT  0x44
#define CLK48M  0x62
#define CLK36M  0x61
#define CORERST 0x68

#define I2C_PORT            0
#define I2C_ADDRESS         0x23
#define I2C_FREQUENCY       400000    //100k hz //400k needed?




/*-----------------------------------------------------------------------------
 * Global Variables Declarations
 *---------------------------------------------------------------------------*/
typedef struct
{
	int pin;
}GPIO_HANDLE;

typedef struct {
	int port;
}I2C_HANDLE;


static struct {
  GPIO_HANDLE reset;
  I2C_HANDLE i2c;
  ft_uint8_t reg_id;
  UINT8 addr;
} _ft800;



unsigned short dli;
unsigned short cmd_offset = 0;

/*-----------------------------------------------------------------------------
 * Global Function Declarations
 *---------------------------------------------------------------------------*/

void I2C_Open(I2C_HANDLE *p, int i, int j, int k, int l);
void I2C_Close(I2C_HANDLE *p);


int I2C_Write(I2C_HANDLE *p, UINT8 addr, int size, UINT8 *command);
int I2C_Transaction(I2C_HANDLE *p, UINT8 addr, int size, UINT8 *command, int size2, UINT8 *retBuf);




void *umalloc(int size);
void ufree(UINT8 *free);



/*-----------------------------------------------------------------------------
 * Global Function Implementation
 *---------------------------------------------------------------------------*/




// --------------------------------------- READ/WRITE ---------------------------------------

void host_command(UINT8 command)
{
    UINT8 data[3] = {};

    data[0] = 0x40 | (command & 0x3F);      // Command[5:0]   // I dont think this is what the arduino code was doing.
    data[1] = 0x00;                         // [15:8]
    data[2] = 0x00;                         // [7:0]

    I2C_Write(&_ft800.i2c, _ft800.addr<<1, 3, &command);
}

void wr8(UINT32 addr, ft_uint8_t value)
{
    int ret;
    UINT8 data[4] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);     // Upper two bits forces to '80'
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value;

    ret = I2C_Write(&_ft800.i2c, _ft800.addr<<1, sizeof(data), data);
}

void wr16(UINT32 addr, ft_uint16_t value)
{
    int ret;
    UINT8 data[5] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value & 0xFF;
    data[4] = value>>8 & 0xFF;

    ret = I2C_Write(&_ft800.i2c, _ft800.addr<<1, 5, data);
}

void wr32(UINT32 addr, ft_uint32_t value)
{
    int ret;
    UINT8 data[7] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value & 0xFF;
    data[4] = value>>8 & 0xFF;
    data[5] = value>>16 & 0xFF;
    data[6] = value>>24 & 0xFF;

    ret = I2C_Write(&_ft800.i2c, _ft800.addr<<1, 7, data);
}

ft_uint8_t rd8(UINT32 addr)
{
    int ret;
    UINT8 data[3] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);   // Upper two bits forces to '00'
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    // ret is error code and should be check
    // step 1 -- 10
    ret = I2C_Transaction(&_ft800.i2c, _ft800.addr<<1,
                            3, data,    //tx
                            1, data);   //rx

    return (ft_uint8_t)data[0];
}

ft_uint16_t rd16(UINT32 addr)
{
    int ret;
    UINT8 data[3] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    ret = I2C_Transaction(&_ft800.i2c, _ft800.addr<<1,
                            3, data,
                            2, data);

    return (ft_uint16_t)((data[0] << 8) | (data[1]));
}

ft_uint32_t rd32(UINT32 addr)
{
    int ret;
    UINT8 data[4] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    ret = I2C_Transaction(&_ft800.i2c, _ft800.addr<<1,
                            3, data,
                            4, data);

    return (ft_uint32_t)((data[0] << 24) | (data[1] << 16)
                       | (data[2] << 8)  | (data[3]));
}

ft_uint8_t wr8s(UINT32 addr, ft_char8_t *s)   // max 20 bytes of data excluding addr
{
    int length = strlen(s);
    int allignment = length%4;
    int i = 0;
    UINT8 *data = NULL;


	if ((data = (UINT8 *)umalloc((length+3) + allignment)) == NULL)
	{
		return i;
	}

    data[0] = 0x80 + ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    for (i = 0; i<length ; i++)
    {
      data[i+3] = s[i];
    }
    for (; i%4;i++)
    {
      data[i+3] = 0x00;
    }

    I2C_Write(&_ft800.i2c, _ft800.addr<<1, (length + 3 + allignment), data);

    ufree(data);
    return i;
}

unsigned int incCMDOffset(unsigned int currentOffset, unsigned char commandSize)
{
    unsigned int newOffset;			// used to hold new offset
    newOffset = currentOffset + commandSize;	// Calculate new offset
    if(newOffset > 4095)			// If new offset past boundary...
    {
        newOffset = (newOffset - 4096);		// ... roll over pointer
    }
    return newOffset;				// Return new offset
}


void cmd_incrementn(unsigned char n)
{
    cmd_offset=(cmd_offset+n)%4096;
}

void dl_cmd(unsigned long cmd)
{
  wr32(RAM_DL + dli, cmd);
  dli += 4;
}


void cmd(ft_uint32_t command){
    wr32(RAM_CMD + cmd_offset, command);
    cmd_increment4();
}

void i2c_LCD_port_OPEN()
{
    I2C_Open(&_ft800.i2c, I2C_PORT, I2C_FREQUENCY, 30, 1);
}

void i2c_LCD_port_CLOSE()
{
    I2C_Close(&_ft800.i2c);
}










/// REDO: Just so project compiles


int I2C_Write(I2C_HANDLE *p, UINT8 addr, int size, UINT8 *command)
{
	addr = 0;
	return 0;
}


int I2C_Transaction(I2C_HANDLE *p, UINT8 addr, int size, UINT8 *command, int size2, UINT8 *retBuf)
{
	return 0;
}


void I2C_Open(I2C_HANDLE *p, int i, int j, int k, int l)
{
	i = 1;
	return;
}

void I2C_Close(I2C_HANDLE *p)
{
	return;
}


void *umalloc(int size)
{
	return 0;
}
void ufree(UINT8 *free)
{
	return;
}
