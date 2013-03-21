//	lcd.c
//******************************************************************************
//******************************************************************************
//	LL        CCCCC     DDCDDD
//	LL       CC   CC    DD   DD
//	LL      CC          DD    DD
//	LL      CC          DD    DD
//	LL      CC          DD    DD
//	LL       CC   CC    DD   DD
//	LLLLLL    CCCCC     DDDDDD
//******************************************************************************
//******************************************************************************
//	Author:			Paul Roper
//	Revision:		1.0		02/15/2010
//					1.1		04/22/2010	4XFONT
//					1.2		11/09/2010	typedef's & uint8's
//					1.3		11/07/2011	align static variables
//							lcd_display(0) clears mode
//
//	Description:	Controller firmware for NHD-C160100DiZ-FSW-FBW LCD
//
//	See lcd.h for prototypes
//
//
#define BYU_LOGO	1			// include BYU logo
//
//******************************************************************************
//******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

#include "msp430x22x4.h"
#include "eZ430X.h"
#include "lcd.h"

extern jmp_buf i2c_context;				// error context
extern uint8 TXData[];					// i2c tx data

static uint8 lcd_mode;					// lcd mode
#pragma DATA_ALIGN(lcd_page,2);
static uint8 lcd_page;					// page (0-12)
static uint8 lcd_column;				// column (0-159)


//******************************************************************************
//******************************************************************************
// constant image - BYU
//
#if BYU_LOGO
const uint8 byu_image[] = { 91, 24,		// 91 wide, 24 height
//           . 1      .   2    .     3  .       4.        . 5      .   6    .     7  .       8.          9         1
//   12345678.90123456.78901234.56789012.34567890.12345678.90123456.78901234.56789012.34567890.12345678.90123456|890
// 0         .        .        . XXXXXXX.XXXXXXXX.        . XXXXXXX.XXXXXXXX.        .        .        .   |
// 1         .        .        . XXXXXXX.XXXXXXXX.        . XXXXXXX.XXXXXXXX.        .        .        .   |
// 2         .        .        .    XXXX.XXXXXX  .        .   XXXXX.XXXXX   .        .        .        .   |
// 3         .        .        .     XXX.XXXXXXX .        .  XXXXXX.XXXX    .        .        .        .   |
// 4         .        .        .      XX.XXXXXXXX.        . XXXXXXX.XXX     .        .        .        .   |
// 5 XXXXXXXX.XXXXXXXX.XXXXXXXX.       X.XXXXXXXX.X       .XXXXXXXX.XX   XXX.XXXXXXXX.X      X.XXXXXXXX.XXX|
// 6 XXXXXXXX.XXXXXXXX.XXXXXXXX.XX      .XXXXXXXX.XX     X.XXXXXXXX.X    XXX.XXXXXXXX.X      X.XXXXXXXX.XXX|
// 7  XXXXXXX.XXXXXXXX.XXXXXXXX.XXX     . XXXXXXX.XXX   XX.XXXXXXXX.      XX.XXXXXXXX.        .XXXXXXXX.XX |
	0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x03, 0xc3, 0xc1, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xcf,
	0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03,
	0x07, 0xcf, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc6, 0xc7, 0xc7,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x06,
// 0  XXXXXXX.XXX     .  XXXXXX.XXXX    .  XXXXXX.XXXX XXX.XXXXXXX .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 1  XXXXXXX.XXX     .   XXXXX.XXXXX   .   XXXXX.XXXXXXXX.XXXXXX  .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 2  XXXXXXX.XXX     .   XXXXX.XXXXX   .    XXXX.XXXXXXXX.XXXXX   .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 3  XXXXXXX.XXX     .   XXXXX.XXXXX   .     XXX.XXXXXXXX.XXXX    .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 4  XXXXXXX.XXX     .  XXXXXX.XXXX    .      XX.XXXXXXXX.XXX     .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 5  XXXXXXX.XXXXXXXX.XXXXXXXX.XXX     .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 6  XXXXXXX.XXXXXXXX.XXXXXXXX.XX      .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 7  XXXXXXX.XXXXXXXX.XXXXXXXX.XXX     .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfd, 0xf8, 0x70, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x00,
// 0  XXXXXXX.XXX     .  XXXXXX.XXXX    .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 1  XXXXXXX.XXX     .   XXXXX.XXXXX   .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 2  XXXXXXX.XXX     .   XXXXX.XXXXX   .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 3  XXXXXXX.XXX     .   XXXXX.XXXXX   .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.        .XXXXXXXX.XX |
// 4  XXXXXXX.XXX     .  XXXXXX.XXXX    .       X.XXXXXXXX.XX      .      XX.XXXXXXXX.X      X.XXXXXXXX.XX |
// 5  XXXXXXX.XXXXXXXX.XXXXXXXX.XXX     .       X.XXXXXXXX.XX      .       X.XXXXXXXX.XXXXXXXX.XXXXXXXX.X  |
// 6 XXXXXXXX.XXXXXXXX.XXXXXXXX.XX      .      XX.XXXXXXXX.XXX     .        .XXXXXXXX.XXXXXXXX.XXXXXXXX.   |
// 7 XXXXXXXX.XXXXXXXX.XXXXXXXX.        .      XX.XXXXXXXX.XXX     .        . XXXXXXX.XXXXXXXX.XXXXXXX .   |
	0x03, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff,
	0xff, 0xff, 0xff, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xfe, 0xfc, 0xf8, 0x70, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xfc,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	0xfc, 0xf8, 0x00
};
#endif

