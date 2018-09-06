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

#ifndef SRC_FT800_FT_DATATYPES_H_
#define SRC_FT800_FT_DATATYPES_H_

/*-----------------------------------------------------------------------------
 * Included Files
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Definitions
 *---------------------------------------------------------------------------*/

#define FT_FALSE           (0)
#define FT_TRUE            (1)

/*-----------------------------------------------------------------------------
 * Global Variables Declarations
 *---------------------------------------------------------------------------*/

typedef unsigned char ft_uint8_t;
typedef char ft_char8_t;
typedef signed char ft_schar8_t;
typedef unsigned char ft_uchar8_t;
typedef int  ft_int16_t;
typedef unsigned int ft_uint16_t;
typedef unsigned long ft_uint32_t;
typedef long ft_int32_t;
typedef void ft_void_t;

typedef unsigned char ft_bool_t;
//typedef ft_bool_t   bool;


#define UINT8 ft_uint8_t
#define UINT16 ft_uint16_t
#define UINT32 ft_uint32_t



/*-----------------------------------------------------------------------------
 * Global Function Declarations
 *---------------------------------------------------------------------------*/

#endif /* SRC_FT800_FT_DATATYPES_H_*/
/// @}
