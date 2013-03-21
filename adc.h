//******************************************************************************
//	ADC
//******************************************************************************
#ifndef ADC_H_
#define ADC_H_

#if REV_C == 1
#define RIGHT_POT			12
#define LEFT_POT			13
#else
#define LEFT_POT			12
#define RIGHT_POT			13
#endif

#define TEMPERATURE			10
#define THERMISTOR			15

//	ADC Prototypes
int ADC_init(void);
int ADC_read(int channel);

#endif /*ADC_H_*/