//******************************************************************************
//	Initialization sequence for LCD
//
//		0xff = 20 ms delay
//		0xfe = 2 byte lcd command
//		0xfd = 4 command gray scale
//		else single command
//
const uint8 lcd_rs[] = {
	0xe2, 0xff,			// reset
	0xa2,				// ICON  OFF;
	0xae,				// Display OFF

	// User Application Setup by Internal Instructions
	0x48,				// [Display Duty Select]	set Duty ratio 1/128
	0x80,				// nop
	0xa0,				// [ADC Select]				Reverse direction, ADC=0
	0xc0,				// [SHL Select]				SHL select (normal)
						// [COM0 Register Select]

	0xfe, 0x40, 0x00,	// Set initial display line register
	0xfe, 0x44, 0x00, 	// Set initial COM0 register
	0xfe, 0x4c, 0x00,	// Set N-line inversion 0 (disable)

	// User LCD Power Setup by Internal Instructions
	0xab,				// [Oscillator ON]			OSC on
	0x20 + RB_RA,		// [Regulator Resistor]		Select regulator register
	0xfe, 0x81, EV,		// [Electronic Volume]		Set electronic volume register, n=0~3f
	0x57,				// [LCD Bias Register]		1/12 bias

	// Select the step-up of internal voltage converter
	0x28, 0xff,			// [Power Control VC,VR,VF=0,0,0]
	0x64, 0xff,			// [(DC[1:0]=00) booster 3 x]
	0x2c, 0xff,			// [Power Control VC,VR,VF=1,0,0]
	0x67, 0xff,			// [(DC[1:0]=11) booster 6 x]
	0x2e, 0xff,			// [Power Control VC,VR,VF=1,1,0]
	0x2f,				// [Power Control VC,VR,VF=1,1,1]
	0x96,				// 3 frc, 60 pwm

	// start settings for 16-level grayscale
	0xfe, 0x38, (FR<<4) + (BE<<2) + EXT,
	0xfd, 0x80, 0x00,
	0xfd, 0x84, 0x06,
	0xfd, 0x88, 0x0b,
	0xfd, 0x8c, 0x10,
	0xfd, 0x90, 0x15,
	0xfd, 0x94, 0x1a,
	0xfd, 0x98, 0x1e,
	0xfd, 0x9c, 0x23,
	0xfd, 0xa0, 0x27,
	0xfd, 0xa4, 0x2b,
	0xfd, 0xa8, 0x2f,
	0xfd, 0xac, 0x32,
	0xfd, 0xb0, 0x35,
	0xfd, 0xb4, 0x38,
	0xfd, 0xb8, 0x3a,
	0xfd, 0xbc, 0x3c,
	0xfe, 0x38, (FR<<4) + (BE<<2),
	//	end settings for 16-level grayscale

	0x00				// end of initialization
	};


//******************************************************************************
//	initialize NHD-C160100DiZ-FSW-FBW lcd display
//
//	process lcd initialization sequence:
//		0xff = 20 ms delay
//		0xfe = 2 byte lcd command
//		0xfd = 4 command gray scale
//		else single command
//
int16 lcd_init(void)
{
	uint8 c1, i;
	uint8* dptr = (uint8*)lcd_rs;
	int error;

	// error setup
	if (error = setjmp(i2c_context))
	{
		init_USCI_B0(LCD_ADDRESS);
		return error;
	}

	while (*dptr)
	{
		switch(*dptr)
		{
			case 0xff:			// delay
				wait(20);
				break;

			case 0xfe:			// 2 byte command
				dptr++;
				c1 = *dptr++;
				lcd_WC2(c1, *dptr);
				break;

			case 0xfd:			// gray scale
				dptr++;
				c1 = *dptr++;
				for (i = 0; i < 4; i++)
				{
					lcd_WC2(c1++, *dptr);
				}
				break;

			default:
				lcd_WC(*dptr);	// single byte command
				break;
		}
		dptr++;
	}
	// reset LCD global variables
	lcd_page = 0;
	lcd_column = 0;
	lcd_mode = 0;				// reset LCD mode

	// clear display & put cursor at tos
	lcd_clear(0);				// clear display
	lcd_cursor(0, 12);			// position to tos

	lcd_WC(0xaf); 	    		// Turn on display
	return 0;
} // end lcd_init


