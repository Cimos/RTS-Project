/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     FT800.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/



/*

Copyright (c) Future Technology Devices International 2014

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PAR NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Author : FTDI

Revision History:
0.1 - date 2013.04.24 - initial version

*/

#ifndef SRC_FT800_FT800_H_
#define	SRC_FT800_FT800_H_

/*-----------------------------------------------------------------------------
 * Included Files
 *---------------------------------------------------------------------------*/

//#include "portable.h"
#include "FT_DataTypes.h"
#include "FT_Gpu.h"

/*-----------------------------------------------------------------------------
 * Definitions * Global Variables Declarations
 *---------------------------------------------------------------------------*/
/**
 *	@breif screen settings
 *	@
 */
#define SCREEN_HSIZE_16 	480		// width resolution
#define SCREEN_VSIZE_16 	272		// height resolution
#define SCREEN_HCYCLE_16 	548		// number if horizontal cycles for display
#define SCREEN_HSYNC0_16 	0		// hsync falls
#define SCREEN_HSYNC1_16 	41		// hsync rise
#define SCREEN_HOFFSET_16 	43		// horizontal offset from starting signal
#define SCREEN_VCYCLE_16 	292		// number of vertical cycles for display
#define SCREEN_VSYNC0_16 	0		// vsync falls
#define SCREEN_VSYNC1_16 	10		// vsync rise
#define SCREEN_VOFFSET_16 	12		// vertical offset from start signal
#define SCREEN_CSPREAD_8 	1		// output clock spread enable
#define SCREEN_DITHER_8 	1		// output number of bits
#define SCREEN_SWIZZLE_8 	0               // output swizzle
#define SCREEN_PCLK             5              // 48M/5 =
#define SCREEN_PCLK_POL         1

#define STARTING_SCREEN_ROTATION  0

extern unsigned short dli;
/**
 * Write the specified 32 bits display list command to RAM_DL. (see 2.5.4)
 */
#define dl(cmd) wr32(RAM_DL + dli, cmd);dli += 4

extern unsigned short cmd_offset;
/*#define cmd_increment4() cmd_offset=(cmd_offset==4092 ? 0 : cmd_offset+4)
#define cmd_increment2() cmd_offset=(cmd_offset==4094 ? 0 : cmd_offset+2)*/
//#define cmd_incrementn(n) cmd_offset=(cmd_offset+n)%4096
void cmd_incrementn(unsigned char n);
#define cmd_increment4() cmd_incrementn(4)
#define cmd_increment2() cmd_incrementn(2)
//#define cmd(command) wr32(RAM_CMD + cmd_offset, command);cmd_increment4()
void cmd(ft_uint32_t command);
#define cmd_exec() wr16(REG_CMD_WRITE, cmd_offset)
#define cmd_wait() cmd_offset=rd16(REG_CMD_WRITE);while(rd16(REG_CMD_READ) != cmd_offset)


/**
 * start a new display list
 */
#define cmd_dlstart() cmd(CMD_DLSTART)

/**
 * swap the current display list
 */
#define cmd_swap() cmd(CMD_DLSWAP)

/**
 * draw text
 */
#define cmd_text(x, y, font, options, s) cmd(CMD_TEXT);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, font);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();cmd_incrementn(wr8s(RAM_CMD+cmd_offset, s))

/**
 * draw a decimal number
 */
#define cmd_number(x, y, font, options, n) cmd(CMD_NUMBER);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, font);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();cmd(n)

/**
 * draw a button
 */
#define cmd_button(x, y, w, h, font, options, s) cmd(CMD_BUTTON);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, font);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();cmd_incrementn(wr8s(RAM_CMD+cmd_offset, s))

/**
 * draw an analog clock
 */
#define cmd_clock(x, y, r, options, h, m, s, ms) cmd(CMD_CLOCK);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, r);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, m);cmd_increment2();wr16(RAM_CMD+cmd_offset, s);cmd_increment2();wr16(RAM_CMD+cmd_offset, ms);cmd_increment2()

/**
 * set the foreground color
 */
#define cmd_fgcolor(c) cmd(CMD_FGCOLOR);cmd(c)

/**
 * set the background color
 */
