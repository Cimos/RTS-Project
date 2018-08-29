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

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Author : FTDI

Revision History:
0.1 - date 2013.04.24 - initial version

*/
#ifndef SRC_FT800_FT_GPU_H_
#define SRC_FT800_FT_GPU_H_

/* Definitions used for FT800 co processor command buffer */
#define FT_DL_SIZE           (8*1024)  //8KB Display List buffer size
#define FT_CMD_FIFO_SIZE     (4*1024)  //4KB coprocessor Fifo size
#define FT_CMD_SIZE          (4)       //4 byte per coprocessor command of EVE

#define FT800_VERSION "1.9.0"
#define ADC_DIFFERENTIAL     1UL
#define ADC_SINGLE_ENDED     0UL
#define ADPCM_SAMPLES        2UL
#define ALWAYS               7UL
#define ARGB1555             0UL
#define ARGB2                5UL
#define ARGB4                6UL
#define BARGRAPH             11UL
#define BILINEAR             1UL
#define BORDER               0UL

// section 4.2
#define BITMAPS              1UL
#define POINTS               2UL
#define LINES               3UL
#define LINE_STRIP          4UL
#define EDGE_R              5UL
#define EDGE_L              6UL
#define EDGE_A              7UL
#define EDGE_B              8UL
#define RECTS               9UL

#define CMDBUF_SIZE          4096UL
#define CMD_APPEND           4294967070UL
#define CMD_BGCOLOR          4294967049UL
#define CMD_BITMAP_TRANSFORM 4294967073UL
#define CMD_BUTTON           4294967053UL
#define CMD_CALIBRATE        4294967061UL
#define CMD_CLOCK            4294967060UL
#define CMD_COLDSTART        4294967090UL
#define CMD_CRC              4294967043UL
#define CMD_DIAL             4294967085UL
#define CMD_DLSTART          4294967040UL
#define CMD_DLSWAP           4294967041UL
#define CMD_EXECUTE          4294967047UL
#define CMD_FGCOLOR          4294967050UL
#define CMD_GAUGE            4294967059UL
#define CMD_GETMATRIX        4294967091UL
#define CMD_GETPOINT         4294967048UL
#define CMD_GETPROPS         4294967077UL
#define CMD_GETPTR           4294967075UL
#define CMD_GRADCOLOR        4294967092UL
#define CMD_GRADIENT         4294967051UL
#define CMD_HAMMERAUX        4294967044UL
#define CMD_IDCT             4294967046UL
#define CMD_INFLATE          4294967074UL
#define CMD_INTERRUPT        4294967042UL
#define CMD_KEYS             4294967054UL
#define CMD_LOADIDENTITY     4294967078UL
#define CMD_LOADIMAGE        4294967076UL
#define CMD_LOGO             4294967089UL
#define CMD_MARCH            4294967045UL
#define CMD_MEMCPY           4294967069UL
#define CMD_MEMCRC           4294967064UL
#define CMD_MEMSET           4294967067UL
#define CMD_MEMWRITE         4294967066UL
#define CMD_MEMZERO          4294967068UL
#define CMD_NUMBER           4294967086UL
#define CMD_PROGRESS         4294967055UL
#define CMD_REGREAD          4294967065UL
#define CMD_ROTATE           4294967081UL
#define CMD_SCALE            4294967080UL
#define CMD_SCREENSAVER      4294967087UL
#define CMD_SCROLLBAR        4294967057UL
#define CMD_SETFONT          4294967083UL
#define CMD_SETMATRIX        4294967082UL
#define CMD_SKETCH           4294967088UL
#define CMD_SLIDER           4294967056UL
#define CMD_SNAPSHOT         4294967071UL
#define CMD_SPINNER          4294967062UL
#define CMD_STOP             4294967063UL
#define CMD_SWAP             4294967041UL
#define CMD_TEXT             4294967052UL
#define CMD_TOGGLE           4294967058UL
#define CMD_TOUCH_TRANSFORM  4294967072UL
#define CMD_TRACK            4294967084UL
#define CMD_TRANSLATE        4294967079UL

