/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     FT800.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/





/*-----------------------------------------------------------------------------
 * Included Files
 *---------------------------------------------------------------------------*/
#include "FT800.h"

#include <unistd.h>

#include "../../public/debug.h"
#include "../i2c/i2c_HAL.h"
#include "../gpio/boneGpio.h"
#include "string.h"
#include "stdio.h"

/*-----------------------------------------------------------------------------
 * Definitions
 *---------------------------------------------------------------------------*/
#define ACTIVE  0x00
#define STANDBY 0x41
#define SLEEP   0x42
#define PWRDOWN 0x50
#define CLKEXT  0x44
#define CLK48M  0x62
#define CLK36M  0x61
#define CORERST 0x68

#define LCD_RST (1<<16)  // LCD_RST is connected to rst on the click breakout board		pin 16


/*-----------------------------------------------------------------------------
 * Global Variables Declarations
 *---------------------------------------------------------------------------*/

//extern unsigned short cmdOffset;


  ft_uint8_t reg_id;

  unsigned short dli;
  unsigned short cmd_offset = 0;


void set_screen_rotation(UINT32 bit_0);

// --------------------------------------- INIT SCREEN ---------------------------------------
void FT800_Init(void)
{
	DEBUGF("Initializing FT800_i2c_port");


	i2c_LCD_port_OPEN();


	if (setPinIOStatus(gpio1_13,gpio1_13_config,0))
			DEBUGF("Pin Set\n");

	usleep(1);
	wake_screen();

	wr8(REG_PCLK, ZERO);    // Set PCLK to zero - don't clock the LCD until later
	wr8(REG_PWM_DUTY, ZERO);    // Turn off backlight

	// Initialize Display
	wr16(REG_HSIZE, SCREEN_HSIZE_16); // width resolution
	wr16(REG_VSIZE, SCREEN_VSIZE_16); // height resolution
	wr16(REG_HCYCLE, SCREEN_HCYCLE_16); // number if horizontal cycles for display
	wr16(REG_HSYNC0, SCREEN_HSYNC0_16); // hsync falls
	wr16(REG_HSYNC1, SCREEN_HSYNC1_16); // hsync rise
	wr16(REG_HOFFSET, SCREEN_HOFFSET_16); // horizontal offset from starting signal
	wr16(REG_VCYCLE, SCREEN_VCYCLE_16); // number of vertical cycles for display
	wr16(REG_VSYNC0, SCREEN_VSYNC0_16); // vsync falls
	wr16(REG_VSYNC1, SCREEN_VSYNC1_16); // vsync rise
	wr16(REG_VOFFSET, SCREEN_VOFFSET_16); // vertical offset from start signal
	wr8(REG_SWIZZLE, SCREEN_SWIZZLE_8); // output swizzle
	wr8(REG_PCLK_POL, SCREEN_PCLK_POL); // clock polarity: 0 - rising edge, 1 - falling edge

	wr8(REG_CSPREAD, 1); // output clock spread enable
	wr8(REG_DITHER, 1); // output number of bits

	// End of Initialize Display
	//***************************************
	//***************************************

	// Configure Touch and Audio - not used in this example, so disable both
	wr8(REG_TOUCH_MODE, ZERO);    // Disable touch
	wr16(REG_TOUCH_RZTHRESH, ZERO); // Eliminate any false touches

	wr8(REG_VOL_PB, ZERO);    // turn recorded audio volume down
	wr8(REG_VOL_SOUND, ZERO);   // turn synthesizer volume down
	wr16(REG_SOUND, 0x6000);    // set synthesizer to mute

	// End of Configure Touch and Audio
	//***************************************
	//***************************************

	// Write Initial Display List & Enable Display

	// write first display list
	wr32(RAM_DL+0,CLEAR_COLOR_RGB(0,0,0));
	wr32(RAM_DL+4,CLEAR(1,1,1));

	wr32(RAM_DL+8,DISPLAY());

	usleep(100);

	wr8(REG_DLSWAP,DLSWAP_FRAME);//display list swap

	//wr8(REG_GPIO_DIR,0x80 | rd8(REG_GPIO_DIR));
	//wr8(REG_GPIO,0x080 | rd8(REG_GPIO));//enable display bit
	//wr8(REG_GPIO,0x80);
	wr8(REG_GPIO_DIR, 0xfc);
	wr8(REG_GPIO, 0xff);

	wr8(REG_PCLK,5); // clock prescaler (0: disable, >0: 48MHz/pclock)

	set_screen_rotation(STARTING_SCREEN_ROTATION);

	usleep(50);

	start_screen(CLEAR_COLOR_RGB(0,0,0));
	end_screen();

	wr8(REG_PWM_DUTY, 255);
	usleep(10000);

	return;
}


// ------------------------------ SCREEN MAKE FUCNTIONS ------------------------------


int wake_screen()
{
	writepin_gpio1(gpio1_13,1);
	usleep(2000);    // 20 millisecond delay
	writepin_gpio1(gpio1_13,0);
	usleep(2000);    // 20 millisecond delay
	writepin_gpio1(gpio1_13,1);
	usleep(2000);    // 20 millisecond delay


	host_command(ACTIVE); //send host command "ACTIVE" to FT800
	host_command(CLKEXT); //send command to "CLKEXT" to FT800
  	host_command(CLK48M);

  	usleep(2000);
  	reg_id = rd8(REG_ID);

  	if (reg_id != 0x7C)
  	{
  		DEBUGF("FT800 could not be initialised! BAD ID\r\n");
  		return -1;
  	}
  	return 0;
}

void power_down_screen()
{
  host_command(PWRDOWN);
}