#define cmd_bgcolor(c) cmd(CMD_BGCOLOR);cmd(c)

/**
 * set the 3D effects for CMD_BUTTON and CMD_KEYS highlight color
 */
#define cmd_gradcolor(c) cmd(CMD_GRADCOLOR);cmd(c)

/**
 * draw a gauge
 */
#define cmd_gauge(x, y, r, options, major, minor, val, range) cmd(CMD_GAUGE);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, r);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, major);cmd_increment2();wr16(RAM_CMD+cmd_offset, minor);cmd_increment2();wr16(RAM_CMD+cmd_offset, val);cmd_increment2();wr16(RAM_CMD+cmd_offset, range);cmd_increment2()

/**
 * draw a draw a smooth color gradient
 */
#define cmd_gradient(x0, y0, rgb0, x1, y1, rgb1) cmd(CMD_GRADIENT);wr16(RAM_CMD+cmd_offset, x0);cmd_increment2();wr16(RAM_CMD+cmd_offset, y0);cmd_increment2();cmd(rgb0);wr16(RAM_CMD+cmd_offset, x1);cmd_increment2();wr16(RAM_CMD+cmd_offset, y1);cmd_increment2();cmd(rgb1)

/**
 * draw a row of keys
 */
#define cmd_keys(x, y, w, h, font, options, s) cmd(CMD_KEYS);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, font);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();cmd_incrementn(wr8s(RAM_CMD+cmd_offset, s))

/**
 * draw a progress bar
 */
#define cmd_progress(x, y, w, h, options, val, range) cmd(CMD_PROGRESS);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, val);cmd_increment2();wr16(RAM_CMD+cmd_offset, range);cmd_increment4()

/**
 * draw a draw a scroll bar
 */
#define cmd_scrollbar(x, y, w, h, options, val, size, range) cmd(CMD_SCROLLBAR);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, val);cmd_increment2();wr16(RAM_CMD+cmd_offset, size);cmd_increment2();wr16(RAM_CMD+cmd_offset, range);cmd_increment2()

/**
 * draw a slider
 */
#define cmd_slider(x, y, w, h, options, val, range) cmd(CMD_SLIDER);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, val);cmd_increment2();wr16(RAM_CMD+cmd_offset, range);cmd_increment4()

/**
 * draw a rotary dial control
 */
#define cmd_dial(x, y, r, options, val) cmd(CMD_DIAL);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, r);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, val)

/**
 * draw a toggle switch
 */
#define cmd_toggle(x, y, w, font, options, state, s) cmd(CMD_TOGGLE);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, font);cmd_increment2();wr16(RAM_CMD+cmd_offset, options);cmd_increment2();wr16(RAM_CMD+cmd_offset, state);cmd_incrementn(wr8s(RAM_CMD+cmd_offset, s))

/**
 * compute a CRC-32 for memory
 */
#define cmd_memcrc(ptr, num, result) cmd(CMD_MEMCRC);cmd(ptr);cmd(num);unsigned short cmd_memcrc_result_ptr=cmd_offset;cmd(result);cmd_wait();result=rd32(cmd_memcrc_result_ptr)

/**
 * write zero to a block of memory
 */
#define cmd_memzero(ptr, num) cmd(CMD_MEMCRC);cmd(ptr);cmd(num)

/**
 * fill memory with a byte value
 */
#define cmd_memset(ptr, value, num) cmd(CMD_MEMCRC);cmd(ptr);cmd(value);cmd(num)

/**
 * write bytes into memory
 */
#define cmd_memwrite(ptr, num)      cmd(CMD_MEMWRITE);cmd(ptr);cmd(num)

/**
 * copy a block of memory
 */
#define cmd_memcpy(dest, src, num) cmd(CMD_MEMCRC);cmd(dest);cmd(src);cmd(num)

/**
 * append memory to display list
 */
#define cmd_append(ptr, num)        cmd(CMD_APPEND);cmd(ptr);cmd(num)

/**
 * decompress data into memory
 */
#define cmd_inflate(ptr) cmd(CMD_INFLATE);cmd(ptr)

/**
 * set the current matrix to identity
 */
#define cmd_loadidentity() cmd(CMD_LOADIDENTITY)