#define DECR                 4UL
#define DECR_WRAP            7UL
#define DLSWAP_DONE          0UL
#define DLSWAP_FRAME         2UL
#define DLSWAP_LINE          1UL
#define DST_ALPHA            3UL
#define EQUAL                5UL
#define GEQUAL               4UL
#define GREATER              3UL
#define INCR                 3UL
#define INCR_WRAP            6UL
#define INT_CMDEMPTY         32UL
#define INT_CMDFLAG          64UL
#define INT_CONVCOMPLETE     128UL
#define INT_PLAYBACK         16UL
#define INT_SOUND            8UL
#define INT_SWAP             1UL
#define INT_TAG              4UL
#define INT_TOUCH            2UL
#define INVERT               5UL

#define KEEP                 1UL
#define L1                   1UL
#define L4                   2UL
#define L8                   3UL
#define LEQUAL               2UL
#define LESS                 1UL
#define LINEAR_SAMPLES       0UL
#define NEAREST              0UL
#define NEVER                0UL
#define NOTEQUAL             6UL
#define ONE                  1UL
#define ONE_MINUS_DST_ALPHA  5UL
#define ONE_MINUS_SRC_ALPHA  4UL
#define OPT_CENTER           1536UL
#define OPT_CENTERX          512UL
#define OPT_CENTERY          1024UL
#define OPT_FLAT             256UL
#define OPT_MONO             1UL
#define OPT_NOBACK           4096UL
#define OPT_NODL             2UL
#define OPT_NOHANDS          49152UL
#define OPT_NOHM             16384UL
#define OPT_NOPOINTER        16384UL
#define OPT_NOSECS           32768UL
#define OPT_NOTICKS          8192UL
#define OPT_RIGHTX           2048UL
#define OPT_SIGNED           256UL
#define PALETTED             8UL

#define RAM_G                0UL
#define ROM_CHIPID           786432UL
#define ROM_FONT             766524UL
#define ROM_FONT_ADDR        1048572UL
#define RAM_DL               1048576UL
#define RAM_PAL              1056768UL
#define RAM_REG              1057792UL
#define RAM_CMD              1081344UL



#define REG_ANALOG           1058104UL
#define REG_ANA_COMP         1058160UL
#define REG_CLOCK            1057800UL
#define REG_CMD_DL           1058028UL
#define REG_CMD_READ         1058020UL
#define REG_CMD_WRITE        1058024UL
#define REG_CPURESET         1057820UL
#define REG_CRC              1058152UL
#define REG_CYA0             1058000UL
#define REG_CYA1             1058004UL
#define REG_CYA_TOUCH        1058100UL
#define REG_DATESTAMP        1058108UL
#define REG_FRAMES           1057796UL
#define REG_FREQUENCY        1057804UL
#define REG_GPIO             1057936UL
#define REG_GPIO_DIR         1057932UL

#define REG_HCYCLE           1057832UL
#define REG_HOFFSET          1057836UL
#define REG_HSIZE            1057840UL
#define REG_HSYNC0           1057844UL
#define REG_HSYNC1           1057848UL
#define REG_VCYCLE           1057852UL
#define REG_VOFFSET          1057856UL
#define REG_VSIZE            1057860UL
#define REG_VSYNC0           1057864UL
#define REG_VSYNC1           1057868UL
#define REG_DLSWAP           1057872UL
#define REG_ROTATE           1057876UL
#define REG_OUTBITS          1057880UL
#define REG_DITHER           1057884UL
#define REG_SWIZZLE          1057888UL
#define REG_CSPREAD          1057892UL
#define REG_PCLK_POL         1057896UL
#define REG_PCLK             1057900UL