//******************************************************************************
//	change lcd volume (brightness)
//
uint8 lcd_volume(uint8 volume)
{
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	lcd_WC2(0x81, volume & 0x003f);
	return 0;
} // end lcd_volume


//******************************************************************************
//	Turn ON/OFF LCD backlight
//
void lcd_backlight(uint8 backlight)
{
	if (backlight)
	{
		BACKLIGHT_ON;					// turn on backlight
	}
	else
	{
		BACKLIGHT_OFF;					// turn off backlight
	}
	return;
} // end lcd_backlight


//******************************************************************************
//	Display Mode
//
//	0bxxxx xxxx
//	   \\\\ \\\\___ LCD_PROPORTIONAL		proportional font
//	    \\\\ \\\___ LCD_REVERSE_FONT		reverse font
//	     \\\\ \\___ LCD_2X_FONT			2x font
//	      \\\\ \___ LCD_FRAM_CHARACTER	write to FRAM
//	       \\\\____ LCD_REVERSE_DISPLAY	reverse display
//	        \\\____ 
//			 \\____ 
//	          \____
//
//	~mode = Turn OFF mode bit(s)
//
uint8 lcd_display(int16 mode)
{
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	if (mode)
	{
		// set/reset mode bits
		if (mode > 0) lcd_mode |= mode;		// set mode bits
		else lcd_mode &= mode;				// reset mode bits

		// reverse display
		if (lcd_mode & LCD_REVERSE_DISPLAY)
			 lcd_WC(0xa7);					// reverse display
		else lcd_WC(0xa6);					// normal display
	}
	else
	{
		lcd_mode = 0;
	}
	return lcd_mode;
} // end lcd_mode


//******************************************************************************
//	write single command to LCD
//
void lcd_WC(uint8 datum)
{
	TXData[0] = 0x00;					// Co=0, A0=0
	TXData[1] = datum;
	i2c_write(LCD_ADDRESS, TXData, 2);
	return;
} // end lcd_WC


//******************************************************************************
//	write double command to LCD
//
void lcd_WC2(uint8 datum1, uint8 datum2)
{
	TXData[0] = 0x80;					// Co=1, A0=0
	TXData[1] = datum1;
	TXData[2] = 0x00;					// Co=0, A0=0
	TXData[3] = datum2;
	i2c_write(LCD_ADDRESS, TXData, 4);
	return;
} // end lcd_WC2


//******************************************************************************
//	write data to LCD
//
void lcd_WD(uint8 datum)
{
	TXData[0] = 0xc0;					// Co=1, A0=1
	TXData[1] = datum;
	TXData[2] = 0xc0;					// Co=1, A0=1
	TXData[3] = datum;
	TXData[4] = 0xc0;					// Co=1, A0=1
	TXData[5] = datum;
	TXData[6] = 0x40;					// Co=0, A0=1
	TXData[7] = datum;
	i2c_write(LCD_ADDRESS, TXData, 8);
	return;
} // end lcd_WD


//******************************************************************************
//	clear LCD memory
//
uint8 lcd_clear(uint8 value)
{
	int page, column;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	for (page = MIN_PAGE; page <= MAX_PAGE; page++)
	{
		lcd_WC(page);				// set page
		lcd_WC2(0x01, 0x00);
		for (column = 0; column < HD_X_MAX; column++)
		{
			lcd_WD(value);			// zero display memory
		}
	}
	return 0;
} // end lcd_clear


//******************************************************************************
//	blank lcd area
//
uint8 lcd_blank(uint16 column, uint16 page, uint16 width, uint16 height)
{
	int x;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	height >>= 3;				// get image height (in pages)
	page += MIN_PAGE;			// get start page

	while (height--)
	{
		// output page and column
		lcd_WC(page--);
		lcd_WC2(0x10 + (column>>4), column & 0x000f);
		// blank image row
		for (x = 0; x < width; x++) lcd_WD(0);
	}
	return 0;
} // end lcd_display_image


