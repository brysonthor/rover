//	graphics.c
//******************************************************************************
//******************************************************************************
//
//	  GGGGG    RRRRRR       AA      PPPPPP   HH   HH  IIII    CCCCC     SSSSS
//	 GG   GG   RR   RR     AAAA     PP   PP  HH   HH   II    CC   CC   SS   SS
//	GG         RR   RR    AA  AA    PP   PP  HH   HH   II   CC        SS
//	GG         RRRRRR    AA    AA   PPPPPP   HHHHHHH   II   CC         SSSSS
//	GG   GGG   RR RR    AAAAAAAAAA  PP       HH   HH   II   CC             SS
//	 GG   GG   RR  RR   AA      AA  PP       HH   HH   II    CC   CC  SS    SS
//	  GGGGG    RR   RR  AA      AA  PP       HH   HH  IIII    CCCCC    SSSSSS
//
//******************************************************************************
//******************************************************************************
//	Author:			Paul Roper, Brigham Young University
//	Revision:		1.0		02/15/2010
//					1.1		03/24/2010	added retry to i2c_write
//
//	Description:	Routines to access points on LCD
//
//******************************************************************************
//******************************************************************************
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

#include "msp430x22x4.h"
#include "eZ430X.h"
#include "lcd.h"
#include "graphics.h"

extern uint8 TXData[];
extern jmp_buf i2c_context;				// error context

#if FRAM_CACHE_SIZE
FRAM_ENTRY FRAM_cache[FRAM_CACHE_SIZE];	// FRAM cache
uint16 FRAM_address;
#endif

//******************************************************************************
//	access lcd point at x,y
//
//	flag	0 = turn point off
//			1 = turn point on
//			2 = turn double point off
//			3 = turn double point on
//			4 = read point (0 or 1)
//			5 = read byte
//
//	return results
//
uint8 lcd_point(uint16 x, uint16 y, uint8 flag)
{
	unsigned char page, data, framData;
	unsigned int FRAM_address;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) ERROR2(error);

	// return 1 if out of range
	if ((x >= HD_X_MAX) || (y >= HD_Y_MAX)) return 1;

	// calculate FRAM address
	page = y >> 3;
	data = 0x01 << (y & 0x0007);

	FRAM_address = page * 160 + x;

	// read data from FRAM
	TXData[0] = FRAM_address >> 8;			// address MSB
	TXData[1] = FRAM_address & 0x00ff;		// address LSB
	i2c_write(FRAM_ADR, TXData, 2);
	framData = i2c_read(FRAM_ADR, &framData, 1);

	// process FRAM data
	switch (flag)
	{
		case 0:						// turn point off
			data = ~data & framData;
			break;

		case 1:						// turn point on
			data |= framData;
			break;

		case 2:						// double point off
			lcd_point(x-1, y, OFF);
			lcd_point(x, y+1, OFF);
			lcd_point(x+1, y, OFF);
			lcd_point(x, y-1, OFF);
			lcd_point(x, y, OFF);
			return 0;

		case 3:						// double point on
			lcd_point(x-1, y, ON);
			lcd_point(x, y+1, ON);
			lcd_point(x+1, y, ON);
			lcd_point(x, y-1, ON);
			lcd_point(x, y, ON);
			return 0;

		case 4:						// read point
			return (data | framData) ? 1 : 0;

		default:
		case 5:						// read byte
			return framData;
	}

	// write data to FRAM
	FRAM_write(FRAM_address, data);	//	write to FRAM

	// write data to lcd
	lcd_WC(page + MIN_PAGE);		// set page
	lcd_WC(0x10 + (x >> 4));		// set col = x
	lcd_WC(x % 16);
	lcd_WD(data);					// write data back

	return 0;						// return success
} // end lcd_point


//******************************************************************************
//	draw circle of radius r0 and center x0,y0
//
void lcd_circle(uint16 x0, uint16 y0, uint16 r0, uint8 pen)
{
	int16  x, y, d;
  
	if (r0 == 0) return;			// ignore invalid parameters

	x = x0;
	y = y0 + r0;
	d =  3 - r0 * 2;
  
	do 
	{
    	lcd_point (x, y, 1);
    	lcd_point (x, y0 - (y - y0), 1);
    	lcd_point (x0 - (x - x0), y, 1);
    	lcd_point (x0 - (x - x0), y0 - (y - y0), 1);
    
    	lcd_point (x0 + (y - y0), y0 + (x - x0), 1);
    	lcd_point (x0 + (y - y0), y0 - (x - x0), 1);
    	lcd_point (x0 - (y - y0), y0 + (x - x0), 1);
    	lcd_point (x0 - (y - y0), y0 - (x - x0), 1);
    
		if (d < 0)
		{
			d = d +  ((x - x0) << 2) + 6;
		}
		else 
		{
			d = d + (((x - x0) - (y - y0)) << 2) + 10; 
			y--;
		}
		x++;
	} while ((x - x0) <= (y - y0));
	return;
} // end lcd_circle