#define REG_ID               1057792UL
#define REG_INT_EN           1057948UL
#define REG_INT_FLAGS        1057944UL
#define REG_INT_MASK         1057952UL
#define REG_MACRO_0          1057992UL
#define REG_MACRO_1          1057996UL
#define REG_PLAY             1057928UL
#define REG_PLAYBACK_FORMAT  1057972UL
#define REG_PLAYBACK_FREQ    1057968UL
#define REG_PLAYBACK_LENGTH  1057960UL
#define REG_PLAYBACK_LOOP    1057976UL
#define REG_PLAYBACK_PLAY    1057980UL
#define REG_PLAYBACK_READPTR 1057964UL
#define REG_PLAYBACK_START   1057956UL
#define REG_PWM_DUTY         1057988UL
#define REG_PWM_HZ           1057984UL
#define REG_RENDERMODE       1057808UL
#define REG_ROMSUB_SEL       1058016UL
#define REG_SNAPSHOT         1057816UL
#define REG_SNAPY            1057812UL
#define REG_SOUND            1057924UL
#define REG_TAG              1057912UL
#define REG_TAG_X            1057904UL
#define REG_TAG_Y            1057908UL
#define REG_TAP_CRC          1057824UL
#define REG_TAP_MASK         1057828UL
#define REG_TOUCH_ADC_MODE   1058036UL
#define REG_TOUCH_CHARGE     1058040UL
#define REG_TOUCH_DIRECT_XY  1058164UL
#define REG_TOUCH_DIRECT_Z1Z2 1058168UL
#define REG_TOUCH_MODE       1058032UL
#define REG_TOUCH_OVERSAMPLE 1058048UL
#define REG_TOUCH_RAW_XY     1058056UL
#define REG_TOUCH_RZ         1058060UL
#define REG_TOUCH_RZTHRESH   1058052UL
#define REG_TOUCH_SCREEN_XY  1058064UL
#define REG_TOUCH_SETTLE     1058044UL
#define REG_TOUCH_TAG        1058072UL
#define REG_TOUCH_TAG_XY     1058068UL
#define REG_TOUCH_TRANSFORM_A 1058076UL
#define REG_TOUCH_TRANSFORM_B 1058080UL
#define REG_TOUCH_TRANSFORM_C 1058084UL
#define REG_TOUCH_TRANSFORM_D 1058088UL
#define REG_TOUCH_TRANSFORM_E 1058092UL
#define REG_TOUCH_TRANSFORM_F 1058096UL
#define REG_TRACKER          1085440UL
#define REG_TRIM             1058156UL
#define REG_VOL_PB           1057916UL
#define REG_VOL_SOUND        1057920UL


#define REPEAT               1UL
#define REPLACE              2UL
#define RGB332               4UL
#define RGB565               7UL
#define SRC_ALPHA            2UL
#define TEXT8X8              9UL
#define TEXTVGA              10UL
#define TOUCHMODE_CONTINUOUS 3UL
#define TOUCHMODE_FRAME      2UL
#define TOUCHMODE_OFF        0UL
#define TOUCHMODE_ONESHOT    1UL
#define ULAW_SAMPLES         1UL
#define ZERO                 0UL


