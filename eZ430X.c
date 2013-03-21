//	eZ430X.c - eZ430X REV C board system functions
//******************************************************************************
//******************************************************************************
//	        ZZZZZZZZ     44     3333      0000    XX    XX
//	             ZZ     444    33  33    00  00    XX  XX
//	 eeee       ZZ     4444         33  00    00    XXXX           ccccc
//	ee  ee     ZZ     44 44      3333   00    00     XX           cc   cc
//	eeeee     ZZ     44444444       33  00    00    XXXX         cc
//	ee       ZZ          44    33  33    00  00    XX  XX   xxx   cc   cc
//	 eeee   ZZZZZZZZ     44     3333      0000    XX    XX  xxx    ccccc
//
//	Author:			Paul Roper, Brigham Young University
//	Revision:		1.0		02/15/2010
//					1.1		04/22/2010	I2C_FSCL updated to 400 khz
//					1.2		11/04/2010	typedef's included in eZ430X.h
//					1.3		12/06/2010	wait(i2c_fSCL * 700) - depends on speed
//					1.4		03/24/2011	retry for i2c_write
//										eZ430X_init uses constants for clock
//
//	Description:	Initialization firmware for eZ430X Development Board
//
//	See eZ430X.h for eZ430X and i2c prototypes
//
//	                  MSP430F2274 (REV C)
//	               .----------------------.             LEFT         RIGHT
//	     RED LED-->|P1.0    (ACLK/A0) P2.0|<--SW_1
//	   GREEN LED-->|P1.1   (SMCLK/A1) P2.1|<--(H)       SW_2         EZ0_TX
//	eZ430 BUTTON-->|P1.2     (TA0/A2) P2.2|<--(D)       ADXL_INT     SW_3
//	               |         (TA1/A3) P2.3|<--(E)       SW_4         SERVO_3
//	               |         (TA2/A4) P2.4|-->(F)       LCD_BLT      SERVO_4
//	               |              XIN P2.6|-->LED_1
//	               |             XOUT P2.7|-->LED_2
//                 |     (UCB0STE/A5) P3.0|-->i2c_RST
//                 |        (UCB0SDA) P3.1|<->i2c_SDA
//                 |        (UCB0SCL) P3.2|-->i2c_SCL
//                 |        (UCB0CLK) P3.3|-->LED_3
//	               |        (TB0/A12) P4.3|-->(G)       RPOT         EZ0_AN
//	               |        (TB1/A13) P4.4|-->(A)       LPOT         SERVO_1
//	               |        (TB2/A14) P4.5|-->(B)       TRANSDUCER   SERVO_2
//	               |     (TBOUTH/A15) P4.6|-->(C)       THERMISTOR   LED_4
//	               '----------------------'
//
//  Built with Code Composer Essentials Version: 3.1
//******************************************************************************
//******************************************************************************
#include <setjmp.h>
#include "msp430x22x4.h"
#include "eZ430X.h"

static void tdelay(uint16 time);

#define I2C_BIT_BANG	0

//******************************************************************************
//******************************************************************************
//	LCD Global variables (NOT ZERO'D!!!)
//
jmp_buf i2c_context;			// error context
uint16 i2c_fSCL;				// i2c timing constant
uint8 TXData[8];				// i2c tx buffer