//******************************************************************************
//
uint8 read_FRAM_point(uint16 x, uint16 y)
{
	unsigned char data, framData;
	unsigned int FRAM_address;

	if ((x >= 160) || (y >= 100)) return 0;

	data = 0x01 << (y & 0x0007);
	FRAM_address = (y >> 3) * 160 + x;
	framData = FRAM_read(FRAM_address);
	return (data & framData) ? 1 : 0;
} // end read_FRAM_point


//******************************************************************************
//
uint8 write_FRAM_point(uint16 x, uint16 y, uint8 value)
{
	unsigned int data, framData;
	unsigned int FRAM_address;

	if ((x >= 160) || (y >= 100)) return 1;

	data = 0x01 << (y & 0x0007);
	FRAM_address = (y >> 3) * 160 + x;
	framData = FRAM_read(FRAM_address);

	if (value) framData |= data;
	else framData &= ~data;

	FRAM_write(FRAM_address, framData);	//	write to FRAM
	return 0;
} // end write_FRAM_point


//******************************************************************************
//	initialize lcd FRAM memory
//
uint8 lcd_FRAM_init(void)
{
	unsigned int i;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	for (i = 0; i < FRAM_LCD_SIZE; i++)
	{
		TXData[0] = i >> 8;				// address MSB
		TXData[1] = i & 0x00ff;			// address LSB
		TXData[2] = 0;					// data byte
		i2c_write(FRAM_ADR, TXData, 3);
	}

#if FRAM_CACHE_SIZE
	for (i = 0; i < FRAM_CACHE_SIZE; i++)
	{
		FRAM_cache[i].value = 0xffffffff;
	}
	FRAM_address = 0;
#endif

	return 0;
} // end lcd_FRAM_init


//******************************************************************************
//	initialize FRAM
//
uint8 FRAM_init(uint16 size, uint8 data)
{
	unsigned int i;
	int error;

	// error setup
	if (error = setjmp(i2c_context)) return error;

	for (i = 0; i < size; i++)
	{
		FRAM_write(i, data);
	}
	return 0;
} // end FRAM_init1


#if FRAM_CACHE_SIZE
//******************************************************************************
//	read from FRAM
//
uint8 FRAM_read(uint16 address)
{
	unsigned int i;
	FRAM_ENTRY temp1, temp2;

	if (FRAM_cache[0].entry.addr == address)
	{
		return FRAM_cache[0].entry.data;
	}

	temp2.entry.addr = address;

	for (i = 0; i < FRAM_CACHE_SIZE; i++)
	{
		temp1 = FRAM_cache[i];
		FRAM_cache[i] = temp2;
		if (temp1.entry.addr == address)
		{
			FRAM_cache[0].entry.data = temp1.entry.data;
			return (unsigned char)temp1.entry.data;
		}
		temp2 = temp1;
	}
	// not in cache, read FRAM_ADR_CACHE[0]

	// could we cache writing an address each time?
	if (++FRAM_address != address)
	{
//		TXData[0] = address >> 8;			// address MSB
//		TXData[1] = address & 0x00ff;		// address LSB
//		i2c_write(FRAM_ADR, TXData, 2);
		i2c_write(FRAM_ADR, (uint8*)&address, 2);
		FRAM_address = address;
	}
	// update cache
	return (unsigned char)i2c_read(FRAM_ADR, &FRAM_cache[0].entry.data, 1);
} // end FRAM_read


//******************************************************************************
//
uint8 FRAM_write(uint16 address, uint8 datum)
{
	unsigned int i;

	// look for entry in cache
	for (i = 0; i < FRAM_CACHE_SIZE; i++)
	{
		if (FRAM_cache[i].entry.addr == address)
		{
			FRAM_cache[i].entry.data = datum;
			break;
		}
	}
	// now write to FRAM
	TXData[0] = address >> 8;			// address MSB
	TXData[1] = address & 0x00ff;		// address LSB
	TXData[2] = datum;					// data byte
	i2c_write(FRAM_ADR, TXData, 3);
	FRAM_address = address;
	return 1;
} // end FRAM_write

#else
//******************************************************************************
//	read from FRAM
//
unsigned char FRAM_read(unsigned int address)
{
//	TXData[0] = address >> 8;			// address MSB
//	TXData[1] = address & 0x00ff;		// address LSB
//	i2c_write(FRAM_ADR, TXData, 2);
	i2c_write(FRAM_ADR, (uint8*)&address, 2);
	return i2c_read(FRAM_ADR, 1);
} // end FRAM_read


//******************************************************************************
//	write to FRAM
//
int FRAM_write(unsigned int address, unsigned char datum)
{
	TXData[0] = address >> 8;			// address MSB
	TXData[1] = address & 0x00ff;		// address LSB
	TXData[2] = datum;					// data byte
	i2c_write(FRAM_ADR, TXData, 3);
	FRAM_address = address;
	return 1;
} // end FRAM_write
#endif