void start_screen(UINT32 clear_screen_color)
{
  wr32(RAM_CMD + cmd_offset, (CMD_DLSTART));
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, clear_screen_color);
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, CLEAR(1, 1, 1)); // clear screen
  cmd_increment4();
  //save_context();
}

void end_screen()
{
  wr32(RAM_CMD + cmd_offset, DISPLAY()); // display the image
  cmd_increment4();
  // Bug removed!!
  //wr32(REG_CMD_WRITE, (cmd_offset));
  // wr8(REG_DLSWAP,DLSWAP_FRAME);//display list swap
  usleep(100);

  wr32(RAM_CMD + cmd_offset, (CMD_SWAP));
  cmd_increment4();

  wr32(REG_CMD_WRITE, (cmd_offset));
}

void save_context()    // does not work with bitmaps
{
  wr32(RAM_CMD + cmd_offset, SAVE_CONTEXT());
  cmd_increment4();
}

void restore_context()    // does not work with bitmaps
{
  wr32(RAM_CMD + cmd_offset, RESTORE_CONTEXT());
  cmd_increment4();
}

void restore_save_context()
{
  restore_context();
  save_context();
}

void make_rect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2,
                UINT32 color, UINT32 line_width)
// with x and y coordinates. Everything has to be 16 times what you want.
// for example if you wanted something to start at pixel 10, 10 would need to times it by 16 .. ie  160, 160
{
    wr32(RAM_CMD + cmd_offset, color);
    cmd_increment4();
    wr32(RAM_CMD + cmd_offset, LINE_WIDTH(line_width));
    cmd_increment4();
    wr32(RAM_CMD + cmd_offset, BEGIN(RECTS));
    cmd_increment4();
    wr32(RAM_CMD + cmd_offset, VERTEX2F(x1*16,y1*16));
    cmd_increment4();
    wr32(RAM_CMD + cmd_offset, VERTEX2F(x2*16,y2*16));
    cmd_increment4();
    wr32(RAM_CMD + cmd_offset, END());
    cmd_increment4();
}


void make_point(UINT16 x1, UINT16 y1, UINT32 color, UINT16 point_size)
{
  wr32(RAM_CMD + cmd_offset, color);
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, POINT_SIZE(point_size));
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, BEGIN(POINTS));
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(x1, y1, 0, 0));
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, END());
  cmd_increment4();
}


void make_string(UINT16 x1, UINT16 y1, UINT8 font_size,
                  UINT16 options, UINT32 color, char *name)
{
  UINT16 string_size = strlen(name);

  wr32(RAM_CMD + cmd_offset, color);
  cmd_increment4();
  cmd_text(x1,y1,font_size,options, name);
  wr32(RAM_CMD + cmd_offset, END());  // needed after
  cmd_increment4();
}

void make_number(UINT16 x1, UINT16 y1, UINT8 font_size,
                  UINT16 options, UINT32 color, UINT32 number)
{
  wr32(RAM_CMD + cmd_offset, color);
  cmd_increment4();

  cmd_number(x1, y1, font_size, options, number);
  wr32(RAM_CMD + cmd_offset, END());  // needed after cmd_number
  cmd_increment4();
}


void make_button(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                  UINT8 font_size, UINT16 options, UINT32 color,
                    char *name)
{
  cmd_fgcolor(color);
  cmd_button(x1, y1, w, h, font_size, options, name);
  wr32(RAM_CMD + cmd_offset, END());  // needed after cmd_
  cmd_increment4();
}


/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *if color's == 0 then cmd not executed
**/
void make_progress(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range,
                        UINT32 background_color, UINT32 foreground_color)
{
 if((foreground_color)) { cmd_color_rgb(foreground_color); }
  if ((background_color)) { cmd_bgcolor(background_color);}

  cmd_progress(x1, y1, w, h, options, val, range);

  wr32(RAM_CMD + cmd_offset, END());  // needed after cmd_
  cmd_increment4();
}

/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *  // if color's == 0 then cmd not executed
**/
void make_slider(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range,
                        UINT32 background_color, UINT32 foreground_color)
{
  if((foreground_color)) { cmd_color_rgb(foreground_color); }
  if ((background_color)) { cmd_bgcolor(background_color);}

  cmd_slider(x1, y1, w, h, options, val, range);
  wr32(RAM_CMD + cmd_offset, END());  // needed after cmd_
  cmd_increment4();
}

/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *if color's == 0 then cmd not executed
**/
void make_scrollbar(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range, UINT16 size,
                        UINT32 background_color, UINT32 foreground_color)
{
 if((foreground_color)) { cmd_color_rgb(foreground_color); }
  if ((background_color)) { cmd_bgcolor(background_color);}

  cmd_scrollbar(x1, y1, w, h, options, val, size, range);
  wr32(RAM_CMD + cmd_offset, END());  // needed after cmd_
  cmd_increment4();
}


void cmd_color_rgb(UINT32 color)
{
  wr32(RAM_CMD + cmd_offset, color);  // needed after cmd_
  cmd_increment4();
}


void set_lcd_brightness(double val)
{
  val = (int)val & 0xFF;
  wr8(REG_PWM_DUTY, (UINT8)val);
}

UINT8 get_lcd_brightness()
{
  return rd8(REG_PWM_DUTY);
}

void set_screen_rotation(UINT32 bit_0)
{
  UINT32 temp = rd32(REG_ROTATE);
  temp = temp & (UINT32)0xFFFFFFFE;
  bit_0 = bit_0 & 0x00000001;
  temp = temp | bit_0;
  wr32(REG_ROTATE, temp);
}

UINT8 get_screen_rotation()
{
  UINT32 temp = rd32(REG_ROTATE);
  temp = temp & 0x000000001;

  return (UINT8)(temp & 0xFF);
}