//******************************************************************************
//	Initialization sequence for eZ430X MSP430F2274
//
int eZ430X_init(enum _430clock clock)
{
	WDTCTL = WDTPW + WDTHOLD;				// Stop WDT

	// 	MSP430 Clock - Set DCO to 1-16 MHz:
	switch (clock)
	{
		case _1MHZ:
			BCSCTL1 = CALBC1_1MHZ;			// Set range 1MHz
			DCOCTL = CALDCO_1MHZ;			// Set DCO step + modulation
			i2c_fSCL = (1200/I2C_FSCL);		// fSCL
			break;

		case _8MHZ:
			BCSCTL1 = CALBC1_8MHZ;			// Set range 8MHz
			DCOCTL = CALDCO_8MHZ;			// Set DCO step + modulation
			i2c_fSCL = (8000/I2C_FSCL);		// fSCL
			break;

		case _12MHZ:
			BCSCTL1 = CALBC1_12MHZ;			// Set range 12MHz
			DCOCTL = CALDCO_12MHZ;			// Set DCO step + modulation
			i2c_fSCL = (12000/I2C_FSCL);	// fSCL
			break;

		case _16MHZ:
			BCSCTL1 = CALBC1_16MHZ;			// Set range 16MHz
			DCOCTL = CALDCO_16MHZ;			// Set DCO step + modulation
			i2c_fSCL = (16000/I2C_FSCL);	// fSCL
			break;

		default:
			ERROR2(SYS_ERR_430init);
//			return 1;						// failure!
	}
//	                     MSP430F2274 (REV C)
//	               .-----------------------------.          LEFT         RIGHT
//	     RED LED<--|P1.0           (ACLK/A0) P2.0|<--SW_1
//	   GREEN LED<--|P1.1          (SMCLK/A1) P2.1|<--(H)    SW_2         EZ0_TX
//	eZ430 BUTTON-->|P1.2^           (TA0/A2) P2.2|<--(D)    ADXL_INT     SW_3
//	        N.C.-->|P1.3^           (TA1/A3) P2.3|<->(E)    SW_4         SERVO_3
//	        N.C.-->|P1.4^           (TA2/A4) P2.4|-->(F)    LCD_BLT      SERVO_4
//	        N.C.-->|P1.5^                   ^P2.5|<-- N.C.
//	        N.C.-->|P1.6^                XIN P2.6|-->LED_1
//	        N.C.-->|P1.7^               XOUT P2.7|-->LED_2
//                 |                             |
//       i2c_RST<--|P3.0 (UCB0STE/A5)       ^P4.0|<--N.C.
//       i2c_SDA<->|P3.1 (UCB0SDA)          ^P4.1|<--N.C.
//       i2c_SCL<--|P3.2 (UCB0SCL)          ^P4.2|<--N.C.
//  LED_3/EZ0_RX<--|P3.3 (UCB0CLK) (TB0/A12) P4.3|<--(G)    RPOT         EZ0_AN
//	        N.C.-->|P3.4           (TB1/A13) P4.4|<->(A)    LPOT         SERVO_1
//	        N.C.-->|P3.5           (TB2/A14) P4.5|-->(B)    SPEAKER      SERVO_2
//	        N.C.-->|P3.6        (TBOUTH/A15) P4.6|<->(C)    THERMISTOR   LED_4
//	        N.C.-->|P3.7                    ^P4.7|<--N.C.
//	               '-----------------------------'
#define BINARY(a,b,c,d,e,f,g,h)	((((((((a<<1)+b<<1)+c<<1)+d<<1)+e<<1)+f<<1)+g<<1)+h)
	// configure P1
	P1OUT = BINARY(1,1,1,1,1,1,0,0);		// turn off all output pins
	P1SEL = BINARY(0,0,0,0,0,0,0,0);		// select GPIO
	P1REN = BINARY(1,1,1,1,1,1,0,0);		// pull-up P1.2-7
	P1DIR = BINARY(0,0,0,0,0,0,1,1);		// P1.0-1 output, P1.2-7 input

	// configure P2
	P2OUT = BINARY(0,0,1,0,1,1,1,1);		// turn off all output pins
	P2SEL = BINARY(0,0,0,0,0,0,0,0);		// select GPIO
	P2REN = BINARY(0,0,1,0,0,1,1,1);		// pull-up P2.0-2,5 //this is what was fu**ing my servo bi#$h
	P2IES = BINARY(0,0,0,0,1,1,1,1);		// high to low transition
	P2DIR = BINARY(1,1,0,1,0,0,0,0);		// P2.4,6,7 output, P2.0-3,5 input

	// configure P3: USCI_B0 for i2c operations
	//		P3.0/UCB0STE	output (used for LCD reset)
	//      P3.1/UCB0SDA	input
	//	    P3.2/UCB0SCL	output
	//      P3.3/UCB0CLK	output (not used)
	P3OUT = BINARY(1,1,1,1,0,0,1,0);		// turn off all output pins (~CS reset LCD)
	P3SEL = BINARY(0,0,0,0,1,1,1,0);		// SCA, SCL, CLK USCI_B0 option select
	P3REN = BINARY(1,1,1,1,0,0,0,0);		// pull-up P3.4-7
	P3DIR = BINARY(0,0,0,0,1,1,0,1);		// P3.0,2-3 output, P3.1,4-7 input
	wait(10);								// delay
	P3OUT |= 0x01;							// bring LCD CS high
	P3SEL = BINARY(0,0,0,0,0,1,1,0);		// Assign I2C pins to USCI_B0
	init_USCI_B0(LCD_ADDRESS);				// USCI_B0 for i2c

	// configure P4
	P4OUT = BINARY(1,0,0,0,0,1,1,1);		// turn off all output pins
	P4SEL = BINARY(0,0,0,0,0,0,0,0);		// select GPIO
	P4REN = BINARY(1,0,0,0,0,1,1,1);		// pull-up P4.0-2,7
	P4DIR = BINARY(0,1,1,1,0,0,0,0);		// P4.4-6 output, P4.0-3,7 input

	return 0;								// success
} // end eZ430X_init


