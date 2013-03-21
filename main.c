#include <msp430x22x4.h>
#include "eZ430X.h"
#include <stdio.h>
#include "adc.h"
/*
 * Function	Description	Note
int RBX430_init(enum _430clock clock);					Initialize the LCD reading/writing.
clock is an enumerated value:
enum _430clock {_16MHZ, _12MHZ, _8MHZ, _1MHZ};			MUST BE CALLED BEFORE USING THE LCD!
void lcd_clear(0);										Clears the LCD RAM memory.	Slow function - use sparingly!
uint8 lcd_display(int16 mode);							Sets/clears display modes
0bxxxx xxxx
   \\\\ \\\\___ LCD_PROPORTIONAL      proportional font
    \\\\ \\\___ LCD_REVERSE_FONT      reverse font
     \\\\ \\___ LCD_2X_FONT           2x font
      \\\\ \___ LCD_GRAPHIC_FONT      write to FRAM
       \\\\____ LCD_REVERSE_DISPLAY   reverse display
        \\\____ LCD_BLOCK_CHARACTER   block characters
         \\____
          \____

void lcd_backlight(uint8 backlight);					Turns the LCD backlight on/off	0 = off, 1 = on
uint8 lcd_cursor(uint16 x, uint16 y);					Set the current LCD x/y coordinates.	x=0-159, y=0-159
char lcd_putchar(char c);								Writes a character (c) to the display	Use lcd_cursor to position on LCD
void lcd_printf(char* fmt, ...);						Writes a string to the display	Use 5 x 8 font and writes to current column/page according to conventional C printf parameters.
void lcd_volume(uint8 volume);							Adjusts the LCD brightness	Range of volume is 0-64
void lcd_image(const unsigned char* image, uint16 x, uint16 y);	Write an image to LCD	x = 0-159, y = 0-159
 *
 * */
//*******************************************************************************
//   eZ430X jumper configurations and pin-outs:
//
//      A  P4.4     Left Pot  O--O  O  Servo #1
//      B  P4.5      Speaker  O--O  O  Servo #2
//      C  P4.6   Thermistor  O  O--O  LED_4
//      D  P2.2  ADXL345 INT  O  O--O  SW_3
//      E  P2.3         SW_4  O--O  O  Servo #3
//      F  P2.4    Backlight  O--O  O  Servo #4
//      G  P4.3    Right Pot  O--O  O  EZ0_AN
//      H  P2.1         SW_2  O--O  O  EZ0_TX
//
//	                        MSP430F2274 (REV C)
//                    .-----------------------------.
//         RED LED<---|P1.0        (UCB0STE/A5) P3.0|--->LCD_RST
//       GREEN LED<---|P1.1           (UCB0SDA) P3.1|<-->i2c SDA
//    eZ430 BUTTON--->|P1.2           (UCB0SCL) P3.2|--->i2c SCL
//                N.C.|P1.3           (UCB0CLK) P3.3|--->LED_3
//                N.C.|P1.4                     P3.4| N.C.
//                N.C.|P1.5                     P3.5| N.C.
//                N.C.|P1.6                     P3.6| N.C.
//                N.C.|P1.7                     P3.7| N.C.
//   SW1 (pull-up)--->|P2.0 (ACLK/A0)           P4.0| N.C.
//   SW2 (pull-up)--->|P2.1 (SMCLK/A1)          P4.1| N.C.
//   SW3 (pull-up)--->|P2.2 (TA0/A2)            P4.2| N.C.
//   SW4 (pull-up)--->|P2.3 (TA1/A3)Servo  (TB0/A12) P4.3|<---RIGHT POTENTIOMETER
//   LCD BACKLIGHT<---|P2.4 (TA2/A4)Servo  (TB1/A13) P4.4|<---LEFT POTENTIOMETER Servo
//                N.C.|P2.5           (TB2/A14) P4.5|--->TRANSDUCER Servo
//           LED_1<---|P2.6 XIN    (TBOUTH/A15) P4.6|--->LED_4
//           LED_2<---|P2.7 XOUT                P4.7| N.C.
//                   .-----------------------------.
//
//******************************************************************************
#define myCLOCK          12000000           // clock speed (12 mhz)
#define FREQUENCY(freq)  ((freq*(myCLOCK/1000))/(1000*8))
#define TIMERB_FREQ      FREQUENCY(40000)   // 40 ms
//#define TIMERA_FREQ		  FREQUENCY(40000)
#define SERVO_RIGHT      FREQUENCY(400)     // 0.4 ms
#define SERVO_MIDDLE     FREQUENCY(1500)    // 1.5 ms
#define SERVO_LEFT       FREQUENCY(2600)    // 2.6 ms
#define SERVO_SPEED      12					//the lower the faster it goes
//volatile int WDT_cps_cnt;				// WD cycles per second
extern volatile int left_forward = 1;
extern volatile int right_forward = 1;


