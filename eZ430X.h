//******************************************************************************
//	eZ430X.h
//
//	Author:			Paul Roper
//	Revision:		1.2		11/04/2010	typedef's included
//******************************************************************************
#ifndef eZ430X_H_
#define eZ430X_H_

#define REV_C	1

enum _430clock {_16MHZ, _12MHZ, _8MHZ, _1MHZ};

//******************************************************************************
//	data types
typedef char int8;
typedef int int16;
typedef long int32;

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

#define ON				1
#define OFF				0

#define TRUE			1
#define FALSE			0

//	system errors
enum SYS_ERRORS {	SYS_ERR_430init=1,		// 1 eZ430X initialize
					SYS_ERR_PRINT,			// 2 lprintf line too long
					SYS_ERR_LCD,			// 3 lcd not responding
					SYS_ERR_ADC,			// 4 adc
					SYS_ERR_FRAM,			// 5 FRAM
					SYS_ERR_XL345,			// 6 accelerometer
					SYS_ERR_USCB_RX,		// 7 USCB receive timeout
					SYS_ERR_I2C_TO,			// 8 i2c timeout
					SYS_ERR_I2C_ACK,		// 9 i2c ACK timeout
					SYS_ERR_ADC_TO,			// 10 adc timeout
					SYS_ERR_XL345_TO,		// 11 accelerometer timeout
					SYS_ERR_XL345ID			// 12 accelerometer id
				};

//******************************************************************************
//	Port 2 equates
#define SW_1		0x01			// P2.0
#define SW_2		0x02			// P2.1
#define SW_3		0x04			// P2.2
#define SW_4		0x08			// P2.3
#define BK_LGT		0x10			// P2.4
#define LED_1		0x40			// P2.6
#define LED_2		0x80			// P2.7

//	Port 3 equates
#define LCD_RST		0x01			// P3.0
#define SDA			0x02			// P3.1 - i2c data
#define SCL			0x04			// P3.2 - i2c clock
#define LED_3		0x08			// P3.3

//	Port 4 equates
#define R_POT		0x08			// P4.3
#define L_POT		0x10			// P4.4
#define SPKR		0x20			// P4.5
#define LED_4		0x40			// P4.6

//******************************************************************************
//	LED's
#define LED_4_ON			P4OUT |= LED_4;
#define LED_4_OFF			P4OUT &= ~LED_4;
#define LED_4_TOGGLE		P4OUT ^= LED_4;

#define LED_3_ON			P3OUT |= LED_3;
#define LED_3_OFF			P3OUT &= ~LED_3;
#define LED_3_TOGGLE		P3OUT ^= LED_3;

#define LED_2_ON			P2OUT |= LED_2;
#define LED_2_OFF			P2OUT &= ~LED_2;
#define LED_2_TOGGLE		P2OUT ^= LED_2;

#define LED_1_ON			P2OUT |= LED_1;
#define LED_1_OFF			P2OUT &= ~LED_1;
#define LED_1_TOGGLE		P2OUT ^= LED_1;

#define LED_RED_ON			P1OUT |= 0x01;
#define LED_RED_OFF			P1OUT &= ~0x01;
#define LED_RED_TOGGLE		P1OUT ^= 0x01;

#define LED_GREEN_ON		P1OUT |= 0x02;
#define LED_GREEN_OFF		P1OUT &= ~0x02;
#define LED_GREEN_TOGGLE	P1OUT ^= 0x02;

//******************************************************************************
//
#define SPEAKER_ON			P4OUT |= SPKR;
#define SPEAKER_OFF			P4OUT &= ~SPKR;
#define SPEAKER_TOGGLE		P4OUT ^= ~SPKR;

#define BACKLIGHT_ON		P2OUT |= BK_LGT;	// turn on backlight
#define BACKLIGHT_OFF		P2OUT &= ~BK_LGT;	// turn off backlight

//******************************************************************************
//	i2c equates
#if REV_C == 1
#define ADXL345_ADR			0x1d				// ADXL345 accelerometer
#else
#define ADXL345_ADR			0x53
#endif
#define LCD_ADDRESS			0x3f				// LCD display
#define FRAM_ADR			0x50				// F-RAM

//#define I2C_FSCL	200							// ~200kHz
#define I2C_FSCL	400							// ~400kHz

#define I2C_CLOCK_LOW		P3OUT &= ~SCL		// put clock low
#define I2C_CLOCK_HIGH		P3OUT |= SCL		// put clock high

#define I2C_DATA_LOW		P3OUT &= ~SDA		// put data low
#define I2C_DATA_HIGH		P3OUT |= SDA		// put data high

//******************************************************************************
//	prototypes
//
//	i2c prototypes
void i2c_write(uint16 address, uint8* data, int16 bytes);
int i2c_write2(uint16 address, uint8* data, int16 bytes);
uint8 i2c_read(uint16 address, uint8* buffer, int16 bytes);

//	eZ430X prototypes
int eZ430X_init(enum _430clock clock);
void init_USCI_B0(uint16 i2c_address);
void ERROR2(int16 error);
void wait(uint16 time);

//******************************************************************************
#endif /*eZ430X_H_*/