//******************************************************************************
//	Init Universal Synchronous Controller
//
void init_USCI_B0(uint16 address)
{
	UCB0CTL1 |= UCSWRST;					// Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;	// I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_3 + UCSWRST;			// Use SMCLK, keep SW reset
	UCB0BR0 = i2c_fSCL;						// 1.2 MHz fSCL = SMCLK/12 = ~200kHz
	UCB0BR1 = 0;
	UCB0I2CSA = address;					// Slave Address
	UCB0STAT = 0;
	UCB0CTL1 &= ~UCSWRST;					// Clear SW reset, resume operation
	return;
} // init_USCI_B0


//******************************************************************************
//	write data to i2c address
//
//	IN:	address = i2c address
//		  *data = data array
//		  bytes = # of bytes to write
//
//	longjmp's to i2c_context on error
//
void i2c_write(uint16 address, uint8* data, int16 bytes)
{
	int error;
	int retries = 1000;
	while (retries--)
	{
		error = i2c_write2(address, data, bytes);
		if (error == 0) return;		// success
	}
	longjmp(i2c_context, error);	// return failure error
} // end i2c_write


#if I2C_BIT_BANG==0

int wait_i2c_stop(void)
{
	int timeout = 1;
	while ((UCB0CTL1 & UCTXSTP) && ++timeout);
	if (timeout == 0) return SYS_ERR_I2C_TO;
	return 0;
} // end wait_i2c_stop

int wait_i2c_ack(void)
{
	int error;
	int timeout = 0;

	// wait for start to be sent or data to more to shift register
	while ( !((IFG2 & UCB0TXIFG) || (UCB0STAT & UCNACKIFG)) )
	{
		if (--timeout == 0) return SYS_ERR_I2C_TO;
	}
	if (UCB0STAT & UCNACKIFG)		// check for nack
	{
		UCB0STAT &= UCNACKIFG;		// nack
		UCB0CTL1 |= UCTXSTP;		// generate stop on next ack
		if (error = wait_i2c_stop()) return error;
		return SYS_ERR_I2C_ACK;		// failure
	}
	return 0;						// success
} // end wait_i2c_ack


int i2c_write2(uint16 address, uint8* data, int16 bytes)
{
	int error;
	LED_RED_ON;						// turn on P1.0

	// ensure stop condition was sent
	if (error = wait_i2c_stop()) return error;
	UCB0I2CSA = address;			// slave address
	UCB0CTL1 |= UCTR + UCTXSTT; 	// i2c TX, start condition
	while (bytes--)					// send data
	{
		if (error = wait_i2c_ack()) return error;
		UCB0TXBUF = *data++;		// load TX buffer
	}
	if (error = wait_i2c_ack()) return error;
	UCB0CTL1 |= UCTXSTP;			// generate stop on next ack
	// wait for stop condition to be sent
	if (error = wait_i2c_stop()) return error;

	LED_RED_OFF;					// turn off P1.0
	return 0;						// return success
} // end i2c_write

#else

//******************************************************************************
//	write data to i2c address
//
//	IN:	address = i2c address
//		  *data = data array
//		  bytes = # of bytes to write
//
//	longjmp's to i2c_context on error
//
#define I2C_DELAY	10
void i2c_clocklow()
{
	int delay;
	delay = I2C_DELAY;
	while (--delay);
	I2C_CLOCK_LOW;					// put clock low
	delay = I2C_DELAY;
	while (--delay);
	return;
} // end clocklow

void i2c_clockhigh()
{
	int delay;
	delay = I2C_DELAY;
	while (--delay);
	I2C_CLOCK_HIGH;					// put clock high
	delay = I2C_DELAY;
	while (--delay);
	return;
} // end clockhigh

int i2c_out8bits(uint8 c)
{
	uint8 shift = 0x80;
	while (shift)
	{
		i2c_clocklow();
		P3DIR |= SDA;				// turn SDA to output
		if (c & shift) I2C_DATA_HIGH;	// set SDA high
		else I2C_DATA_LOW;				// set SDA low
		i2c_clockhigh();
		shift >>= 1;				// adjust mask
	}
	// look for slave ack
	i2c_clocklow();
	I2C_DATA_HIGH;
	P3DIR &= ~SDA;				// turn SDA to input
	i2c_clockhigh();
	if (P3IN & SDA) return SYS_ERR_I2C_ACK;
	I2C_DATA_HIGH;				// set SDA high
	P3DIR |= SDA;				// turn SDA to output
	return 0;
} // end out8bits


int i2c_write2(uint16 address, uint8* data, int16 bytes)
{
	int error;

	LED_RED_ON;
	I2C_CLOCK_HIGH;				// set SCL high
	I2C_DATA_HIGH;				// set SDA high

	P3DIR |= 0x06;				// set as output
	P3SEL &= ~0x06;				// change SDA & SCL to GPIO pins

	address = (address << 1) + 0;	// address * 2 + write bit
	I2C_DATA_LOW;				// start (SDA low while SCL high)

	// output address
	if (error = i2c_out8bits(address)) return error;

	// write 8 bits
	while (bytes--)
	{
		// output data
		if (error = i2c_out8bits(*data++)) return error;
	}

	// send stop condition
	I2C_DATA_LOW;					// set SDA low

	// restore to USCI_B0
	I2C_CLOCK_HIGH;					// leave clock & data high
	I2C_DATA_HIGH;
	P3SEL |= 0x06;					// re-assign I2C pins to USCI_B0
	LED_RED_OFF;
	return 0;						// success!
} // end new_i2c_write
#endif