/**
 * write the current matrix as a bitmap transform
 */
#define cmd_setmatrix() cmd(CMD_SETMATRIX)

/**
 * retrieves the current matrix coefficients
 */
#define cmd_getmatrix(a, b, c, d, e, f) cmd(CMD_GETMATRIX);unsigned short cmd_getmatrix_result_ptr=cmd_offset;cmd(0);cmd(0);cmd(0);cmd(0);cmd(0);cmd(0);cmd_exec();cmd_wait();a=rd32(cmd_getmatrix_result_ptr);b=rd32((cmd_getmatrix_result_ptr+4)%4096);c=rd32((cmd_getmatrix_result_ptr+8)%4096);d=rd32((cmd_getmatrix_result_ptr+12)%4096);e=rd32((cmd_getmatrix_result_ptr+16)%4096);f=rd32((cmd_getmatrix_result_ptr+20)%4096)

/**
 * Get the end memory address of inflated data
 */
#define cmd_getptr(result) cmd(CMD_GETPTR);unsigned short cmd_getptr_result_ptr=cmd_offset;cmd(0);cmd_wait();result=rd32(cmd_getptr_result_ptr)

/**
 * apply a scale to the current matrix
 */
#define cmd_scale(sx, sy) cmd(CMD_SCALE);cmd(sx);cmd(sy)

/**
 * apply a rotation to the current matrix
 */
#define cmd_rotate(a) cmd(CMD_ROTATE);cmd(a)

/**
 * apply a translation to the current matrix
 */
#define cmd_translate(tx, ty) cmd(CMD_TRANSLATE);cmd(tx);cmd(ty)

/**
 * load a JPEG image
 */
#define cmd_loadimage(ptr, options) cmd(CMD_LOADIMAGE);cmd(ptr);cmd(options)

/**
 * set co-processor engine state to default values
 */
#define cmd_coldstart()             cmd(CMD_COLDSTART)

/**
 * trigger interrupt INT_CMDFLAG
 */
#define cmd_interrupt(ms)           cmd(CMD_INTERRUPT);cmd(ms)

/**
 * read a register value
 */
#define cmd_regread(ptr, result)    cmd(CMD_REGREAD);cmd(ptr);cmd(result)

/**
 * execute the touch screen calibration routine
 */
#define cmd_calibrate() cmd(CMD_CALIBRATE);unsigned short cmd_calibrate_result_ptr=cmd_offset;cmd(0)
#define cmd_calibrate_result() rd32(cmd_calibrate_result_ptr)

/**
 * start an animated spinner
 */
#define cmd_spinner(x, y, style, scale) cmd(CMD_SPINNER);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, style);cmd_increment2();wr16(RAM_CMD+cmd_offset, scale);cmd_increment2()

/**
 * start an animated screensaver
 */
#define cmd_screensaver() cmd(CMD_SCREENSAVER)

/**
 * start a continuous sketch update
 */
#define cmd_sketch(x, y, w, h, ptr, format) cmd(CMD_SKETCH);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();cmd(ptr);wr16(RAM_CMD+cmd_offset, format);cmd_increment2()

/**
 * stop any spinner, screensaver or sketch
 */
#define cmd_stop() cmd(CMD_STOP)

/**
 * set up a custom font
 */
#define cmd_setfont(font, ptr) cmd(CMD_SETFONT);cmd(font);cmd(ptr)

/**
 * track touches for a graphics object
 */
#define cmd_track(x, y, w, h, tag) cmd(CMD_TRACK);wr16(RAM_CMD+cmd_offset, x);cmd_increment2();wr16(RAM_CMD+cmd_offset, y);cmd_increment2();wr16(RAM_CMD+cmd_offset, w);cmd_increment2();wr16(RAM_CMD+cmd_offset, h);cmd_increment2();wr16(RAM_CMD+cmd_offset, tag);cmd_increment2()

/**
 * take a snapshot of the current screen
 */
#define cmd_snapshot(ptr) cmd(CMD_SNAPSHOT);cmd(ptr)

/**
 * play device logo animation
 */
#define cmd_logo() cmd(CMD_LOGO)