#define VERTEX2F(x,y) ((1UL<<30)|(((x)&32767UL)<<15)|(((y)&32767UL)<<0))
#define VERTEX2II(x,y,handle,cell) ((2UL<<30)|(((x)&511UL)<<21)|(((y)&511UL)<<12)|(((handle)&31UL)<<7)|(((cell)&127UL)<<0))
#define BITMAP_SOURCE(addr) ((1UL<<24)|(((addr)&1048575UL)<<0))
#define CLEAR_COLOR_RGB(red,green,blue) ((2UL<<24)|(((red)&255UL)<<16)|(((green)&255UL)<<8)|(((blue)&255UL)<<0))
#define TAG(s) ((3UL<<24)|(((s)&255UL)<<0))
#define COLOR_RGB(red,green,blue) ((4UL<<24)|(((red)&255UL)<<16)|(((green)&255UL)<<8)|(((blue)&255UL)<<0))
#define BITMAP_HANDLE(handle) ((5UL<<24)|(((handle)&31UL)<<0))
#define CELL(cell) ((6UL<<24)|(((cell)&127UL)<<0))
#define BITMAP_LAYOUT(format,linestride,height) ((7UL<<24)|(((format)&31UL)<<19)|(((linestride)&1023UL)<<9)|(((height)&511UL)<<0))
#define BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((8UL<<24)|(((filter)&1UL)<<20)|(((wrapx)&1UL)<<19)|(((wrapy)&1UL)<<18)|(((width)&511UL)<<9)|(((height)&511UL)<<0))
#define ALPHA_FUNC(func,ref) ((9UL<<24)|(((func)&7UL)<<8)|(((ref)&255UL)<<0))
#define STENCIL_FUNC(func,ref,mask) ((10UL<<24)|(((func)&7UL)<<16)|(((ref)&255UL)<<8)|(((mask)&255UL)<<0))
#define BLEND_FUNC(src,dst) ((11UL<<24)|(((src)&7UL)<<3)|(((dst)&7UL)<<0))
#define STENCIL_OP(sfail,spass) ((12UL<<24)|(((sfail)&7UL)<<3)|(((spass)&7UL)<<0))
#define POINT_SIZE(size) ((13UL<<24)|(((size)&8191UL)<<0))
#define LINE_WIDTH(width) ((14UL<<24)|(((width)&4095UL)<<0))
#define CLEAR_COLOR_A(alpha) ((15UL<<24)|(((alpha)&255UL)<<0))
#define COLOR_A(alpha) ((16UL<<24)|(((alpha)&255UL)<<0))
#define CLEAR_STENCIL(s) ((17UL<<24)|(((s)&255UL)<<0))
#define CLEAR_TAG(s) ((18UL<<24)|(((s)&255UL)<<0))
#define STENCIL_MASK(mask) ((19UL<<24)|(((mask)&255UL)<<0))
#define TAG_MASK(mask) ((20UL<<24)|(((mask)&1UL)<<0))
#define BITMAP_TRANSFORM_A(a) ((21UL<<24)|(((a)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(b) ((22UL<<24)|(((b)&131071UL)<<0))
#define BITMAP_TRANSFORM_C(c) ((23UL<<24)|(((c)&16777215UL)<<0))
#define BITMAP_TRANSFORM_D(d) ((24UL<<24)|(((d)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(e) ((25UL<<24)|(((e)&131071UL)<<0))
#define BITMAP_TRANSFORM_F(f) ((26UL<<24)|(((f)&16777215UL)<<0))
#define SCISSOR_XY(x,y) ((27UL<<24)|(((x)&511UL)<<9)|(((y)&511UL)<<0))
#define SCISSOR_SIZE(width,height) ((28UL<<24)|(((width)&1023UL)<<10)|(((height)&1023UL)<<0))
#define CALL(dest) ((29UL<<24)|(((dest)&65535UL)<<0))
#define JUMP(dest) ((30UL<<24)|(((dest)&65535UL)<<0))
#define BEGIN(prim) ((31UL<<24)|(((prim)&15UL)<<0))
#define COLOR_MASK(r,g,b,a) ((32UL<<24)|(((r)&1UL)<<3)|(((g)&1UL)<<2)|(((b)&1UL)<<1)|(((a)&1UL)<<0))
#define CLEAR(c,s,t) ((38UL<<24)|(((c)&1UL)<<2)|(((s)&1UL)<<1)|(((t)&1UL)<<0))
#define END() ((33UL<<24))
#define SAVE_CONTEXT() ((34UL<<24))
#define RESTORE_CONTEXT() ((35UL<<24))
#define RETURN() ((36UL<<24))
#define MACRO(m) ((37UL<<24)|(((m)&1UL)<<0))
#define DISPLAY() ((0UL<<24))

#define FT_GPU_NUMCHAR_PERFONT (128)
#define FT_GPU_FONT_TABLE_SIZE (148)