//******************************************************************************
//	read bytes into buffer using i2c
//
//	turn on RED LED during any activity
//
//	IN:		address	i2c address
//			buffer	pointer to input buffer
//			bytes	# of bytes to read
//	OUT:	last byte read
//
uint8 i2c_read(uint16 address, uint8* buffer, int16 bytes)
{
	uint8 shift = 0x80;
	uint8 i, data;

	// change form USCI_B0 to bit banging
	LED_RED_ON;
	I2C_CLOCK_HIGH;					// make sure clock is high
	I2C_DATA_HIGH;					// make sure data is high
	P3DIR |= 0x06;					// set as output
	P3SEL &= ~0x06;					// change SDA & SCL to GPIO pins

	address = (address << 1) + 1;	// address * 2 + read bit
	I2C_DATA_LOW;					// send start condition

	while (shift)
	{
		I2C_CLOCK_LOW;
		if (address & shift)
			 I2C_DATA_HIGH;			// set SDA high
		else I2C_DATA_LOW;			// set SDA low
		I2C_CLOCK_HIGH;
		shift >>= 1;				// adjust mask
	}

	// look for slave ack
	I2C_CLOCK_LOW;
	P3DIR &= ~SDA;					// turn SDA to input
	I2C_CLOCK_HIGH;
	if (P3IN & SDA) longjmp(i2c_context, SYS_ERR_I2C_ACK);

	// read 8 bits
	while (bytes--)
	{
		for (i = 0; i < 8; i++)
		{
			I2C_CLOCK_LOW;
			P3DIR &= ~SDA;			// SDA as input
			data <<= 1;				// assume 0
			I2C_CLOCK_HIGH;
			if (P3IN & SDA) data++;
		}
		// save data
		*buffer++ = data;

		// output ack or nack
		I2C_CLOCK_LOW;
		if (bytes) I2C_DATA_LOW;	// ack (0)
		else I2C_DATA_HIGH;			// nack (1)
		P3DIR |= SDA;				// turn SDA back to output
		I2C_CLOCK_HIGH;
	}

	// send stop condition
	I2C_CLOCK_LOW;
	P3DIR |= SDA;					// make sure SDA is output
	I2C_DATA_LOW;					// put SDA low

	// restore to USCI_B0
	I2C_CLOCK_HIGH;					// leave clock & data high
	I2C_DATA_HIGH;
	P3SEL |= 0x06;					// re-assign I2C pins to USCI_B0
	LED_RED_OFF;
	return data;
} // end i2c_read


//******************************************************************************
//	delay routines
//
//	i2c_fSCL =	40 @16 mHz		x 700 = 28000
//				30 @ 12 mHz		x 700 = 21000
//				20 @ 8 mHz		x 700 = 14000
//				3 @ 1.2 mHz		x 700 = 2100
//
void tdelay(uint16 time)
{
	while(time-- > 0);					// count down to zero
} // end tdelay

void wait(uint16 time)
{
	int i;
	for (i=0; i<time; i++) tdelay(i2c_fSCL * 700);
	return;
} // end wait


//******************************************************************************
//	report hard error
//
void ERROR2(int16 error)
{
	int i, j;

	// return if no error
	if (error == 0) return;

	__bic_SR_register(GIE);			// disable interrupts
	WDTCTL = WDTPW + WDTHOLD;		// Stop WDT
	BCSCTL1 = CALBC1_1MHZ;			// Set range 1MHz
	DCOCTL = CALDCO_1MHZ;			// Set DCO step + modulation

	while (1)
	{
		// pause
		LED_RED_OFF;
		for (i=1; i<4; i++) for (j=1; j; j++);

		// flash LED's 10 times
		i = 10;
		while (i--)
		{
			LED_RED_TOGGLE;
			for (j=1; j<8000; j++);
		}

		// pause
		LED_RED_OFF;
		for (i=1; i<2; i++) for (j=1; j; j++);

		// now blink error #
		for (i = 0; i < error; i++)
		{
			LED_RED_ON;
			for (j=1; j; j++);
			LED_RED_OFF;
			for (j=1; j; j++);
		}
	}
} // end ERROR


//******************************************************************************
//	USCI interrupt service routine
//
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	// should not come here!!!!!!!
	ERROR2(SYS_ERR_USCB_RX);
	__bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
	return;
}