//******************************************************************************
//	Display Image
//
//	IN:		const char* image	pointer to image
//			int column,			start column (0-129)
//			int page,			start page (12-0)
//
uint8 lcd_image(const uint8* image, uint16 column, uint16 page)
{
	unsigned char x, width, height;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	width = *image++;					// get image width
	height = (*image++) >> 3;			// get image height (in pages)
	page += MIN_PAGE;					// get start page

	while (height--)
	{
		lcd_WC(page--);					// set page
		lcd_WC2(0x10 + (column>>4), column & 0x000f);	// set column
		for (x = 0; x < width; x++)
		{
			lcd_WD(*image++);			// output image row
		}
	}
	return 0;
} // end lcd_image


//******************************************************************************
//******************************************************************************
//	ASCII character set for LCD
//
const unsigned char cs[][5] = {
//	-----  -XX--  XX-XX  -----  -X---  XX--X  -X---  -XX--
//	-----  -XX--  XX-XX  -X-X-  -XXX-  XX--X  X-X--  -XX--
//	-----  -XX--  X--X-  XXXXX  X----  ---X-  X-X--  -----
//	-----  -XX--  -----  -X-X-  -XX--  --X--  -X---  -----
//	-----  -XX--  -----  -X-X-  ---X-  -X---  X-X-X  -----
//	-----  -----  -----  XXXXX  XXX--  X--XX  X--X-  -----
//	-----  -XX--  -----  -X-X-  --X--  X--XX  -XX-X  -----
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0x00,0x00,0x00,0x00,0x00 },		// SP
	{ 0xfa,0xfa,0x00,0x00,0x00 },		// !
	{ 0xe0,0xc0,0x00,0xe0,0xc0 },		// "
	{ 0x24,0x7e,0x24,0x7e,0x24 },		// #
	{ 0x24,0xd4,0x56,0x48,0x00 },		// $
	{ 0xc6,0xc8,0x10,0x26,0xc6 },		// %
	{ 0x6c,0x92,0x6a,0x04,0x0a },		// &
	{ 0xc0,0xc0,0x00,0x00,0x00 },		// '

//	---X-  -X---  -----  -----  -----  -----  -----  -----
//	--X--  --X--  -X-X-  --X--  -----  -----  -----  ----X
//	--X--  --X--  -XXX-  --X--  -----  -----  -----  ---X-
//	--X--  --X--  XXXXX  XXXXX  -----  XXXXX  -----  --X--
//	--X--  --X--  -XXX-  --X--  -----  -----  -----  -X---
//	--X--  --X--  -X-X-  --X--  -XX--  -----  -XX--  X----
//	---X-  -X---  -----  -----  -XX--  -----  -XX--  -----
//	-----  -----  -----  -----  -X---  -----  -----  -----
	{ 0x7c,0x82,0x00,0x00,0x00 },		// (
	{ 0x82,0x7c,0x00,0x00,0x00 },		// )
	{ 0x10,0x7c,0x38,0x7c,0x10 },		// #
	{ 0x10,0x10,0x7c,0x10,0x10 },		// +
	{ 0x07,0x06,0x00,0x00,0x00 },		// ,
	{ 0x10,0x10,0x10,0x10,0x10 },		// -
	{ 0x06,0x06,0x00,0x00,0x00 },		// .
	{ 0x04,0x08,0x10,0x20,0x40 },		// slash

//	-XXX-  --X--  -XXX-  -XXX-  ---X-  XXXXX  --XXX  XXXXX
//	X---X  -XX--  X---X  X---X  --XX-  X----  -X---  ----X
//	X--XX  --X--  ----X  ----X  -X-X-  X----  X----  ---X-
//	X-X-X  --X--  --XX-  -XXX-  X--X-  XXXX-  XXXX-  --X--
//	XX--X  --X--  -X---  ----X  XXXXX  ----X  X---X  -X---
//	X---X  --X--  X----  X---X  ---X-  X---X  X---X  -X---
//	-XXX-  -XXX-  XXXXX  -XXX-  ---X-  -XXX-  -XXX-  -X---
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0x7c,0x8a,0x92,0xa2,0x7c },		// 0
	{ 0x42,0xfe,0x02,0x00,0x00 },		// 1
	{ 0x46,0x8a,0x92,0x92,0x62 },		// 2
	{ 0x44,0x92,0x92,0x92,0x6c },		// 3
	{ 0x18,0x28,0x48,0xfe,0x08 },		// 4
	{ 0xf4,0x92,0x92,0x92,0x8c },		// 5
	{ 0x3c,0x52,0x92,0x92,0x8c },		// 6
	{ 0x80,0x8e,0x90,0xa0,0xc0 },		// 7