/*-----------------------------------------------------------------------------
 * Global Function Declarations
 *---------------------------------------------------------------------------*/

/**
 * @brief play device logo animation
 * @param command
 */
void host_command(ft_uint8_t command);

/**
 * @brief write 8 bits to intended address location
 * @param addr, value
 */
void wr8(UINT32 addr, ft_uint8_t value);

/**
 * @brief write 16 bits to intended address location
 * @param addr, value
 */
void wr16(UINT32, ft_uint16_t value);

/**
 * @brief write 32 bits to intended address location
 * @param addr, value
 */
void wr32(UINT32, ft_uint32_t value);

/**
 * @brief read 8 bits from intended address location
 * @param addr
 */
ft_uint8_t rd8(UINT32 addr);

/**
 * @brief read 16 bits from intended address location
 * @param addr
 */
ft_uint16_t rd16(UINT32 addr);

/**
 * @brief read 32 bits from intended address location
 * @param addr
 */
ft_uint32_t rd32(UINT32 addr);

/**
 * @brief write 8 bits string to intended address location
 * @param addr *s
 */
ft_uint8_t wr8s(UINT32 addr, ft_char8_t *s);

/**
 * @brief read 8 bits string from intended address location
 * @param  addr, len
 */
//ft_char8_t * rd8s(unsigned short addr, ft_uint8_t len);


/**
 * @brief increments cmd_offset by n
 * @param
 */
unsigned int incCMDOffset(unsigned int currentOffset, unsigned char commandSize);

/**
 * @brief increments cmd_offset by n
 * @param
 */
void cmd_incrementn(unsigned char n);

/**
 * @brief writes a command to RAM_DL with dli offset and adds 4 to dli
 * @param
 */
void dl_cmd(unsigned long cmd);

/**
 * @brief writes a command to RAM_CMD with cmd_offset and adds 4 to cmd_offset
 * @param
 */
void cmd(ft_uint32_t command);

/**
 * @brief closes i2c port for the LCD
 * @param
 */
void i2c_LCD_port_CLOSE();

/**
 * @brief opens i2c port for the LCD
 * @param
 */
void i2c_LCD_port_OPEN();

/**
 * @brief wakes up the LCD
 * @param
 */
ft_bool_t lcd_wake_up();

/**
 * @brief Initializes the FT800
 * @param
 */
void FT800_Init(void);



/*-----------------------------------------------------------------------------
 * Global Variables Declarations
 *---------------------------------------------------------------------------*/

void make_rect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2, UINT32 color, UINT32 line_width);
void make_point(UINT16 x1, UINT16 y1, UINT32 color, UINT16 point_size);
void make_string(UINT16 x1, UINT16 y1, UINT8 font_size, UINT16 option, UINT32 color, char *name);
void make_number(UINT16 x1, UINT16 y1, UINT8 font_size, UINT16 option, UINT32 color, UINT32 number);
void make_button(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h, UINT8 font_size, UINT16 option, UINT32 color, char *name);
void make_progress(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h, UINT16 options, UINT16 val, UINT16 range, UINT32 background_color, UINT32 foreground_color);
//void splash_screen();
void start_screen(UINT32 clear_screen_color);
void end_screen();
void save_context();
void restore_context();
void restore_save_context();
void power_down_screen();
int  wake_screen();
void reset_screen();



/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *if color's == 0 then cmd not executed
**/
void make_progress(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range,
                        UINT32 background_color, UINT32 foreground_color);


/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *  // if color's == 0 then cmd not executed
**/
void make_slider(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range,
                        UINT32 background_color, UINT32 foreground_color);

/*
 * Height of scroll bar, in pixels.
 * If height is greater than width, the scroll bar is drawn vertically
 *if color's == 0 then cmd not executed
**/
void make_scrollbar(UINT16 x1, UINT16 y1, UINT16 w, UINT16 h,
                      UINT16 options, UINT16 val, UINT16 range, UINT16 size,
                        UINT32 background_color, UINT32 foreground_color);


void cmd_color_rgb(UINT32 color);


void set_lcd_brightness(double val);
UINT8 get_lcd_brightness();
UINT8 get_screen_rotation();

#endif /* SRC_FT800_FT800_H_ */
