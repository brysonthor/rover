//******************************************************************************
//******************************************************************************
//
//	    AA      DDDDDD      CCCCC
//	   AAAA     DD   DD    CC   CC
//	  AA  AA    DD    DD  CC
//	 AA    AA   DD    DD  CC
//	AAAAAAAAAA  DD    DD  CC
//	AA      AA  DD   DD    CC   CC
//	AA      AA  DDDDDD      CCCCC
//
//
//   Author:	Paul Roper, Brigham Young University
//
//   Revisions:	03/2010		Original code
//				07/26/2011	Cleared ADC10AE0 
//
//   Built with Code Composer Studio Version: 4.2.3.00004 
//*******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "msp430x22x4.h"
#include "eZ430X.h"

//******************************************************************************
//	initialize A/D converter
//
int ADC_init(void)
{
//	ADC10CTL0 = SREF0 | ADC10SHT_2 | ADC10ON | ADC10IE;
//	ADC10AE0 = 0x00;					// disable A0-A7
//	ADC10AE1 = 0xb0;					// enable A15, A13, A12
	return 0;
} // end ADC_init


//****************************************************************
//	read A/D converted value
//
//		SREF0		VR+ = VREF+ and VR- = VSS
//		ADC10SHT_2	16 x ADC10CLKs
//		ADC10ON		ADC10 on
//		REFON		Reference on
//		REF2_5V		2.5v internal reference
//
//		channel 10 = internal temperature
//		channel 12 = left potentiometer
//		channel 13 = right potentiometer
//		channel 15 = thermistor
//
//		| 15  14  13| 12  11| 10| 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//		  0   0   0   1   1   0   0   0   0   0   0   0   0   0   0   0
//
int ADC_read(int channel)
{
	int result, timeout;

	ADC10CTL0 = SREF0 | ADC10SHT_2 | ADC10ON | REFON | REF2_5V;
	if (channel == 10)
	{
		// internal temperature

		// delay 30 us to allow Ref to settle
		timeout = 30*8;
		while (--timeout);
	}

	if (channel == 12)
	{
		// P4.3 -> Right Potentiometer
		P4DIR &= ~0x08;					// A12 = P4.3
		P4SEL |= 0x08;
		ADC10AE0 = 0x00;
		ADC10AE1 = 0x10;				// P4.3 ADC10 function and enable
	}
	else if (channel == 13)
	{
		// P4.4 -> Left Potentiometer
		P4DIR &= ~0x10;					// A13 - P4.4
		P4SEL |= 0x10;
		ADC10AE0 = 0x00;
		ADC10AE1 = 0x20;				// P4.4 ADC10 function and enable
	}
	else if (channel == 15)
	{
		// P4.6 -> Thermistor
		P4DIR &= ~0x40;					// A15 = P4.6
		P4SEL |= 0x40;
		ADC10AE0 = 0x00;
		ADC10AE1 = 0x80;				// P4.6 ADC10 function and enable
	}
	ADC10CTL1 = channel << 12;
	ADC10CTL0 |= ENC | ADC10SC;			// Sampling and conversion start
	timeout = 1;
	while (!(ADC10CTL0 & ADC10IFG) && ++timeout);
	if (timeout == 0) ERROR2(SYS_ERR_ADC_TO);
	result = ADC10MEM;
	if (result < 0) result = 0;

	ADC10AE0 = 0;
	ADC10AE1 = 0;
	return result;
} // end ADC_sample


//******************************************************************************
//******************************************************************************
// ADC10 interrupt service routine
//
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);		// Clear CPUOFF bit from 0(SR)
  return;
}