//	-XXX-  -XXX-  -----  -----  ---X-  -----  -X---  -XXX-
//	X---X  X---X  -----  -----  --X--  -----  --X--  X---X
//	X---X  X---X  -XX--  -XX--  -X---  XXXXX  ---X-  X---X
//	-XXX-  -XXXX  -XX--  -XX--  X----  -----  ----X  --XX-
//	X---X  ----X  -----  -----  -X---  -----  ---X-  --X--
//	X---X  ---X-  -XX--  -XX--  --X--  XXXXX  --X--  -----
//	-XXX-  -XX--  -XX--  -XX--  ---X-  -----  -X---  --X--
//	-----  -----  -----  -X---  -----  -----  -----  -----
	{ 0x6c,0x92,0x92,0x92,0x6c },		// 8
	{ 0x60,0x92,0x92,0x94,0x78 },		// 9
	{ 0x36,0x36,0x00,0x00,0x00 },		// :
	{ 0x37,0x36,0x00,0x00,0x00 },		// ;
	{ 0x10,0x28,0x44,0x82,0x00 },		// <
	{ 0x24,0x24,0x24,0x24,0x24 },		// =
	{ 0x82,0x44,0x28,0x10,0x00 },		// >
	{ 0x60,0x80,0x9a,0x90,0x60 },		// ?

//	-XXX-  -XXX-  XXXX-  -XXX-  XXXX-  XXXXX  XXXXX  -XXX-
//	X---X  X---X  X---X  X---X  X---X  X----  X----  X---X
//	X-XXX  X---X  X---X  X----  X---X  X----  X----  X----
//	X-X-X  XXXXX  XXXX-  X----  X---X  XXXX-  XXXX-  X-XXX
//	X-XXX  X---X  X---X  X----  X---X  X----  X----  X---X
//	X----  X---X  X---X  X---X  X---X  X----  X----  X---X
//	-XXX-  X---X  XXXX-  -XXX-  XXXX   XXXXX  X----  -XXX-
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0x7c,0x82,0xba,0xaa,0x78 },		// @
	{ 0x7e,0x90,0x90,0x90,0x7e },		// A
	{ 0xfe,0x92,0x92,0x92,0x6c },		// B
	{ 0x7c,0x82,0x82,0x82,0x44 },		// C
	{ 0xfe,0x82,0x82,0x82,0x7c },		// D
	{ 0xfe,0x92,0x92,0x92,0x82 },		// E
	{ 0xfe,0x90,0x90,0x90,0x80 },		// F
	{ 0x7c,0x82,0x92,0x92,0x5c },		// G

//	X---X  -XXX-  ----X  X---X  X----  X---X  X---X  -XXX-
//	X---X  --X--  ----X  X--X-  X----  XX-XX  XX--X  X---X
//	X---X  --X--  ----X  X-X--  X----  X-X-X  X-X-X  X---X
//	XXXXX  --X--  ----X  XX---  X----  X---X  X--XX  X---X
//	X---X  --X--  X---X  X-X--  X----  X---X  X---X  X---X
//	X---X  --X--  X---X  X--X-  X----  X---X  X---X  X---X
//	X---X  -XXX-  -XXX-  X---X  XXXXX  X---X  X---X  -XXX-
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0xfe,0x10,0x10,0x10,0xfe },		// H
	{ 0x82,0xfe,0x82,0x00,0x00 },		// I
	{ 0x0c,0x02,0x02,0x02,0xfc },		// J
	{ 0xfe,0x10,0x28,0x44,0x82 },		// K
	{ 0xfe,0x02,0x02,0x02,0x02 },		// L
	{ 0xfe,0x40,0x20,0x40,0xfe },		// M
	{ 0xfe,0x40,0x20,0x10,0xfe },		// N
	{ 0x7c,0x82,0x82,0x82,0x7c },		// O

