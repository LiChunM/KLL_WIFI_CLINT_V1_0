#ifndef __ADC_H__
#define __ADC_H__	 
#include "sys.h"


#define ADC_BASE		3300

__packed typedef struct 
{
	u16 PA0VOL;			
	u16 PA1VOL;
	u16 PA2VOL;		
	u16 PA3VOL;		
	u16 PA4VOL;		
}_adc_values;

extern _adc_values	adcvols;

void  Adc_Init(void);
u16 Get_Adc(u8 ch);
u16 Get_Adc_Average(u8 ch,u8 times);
void Update_SysVol(void);

#endif