/* FT800 font table structure */
/* Font table address in ROM can be found by reading the address from 0xFFFFC location. */
/* 16 font tables are present at the address read from location 0xFFFFC */
typedef struct FT_Gpu_Fonts
{
	/* All the values are in bytes */
	/* Width of each character font from 0 to 127 */
	ft_uint8_t	FontWidth[FT_GPU_NUMCHAR_PERFONT];
	/* Bitmap format of font wrt bitmap formats supported by FT800 - L1, L4, L8 */
	ft_uint32_t	FontBitmapFormat;
	/* Font line stride in FT800 ROM */
	ft_uint32_t	FontLineStride;
	/* Font width in pixels */
	ft_uint32_t	FontWidthInPixels;
	/* Font height in pixels */
	ft_uint32_t	FontHeightInPixels;
	/* Pointer to font graphics raw data */
	ft_uint32_t	PointerToFontGraphicsData;
}FT_Gpu_Fonts_t;




// Display list commands to be embedded in Graphics Processor
#define DL_ALPHA_FUNC       0x09000000UL // requires OR'd arguments
#define DL_BITMAP_HANDLE    0x05000000UL // requires OR'd arguments
#define DL_BITMAP_LAYOUT    0x07000000UL // requires OR'd arguments
#define DL_BITMAP_SIZE      0x08000000UL // requires OR'd arguments
#define DL_BITMAP_SOURCE    0x01000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_A   0x15000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_B   0x16000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_C   0x17000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_D   0x18000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_E   0x19000000UL // requires OR'd arguments
#define DL_BITMAP_TFORM_F   0x1A000000UL // requires OR'd arguments
#define DL_BLEND_FUNC       0x0B000000UL // requires OR'd arguments
#define DL_BEGIN            0x1F000000UL // requires OR'd arguments
#define DL_CALL             0x1D000000UL // requires OR'd arguments
#define DL_CLEAR            0x26000000UL // requires OR'd arguments
#define DL_CELL             0x06000000UL // requires OR'd arguments
#define DL_CLEAR_RGB        0x02000000UL // requires OR'd arguments
#define DL_CLEAR_STENCIL    0x11000000UL // requires OR'd arguments
#define DL_CLEAR_TAG        0x12000000UL // requires OR'd arguments
#define DL_COLOR_A          0x0F000000UL // requires OR'd arguments
#define DL_COLOR_MASK       0x20000000UL // requires OR'd arguments
#define DL_COLOR_RGB        0x04000000UL // requires OR'd arguments
#define DL_DISPLAY          0x00000000UL
#define DL_END              0x21000000UL
#define DL_JUMP             0x1E000000UL // requires OR'd arguments
#define DL_LINE_WIDTH       0x0E000000UL // requires OR'd arguments
#define DL_MACRO            0x25000000UL // requires OR'd arguments
#define DL_POINT_SIZE       0x0D000000UL // requires OR'd arguments
#define DL_RESTORE_CONTEXT  0x23000000UL
#define DL_RETURN           0x24000000UL
#define DL_SAVE_CONTEXT     0x22000000UL
#define DL_SCISSOR_SIZE     0x1C000000UL // requires OR'd arguments
#define DL_SCISSOR_XY       0x1B000000UL // requires OR'd arguments
#define DL_STENCIL_FUNC     0x0A000000UL // requires OR'd arguments
#define DL_STENCIL_MASK     0x13000000UL // requires OR'd arguments
#define DL_STENCIL_OP       0x0C000000UL // requires OR'd arguments
#define DL_TAG              0x03000000UL // requires OR'd arguments
#define DL_TAG_MASK         0x14000000UL // requires OR'd arguments
#define DL_VERTEX2F         0x40000000UL // requires OR'd arguments
#define DL_VERTEX2II        0x02000000UL // requires OR'd arguments