//	XXXX-  -XXX-  XXXX-  -XXX-  XXXXX  X---X  X---X  X---X
//	X---X  X---X  X---X  X---X  --X--  X---X  X---X  X---X
//	X---X  X---X  X---X  X----  --X--  X---X  X---X  X-X-X
//	XXXX-  X-X-X  XXXX-  -XXX-  --X--  X---X  X---X  X-X-X
//	X----  X--XX  X--X-  ----X  --X--  X---X  X---X  X-X-X
//	X----  X--X-  X---X  X---X  --X--  X---X  -X-X-  X-X-X
//	X----  -XX-X  X---X  -XXX-  --X--  -XXX-  --X--  -X-X-
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0xfe,0x90,0x90,0x90,0x60 },		// P
	{ 0x7c,0x82,0x92,0x8c,0x7a },		// Q
	{ 0xfe,0x90,0x90,0x98,0x66 },		// R
	{ 0x64,0x92,0x92,0x92,0x4c },		// S
	{ 0x80,0x80,0xfe,0x80,0x80 },		// T
	{ 0xfc,0x02,0x02,0x02,0xfc },		// U
	{ 0xf8,0x04,0x02,0x04,0xf8 },		// V
	{ 0xfc,0x02,0x3c,0x02,0xfc },		// W

//	X---X  X---X  XXXXX  -XXX-  -----  -XXX-  --X--  -----
//	X---X  X---X  ----X  -X---  X----  ---X-  -X-X-  -----
//	-X-X-  X---X  ---X-  -X---  -X---  ---X-  X---X  -----
//	--X--  -X-X-  --X--  -X---  --X--  ---X-  -----  -----
//	-X-X-  --X--  -X---  -X---  ---X-  ---X-  -----  -----
//	X---X  --X--  X----  -X---  ----X  ---X-  -----  -----
//	X---X  --X--  XXXXX  -XXX-  -----  -XXX-  -----  XXXXX
//	-----  -----  -----  -----  -----  -----  -----  -----
	{ 0xc6,0x28,0x10,0x28,0xc6 },		// X
	{ 0xe0,0x10,0x0e,0x10,0xe0 },		// Y
	{ 0x86,0x8a,0x92,0xa2,0xc2 },		// Z
	{ 0xfe,0x82,0x82,0x00,0x00 },		// [
	{ 0x40,0x20,0x10,0x08,0x04 },		// back slash
	{ 0x82,0x82,0xfe,0x00,0x00 },		// ]
	{ 0x20,0x40,0x80,0x40,0x20 },		// ^
	{ 0x02,0x02,0x02,0x02,0x02 },		// _

//	-XX--  -----  X----  -----  ----X  -----  --XXX  -----
//	-XX--  -----  X----  -----  ----X  -----  -X---  -----
//	--X--  -XXX-  XXXX-  -XXX-  -XXXX  -XXX-  -X---  -XXXX
//	-----  ----X  X---X  X---X  X---X  X---X  XXXX-  X---X
//	-----  -XXXX  X---X  X----  X---X  XXXX-  -X---  X---X
//	-----  X---X  X---X  X---X  X---X  X----  -X---  -XXXX
//	-----  -XXXX  XXXX-  -XXX-  -XXXX  -XXX-  -X---  ----X
//	-----  -----  -----  -----  -----  -----  -----  -XXX-
	{ 0xc0,0xe0,0x00,0x00,0x00 },		// `
	{ 0x04,0x2a,0x2a,0x2a,0x1e },		// a
	{ 0xfe,0x22,0x22,0x22,0x1c },		// b
	{ 0x1c,0x22,0x22,0x22,0x14 },		// c
	{ 0x1c,0x22,0x22,0x22,0xfc },		// d
	{ 0x1c,0x2a,0x2a,0x2a,0x10 },		// e
	{ 0x10,0x7e,0x90,0x90,0x80 },		// f
	{ 0x18,0x25,0x25,0x25,0x3e },		// g

//	X----  -X---  ----X  X----  X----  -----  -----  -----
//	X----  -----  -----  X----  X----  -----  -----  -----
//	X----  -X---  ---XX  X--X-  X----  XX-X-  XXXX-  -XXX-
//	XXXX-  -X---  ----X  X-X--  X----  X-X-X  X---X  X---X
//	X---X  -X---  ----X  XX---  X----  X-X-X  X---X  X---X
//	X---X  -X---  ----X  X-X--  X----  X---X  X---X  X---X
//	X---X  -XX--  X---X  X--X-  XX---  X---X  X---X  -XXX-
//	-----  -----  -XXX-  -----  -----  -----  -----  -----
	{ 0xfe,0x10,0x10,0x10,0x0e },		// h
	{ 0xbe,0x02,0x00,0x00,0x00 },		// i
	{ 0x02,0x01,0x01,0x21,0xbe },		// j
	{ 0xfe,0x08,0x14,0x22,0x00 },		// k
	{ 0xfe,0x02,0x00,0x00,0x00 },		// l
	{ 0x3e,0x20,0x18,0x20,0x1e },		// m
	{ 0x3e,0x20,0x20,0x20,0x1e },		// n
	{ 0x1c,0x22,0x22,0x22,0x1c },		// o