void delay(int time){
	int inner_count = 6000;
	int outer_count = 2000;
	int secs = time;
	while(secs){
		//6000*2000 should be the number of cycles in sec... it isn't going to be perfect but it should work
		while(outer_count){
			while(inner_count){
				--inner_count;
			}
			--outer_count;
		}
		--secs;
	}
}
void straight(int time){
	left_forward = 1;
	right_forward = 1;
	delay(time);
}
void left(int degrees){
	left_forward = 0;
	right_forward = 1;
	delay(1);
}
void right(int degrees){
	left_forward = 1;
	right_forward = 0;
	delay(1);
}
void reverse(int time){
	left_forward = 0;
	right_forward = 0;
	delay(time);
}
int main(void) {
	eZ430X_init(_12MHZ);
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	P4DIR |= BIT4 | BIT5;                           // P4.4 (TB1/A13)
	P4SEL |= BIT4 | BIT5;							// P4.5 (TB2/A14)
	P2DIR |= BIT3 | BIT4;
	P2SEL |= BIT3 | BIT4;

	TBCTL |= TBSSEL_2 | ID_3 | MC_1 | TBIE;   // SMCLK/8, up mode, interrupt enable
	TBCCR0 = TIMERB_FREQ;                    // frame rate
	TBCCTL1 = OUTMOD_3;                      // set/reset
	TBCCTL2 = OUTMOD_3;                      // set/reset

	//TACCTL0 = CCIE;
	//TACTL |= TASSEL_2 | ID_3 | MC_1 | TAIE;   // SMCLK/8, up mode, interrupt enable
	//TACCR0 = TIMERA_FREQ;                    // frame rate
	//TACCTL1 = OUTMOD_3;                      // set/reset
	//TACCTL2 = OUTMOD_3;
	__bis_SR_register(GIE);       //interrupts enabled

	while(1){
		straight(2);
		right(90);
	}

	return 0;
}


   // configure TimerB to use TIMERB1_VECTOR

#pragma vector = TIMERB1_VECTOR

__interrupt void TIMERB1_ISR(void)
{

   if (left_forward == 1){
	   TBCCR1 = TIMERB_FREQ - SERVO_LEFT;
   }
   else{
	   TBCCR1 = TIMERB_FREQ - SERVO_RIGHT;
   }

   if (right_forward == 1){
	   TBCCR2 = TIMERB_FREQ - SERVO_RIGHT;
   }
   else{
	   TBCCR2 = TIMERB_FREQ - SERVO_LEFT;
   }
   __bic_SR_register_on_exit(LPM0_bits);    // Clear CPUOFF bit from 0(SR)



} // end TIMERB1_ISR


#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{

}

#pragma vector = NMI_VECTOR
__interrupt void NMI_ISR(void)
{}
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{}
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{}
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void)
{
	/*static int servo_speed = 16;
		   static int servo_pulse = SERVO_RIGHT;
		   static int servo_direction = 1;

		   if (TAIV != 0x000A) return;              // acknowledge interrupt
		   if (servo_direction)
		   {
		      servo_pulse += FREQUENCY(1800) / SERVO_SPEED;
		      if (servo_pulse > SERVO_LEFT)
		      {
		         servo_direction = 0;
		      }
		   }
		   else
		   {
		      servo_pulse -= FREQUENCY(1800) / SERVO_SPEED;
		      if (servo_pulse < SERVO_RIGHT)
		      {
		         servo_direction = 1;
		      }
		   }
		   //TBCCR1 = TIMERB_FREQ - servo_pulse;
		   //TBCCR2 = TIMERB_FREQ - servo_pulse;
		   TACCR2 = TIMERA_FREQ - servo_pulse;
		   TACCR1 = TIMERA_FREQ - servo_pulse;
		//   sys_event |= TIMERB_INT;
//		LED_GREEN_TOGGLE;
		__bic_SR_register_on_exit(LPM0_bits);    // Clear CPUOFF bit from 0(SR)
 */
}
#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void)
{

} // end TIMERB1_ISR

#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR(void)
{}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{}
