//******************************************************************************
//	LCD
//******************************************************************************
#ifndef LCD_H_
#define LCD_H_

//#define printf lcd_printf

//	initialization constants
#define EV					24			// LCD volume
#define EXT					1
#define BE					1
#define FR					7
#define RB_RA				0x06 		// internal resistor ratio
//#define RB_RA				0x27

//	display size
#define HD_X_MAX			160
#define HD_Y_MAX			100
#define MIN_PAGE			0xb0
#define MAX_PAGE			0xbc

//	print buffer size
#define PRINT_BUFFER_SIZE	32

//	lcd modes
#define LCD_PROPORTIONAL	0x01
#define LCD_REVERSE_FONT	0x02
#define LCD_2X_FONT			0x04
#define LCD_FRAM_CHARACTER	0x08
#define LCD_REVERSE_DISPLAY	0x10

//	lcd prototypes
int16 lcd_init(void);
uint8 lcd_clear(uint8 value);
void lcd_backlight(uint8 backlight);
uint8 lcd_volume(uint8 volume);
uint8 lcd_display(int16 mode);

void lcd_WC(uint8 cmd);
void lcd_WD(uint8 datum);
void lcd_WC2(uint8 cmd1, uint8 cmd2);

//	lcd character data
char lcd_putchar(char c);
void lcd_printf(char* fmt, ...);
uint8 lcd_cursor(uint16 column, uint16 page);
uint8 lcd_image(const uint8* image, uint16 column, uint16 page);
uint8 lcd_blank(uint16 column, uint16 page, uint16 width, uint16 height);

#endif /*LCD_H_*/