//	-----  -----  -----  -----  -----  -----  -----  -----
//	-----  -----  -----  -----  -X---  -----  -----  -----
//	XXXX-  -XXXX  X-XX-  -XXX-  XXXX-  X--X-  X---X  X---X
//	X---X  X---X  -X--X  X----  -X---  X--X-  X---X  X---X
//	X---X  X---X  -X---  -XXX-  -X---  X--X-  X---X  X-X-X
//	X---X  X---X  -X---  ----X  -X--X  X-XX-  -X-X-  XXXXX
//	XXXX-  -XXXX  XXX--  XXXX-  --XX-  -X-X-  --X--  -X-X-
//	X----  ----X  -----  -----  -----  -----  -----  -----
	{ 0x3f,0x22,0x22,0x22,0x1c },		// p
	{ 0x1c,0x22,0x22,0x22,0x3f },		// q
	{ 0x22,0x1e,0x22,0x20,0x10 },		// r
	{ 0x12,0x2a,0x2a,0x2a,0x04 },		// s
	{ 0x20,0x7c,0x22,0x22,0x04 },		// t
	{ 0x3c,0x02,0x04,0x3e,0x00 },		// u
	{ 0x38,0x04,0x02,0x04,0x38 },		// v
	{ 0x3c,0x06,0x0c,0x06,0x3c },		// w

//	-----  -----  -----  ---XX  --X--  XX---  -X-X-  -----
//	-----  -----  -----  --X--  --X--  --X--  X-X--  -----
//	X---X  X--X-  XXXX-  --X--  --X--  --X--  -----  -----
//	-X-X-  X--X-  ---X-  -XX--  -----  --XX-  -----  -----
//	--X--  X--X-  -XX--  --X--  --X--  --X--  -----  --X--
//	-X-X-  -XXX-  X----  --X--  --X--  --X--  -----  -X-X-
//	X---X  --X--  XXXX-  ---XX  --X--  XX---  -----  XXXXX
//	-----  XX---  -----  -----  -----  -----  -----  -----
	{ 0x22,0x14,0x08,0x14,0x22 },		// x
	{ 0x39,0x05,0x06,0x3c,0x00 },		// y
	{ 0x26,0x2a,0x2a,0x32,0x00 },		// z
	{ 0x10,0x7c,0x82,0x82,0x00 },		// {
	{ 0xee,0x00,0x00,0x00,0x00 },		// |
	{ 0x82,0x82,0x7c,0x10,0x00 },		// }
	{ 0x40,0x80,0x40,0x80,0x00 },		// ~
	{ 0x02,0x06,0x0a,0x06,0x02 }		// _
};



//******************************************************************************
//	set lcd cursor position
//
//	Description: set the position at which the next character will be printed.
//
uint8 lcd_cursor(uint16 column, uint16 page)
{
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	if (column >= HD_X_MAX) column = HD_X_MAX-1;
	lcd_page = page;
	lcd_column = column;

	lcd_WC(MIN_PAGE + lcd_page);		// set page
	lcd_WC2(0x10 + (column >> 4), column & 0x000f);
	return 0;
} // end lcd_moveTo



//******************************************************************************
//	output character to current LCD x,y position
//
//		\a = reverse font
//		\n = new line
//		\r = return

void lcd_setColumnPage(uint16 column, uint16 page)
{
	lcd_WC(MIN_PAGE + page);			// set page
	lcd_WC2(0x10 + (column >> 4), column & 0x000f);
	return;
}