// Command and register value options
#define CLR_COL              0x4
#define CLR_STN              0x2
#define CLR_TAG              0x1
#define DECR                 4UL
#define DECR_WRAP            7UL
#define DLSWAP_DONE          0UL
#define DLSWAP_FRAME         2UL
#define DLSWAP_LINE          1UL
#define DST_ALPHA            3UL
#define EDGE_STRIP_A         7UL
#define EDGE_STRIP_B         8UL
#define EDGE_STRIP_L         6UL
#define EDGE_STRIP_R         5UL
#define EQUAL                5UL
#define GEQUAL               4UL
#define GREATER              3UL
#define INCR                 3UL
#define INCR_WRAP            6UL
#define INT_CMDEMPTY         32UL
#define INT_CMDFLAG          64UL
#define INT_CONVCOMPLETE     128UL
#define INT_PLAYBACK         16UL
#define INT_SOUND            8UL
#define INT_SWAP             1UL
#define INT_TAG              4UL
#define INT_TOUCH            2UL
#define INVERT               5UL
#define KEEP                 1UL
#define L1                   1UL
#define L4                   2UL
#define L8                   3UL
#define LEQUAL               2UL
#define LESS                 1UL
#define LINEAR_SAMPLES       0UL
#define LINES                3UL
#define LINE_STRIP           4UL
#define NEAREST              0UL
#define NEVER                0UL
#define NOTEQUAL             6UL
#define ONE                  1UL
#define ONE_MINUS_DST_ALPHA  5UL
#define ONE_MINUS_SRC_ALPHA  4UL
#define PALETTED             8UL
#define PLAYCOLOR            0x00a0a080
#define FTPOINTS             2UL       // "POINTS" is a reserved word
#define RECTS                9UL
#define REPEAT               1UL
#define REPLACE              2UL
#define RGB332               4UL
#define RGB565               7UL
#define SRC_ALPHA            2UL
#define TEXT8X8              9UL
#define TEXTVGA              10UL
#define TOUCHMODE_CONTINUOUS 3UL
#define TOUCHMODE_FRAME      2UL
#define TOUCHMODE_OFF        0UL
#define TOUCHMODE_ONESHOT    1UL
#define ULAW_SAMPLES         1UL
#define ZERO                 0UL


#define OPT_3D                  0UL       //0x0000
//Co-processor widget is drawn in 3D effect. The default option.
//CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR
#define OPT_RGB565              0UL       //0x0000
//Co-processor option to decode the JPEG image to RGB565 format
//CMD_IMAGE
#define OPT_MONO                1UL       //0x0001
//Co-processor option to decode the JPEG image to L8 format, i.e., monochrome
//CMD_IMAGE
#define OPT_NODL                2UL       //0x0002
//No display list commands generated for bitmap decoded from JPEG image
//CMD_IMAGE
#define OPT_FLAT                256UL     // 0x0100
//Co-processor widget is drawn without 3D effect
//CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR
#define OPT_SIGNED              256UL     // 0x0100
//The number is treated as 32 bit signed integer
//CMD_NUMBER
#define OPT_CENTERX             512UL     // 0x0200
//Co-processor widget centers horizontally
//CMD_KEYS,CMD_TEXT, CMD_NUMBER
#define OPT_CENTERY             1024UL    // 0x0400
//Co-processor widget centers vertically
//CMD_KEYS,CMD_TEXT, CMD_NUMBER
#define OPT_CENTER              1536UL    // 0x6000
//Co-processor widget centers horizontally and vertically.
//CMD_KEYS,CMD_TEXT, CMD_NUMBER
#define OPT_RIGHTX              2048UL    // 0x0800
//The label on the Co-processor widget is right justified
//CMD_KEYS,CMD_TEXT, CMD_NUMBER
#define OPT_NOBACK              4096UL    // 0x1000
//Co-processor widget has no background drawn
//CMD_CLOCK, CMD_GAUGE
#define OPT_NOTICKS             8192UL    // 0x2000
//Co-processor clock widget is drawn without hour ticks. Gauge widget is drawn without major and minor ticks
//CMD_CLOCK, CMD_GAUGE
#define OPT_NOHM                16384UL   // 0x4000
//Co-processor clock widget is drawn without hour and minutes hands, only seconds hand is drawn
//CMD_CLOCK
#define OPT_NOPOINTER           16384UL   // 0x4000
//The Co-processor gauge has no pointer
//CMD_GAUGE
#define OPT_NOSECS 		32768UL   // 0x8000
//Co-processor clock widget is drawn without seconds hand
//CMD_CLOCK
#define OPT_NOHANDS		49152UL   // 0xC168
//Co-processor clock widget is drawn without hour, minutes and seconds hands
//CMD_CLOCK


#endif /* SRC_FT800_FT_GPU_H_ */
