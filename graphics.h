#ifndef GRAPHICS_H_
#define GRAPHICS_H_

//******************************************************************************
//	FRAM
//******************************************************************************

#define FRAM_SIZE			8192
#define FRAM_LCD_SIZE		2080		// 13 x 160
#define FRAM_CACHE_SIZE		7

typedef union
{
	unsigned long value;
	struct
	{
		unsigned int addr;
		unsigned char data;
		unsigned char temp;
	} entry;
} FRAM_ENTRY;

//	LCD Prototypes
uint8 lcd_point(uint16 x, uint16 y, uint8 flag);
void lcd_circle(uint16 x0, uint16 y0, uint16 r0, uint8 pen);
uint8 lcd_FRAM_init(void);

//	FRAM Prototypes
uint8 FRAM_init(uint16 size, uint8 data);
uint8 FRAM_read(uint16 address);
uint8 FRAM_write(uint16 address, uint8 datum);

uint8 read_FRAM_point(uint16 x, uint16 y);
uint8 write_FRAM_point(uint16 x, uint16 y, uint8 value);

#endif /*GRAPHICS_H_*/