char lcd_putchar(char c)
{
	int i;
	int error;

	// error setup
	if (error = setjmp(i2c_context))
	{
		init_USCI_B0(LCD_ADDRESS);
		return error;
	}

	switch (c)
	{
		case '\a':
		{
			lcd_mode |= LCD_REVERSE_FONT;
			break;
		}

		case '\n':
		{
			lcd_page = (lcd_page-1) % ((MAX_PAGE+1) & 0x0f);
			lcd_WC(MIN_PAGE + lcd_page);	// set page
		}

		case '\r':
		{
		 	lcd_WC2(0x10, 0x00);			// set col = 0
		 	lcd_column = 0;
		 	break;
		}

		default:
		{
			if ((c >= ' ') && (c <= '~'))
			{
/*
				if (lcd_mode & LCD_4X_FONT)
				{
					for (i = 0; i < 4; i++)
					{
						lcd_setColumnPage(lcd_column, lcd_page-i);
 						lcd_WD(0x00);
					}
					if (++lcd_column >= HD_X_MAX) lcd_column = 0;

					for (i = 0; i < 5; i++)
					{
						unsigned char mask1 = 0x01;
						unsigned int mask2 = 0x0001;
						unsigned int data = 0;

						while (mask1)
						{
							// double bits into data
							if (cs[c - ' '][i] & mask1) data |= mask2 | (mask2 << 1);
							mask1 <<= 1;
							mask2 <<= 2;
						}
						lcd_setColumnPage(lcd_column, lcd_page);
	 					lcd_WD(data >> 8);
	 					lcd_WD(data >> 8);

						lcd_setColumnPage(lcd_column, lcd_page-1);
			 			lcd_WD(data & 0x00ff);
			 			lcd_WD(data & 0x00ff);

						if ((lcd_column += 2) >= HD_X_MAX) lcd_column = 0;
						if ((lcd_mode & LCD_PROPORTIONAL) && !cs[c - ' '][i+1]) break;
					}
					for (i = 0; i < 4; i++)
					{
						lcd_setColumnPage(lcd_column, lcd_page-i);
 						lcd_WD(0x00);
					}
					if (++lcd_column >= HD_X_MAX) lcd_column = 0;
				}
				else
*/
				if (lcd_mode & LCD_2X_FONT)
				{
					lcd_setColumnPage(lcd_column, lcd_page);
 					lcd_WD(0x00);

					lcd_setColumnPage(lcd_column, lcd_page-1);
 					lcd_WD(0x00);

					if (++lcd_column >= HD_X_MAX) lcd_column = 0;

					for (i = 0; i < 5; i++)
					{
						unsigned char mask1 = 0x01;
						unsigned int mask2 = 0x0001;
						unsigned int data = 0;

						while (mask1)
						{
							// double bits into data
							if (cs[c - ' '][i] & mask1) data |= mask2 | (mask2 << 1);
							mask1 <<= 1;
							mask2 <<= 2;
						}
						lcd_setColumnPage(lcd_column, lcd_page);
	 					lcd_WD(data >> 8);
	 					lcd_WD(data >> 8);

						lcd_setColumnPage(lcd_column, lcd_page-1);
			 			lcd_WD(data & 0x00ff);
			 			lcd_WD(data & 0x00ff);

						if ((lcd_column += 2) >= HD_X_MAX) lcd_column = 0;
						if ((lcd_mode & LCD_PROPORTIONAL) && !cs[c - ' '][i+1]) break;
					}
					lcd_setColumnPage(lcd_column, lcd_page);
 					lcd_WD(0x00);

					lcd_setColumnPage(lcd_column, lcd_page-1);
 					lcd_WD(0x00);

					if (++lcd_column >= HD_X_MAX) lcd_column = 0;
				}
				else
				{
					if (lcd_mode & LCD_REVERSE_FONT) lcd_WD(0xff);
					else lcd_WD(0x00);

					for (i = 0; i < 5; )
					{
						// output character (or reverse)
						if (lcd_mode & LCD_REVERSE_FONT) lcd_WD(~cs[c - ' '][i++]);
						else lcd_WD(cs[c - ' '][i++]);
						if (++lcd_column >= HD_X_MAX) lcd_column = 0;

						// check proportional flag
						// if (lcd_Proportional && !cs[c - ' '][i+1]) break;
						if ((lcd_mode & LCD_PROPORTIONAL) && !cs[c - ' '][i+1]) break;
					}
					if (lcd_mode & LCD_REVERSE_FONT) lcd_WD(0xff);
					else lcd_WD(0x00);
					if (++lcd_column >= HD_X_MAX) lcd_column = 0;
				}
			}
		}
	}
	return c;
} // end my_putchar



//******************************************************************************
//	formatted print to lcd
//
void lcd_printf(char* fmt, ...)
{
	static char printBuffer[PRINT_BUFFER_SIZE+1];
	char* s_ptr = printBuffer;
	va_list arg_ptr;

	if (strlen(fmt) > PRINT_BUFFER_SIZE) ERROR2(SYS_ERR_PRINT);

	lcd_mode &= ~LCD_REVERSE_FONT;			// turn off reverse
	va_start(arg_ptr, fmt);					// create pointer to args
	vsprintf(s_ptr, fmt, arg_ptr);			// generate print string
	while (*s_ptr) lcd_putchar(*s_ptr++);	// output string
	va_end(arg_ptr);						// destroy arg pointer
	return;
} // end lcd_printf

