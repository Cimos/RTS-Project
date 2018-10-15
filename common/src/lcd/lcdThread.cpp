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

#define LCD2_MAIN_MENU  0
#define LCD2_PREVIOUS   1
#define LCD2_FILE_1     2
#define LCD2_FILE_2     3
#define LCD2_FILE_3     4
#define LCD2_FILE_4     5
#define LCD2_FILE_5     6
#define LCD2_FILE_6     7
struct
{
  struct{
    char file_playing[255];
    char dir_name[6][255];
    UINT8 selected;
    UINT32 selected_text_color[8];
  }LCD_SCREEN_2;

  struct{
    bool selected;
    UINT32 back_text_color;
    UINT8 selector;
    UINT8 eq_bands[8];
    UINT32 eq_band_color[7];

  }eq_screen;


  struct{
   UINT32 screen_rotation;
   UINT8 screen_brightness;   //MAX brightness = 128
   UINT8 selector;
   UINT8 selected;
   UINT32 opt_screen_selector_color[4];

  }opt_screen;


}_self;



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

int lcdInit(_self_i2c *self)
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




void splash_screen()
{
  cmd(CMD_DLSTART);
  wr32(RAM_CMD + cmd_offset, CLEAR(1, 1, 1)); // clear screen
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, BEGIN(BITMAPS)); // start drawing bitmaps
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(244, 110, 31, 'T')); // ascii T
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(270, 110, 31, 'D')); // ascii D
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(299, 110, 31, 'I')); // ascii I
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, END());
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, COLOR_RGB(255, 0, 0)); // change color to red
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, POINT_SIZE(320)); // set point size to 20 pixels in radius
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, BEGIN(POINTS)); // start drawing points
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, VERTEX2II(192, 133, 0, 0)); // red point
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, END());
  cmd_increment4();
  wr32(RAM_CMD + cmd_offset, DISPLAY()); // display the image
  cmd_increment4();
  wr32(REG_CMD_WRITE, (cmd_offset));
  wr8(REG_DLSWAP,DLSWAP_FRAME);//display list swap
  wr32(RAM_CMD + cmd_offset, (CMD_SWAP));
  cmd_increment4();
  wr32(REG_CMD_WRITE, (cmd_offset));
}


// 240 *2
// 136 * 2
void splash_screen2()
{
	for (int i = 1; i < 255; i+=5)
	{

		start_screen(CLEAR_COLOR_RGB(0,0,0));
		make_string(240,106,31,OPT_CENTER,COLOR_RGB(i,i,i), "RTS Project 2018");
		make_string(240,166,22,OPT_CENTER,COLOR_RGB(i,i,i), "Simon Maddison, Shawn Buschmann, Michael Stekla");
		end_screen();

		usleep(10000);
	}

}






void lcd_2()
{
    start_screen(CLEAR_COLOR_RGB(255,255,255));

    // make blue rectangle
    make_rect(0,73,478,231,COLOR_RGB(29,105,175),16);
    save_context();

    // write file browse location
    make_string(10,53,28,OPT_CENTERY,COLOR_RGB(11,11,11),_self.LCD_SCREEN_2.file_playing);
    restore_save_context();

    make_rect(0,0,480,37,COLOR_RGB(29,105,175),16);
    restore_save_context();

  // make green rectangle  on main_menu

    for(int i = 0; i <8; i++) { _self.LCD_SCREEN_2.selected_text_color[i] = COLOR_RGB(255,255,255); }

    switch(_self.LCD_SCREEN_2.selected)
    {
        case LCD2_MAIN_MENU:
            _self.LCD_SCREEN_2.selected_text_color[0] = COLOR_RGB(9,214,22);
//          make_rect(5,73,100,93,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_PREVIOUS:
            _self.LCD_SCREEN_2.selected_text_color[1] = COLOR_RGB(9,214,22);
//          make_rect(0,73,100,93,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;
        case LCD2_FILE_1:

            _self.LCD_SCREEN_2.selected_text_color[2] = COLOR_RGB(9,214,22);
//          make_rect(0,100,100,93,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_FILE_2:
            _self.LCD_SCREEN_2.selected_text_color[3] = COLOR_RGB(9,214,22);
//          make_rect(0,120,100,113,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_FILE_3:
            _self.LCD_SCREEN_2.selected_text_color[4] = COLOR_RGB(9,214,22);
//          make_rect(0,140,100,133,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_FILE_4:
            _self.LCD_SCREEN_2.selected_text_color[5] = COLOR_RGB(9,214,22);
//          make_rect(0,160,100,153,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_FILE_5:
            _self.LCD_SCREEN_2.selected_text_color[6] = COLOR_RGB(9,214,22);
//          make_rect(0,180,100,173,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;

        case LCD2_FILE_6:
            _self.LCD_SCREEN_2.selected_text_color[7] = COLOR_RGB(9,214,22);
//          make_rect(0,220,100,193,COLOR_RGB(9,214,22),16);
//          restore_save_context();
          break;
    }

    // make main menu string
      make_string(10,70,28,0,_self.LCD_SCREEN_2.selected_text_color[0],"Main Menu");
      restore_save_context();

      //write previous
      make_string(10,90,28,0,_self.LCD_SCREEN_2.selected_text_color[1],"Previous");
      restore_save_context();

      make_string(10,110,28,0,_self.LCD_SCREEN_2.selected_text_color[2],_self.LCD_SCREEN_2.dir_name[0]);
      restore_save_context();

      make_string(10,130,28,0,_self.LCD_SCREEN_2.selected_text_color[3],_self.LCD_SCREEN_2.dir_name[1]);
      restore_save_context();

      make_string(10,150,28,0,_self.LCD_SCREEN_2.selected_text_color[4],_self.LCD_SCREEN_2.dir_name[2]);
      restore_save_context();

      make_string(10,170,28,0,_self.LCD_SCREEN_2.selected_text_color[5],_self.LCD_SCREEN_2.dir_name[3]);
      restore_save_context();

      make_string(10,190,28,0,_self.LCD_SCREEN_2.selected_text_color[6],_self.LCD_SCREEN_2.dir_name[4]);
      restore_save_context();

      make_string(10,210,28,0,_self.LCD_SCREEN_2.selected_text_color[7],_self.LCD_SCREEN_2.dir_name[5]);
      restore_save_context();


      make_string(242,252,30,OPT_CENTER,COLOR_RGB(0,0,0),"Innovative Solutions");
      restore_save_context();

      //make file browser text
      make_string(104,19,30,OPT_CENTER,COLOR_RGB(255,255,255),"File Browser");

      restore_context();
      end_screen();
  }

// ---------------------------------------------------------------------------
//							Local Functions Implementation
// ---------------------------------------------------------------------------





