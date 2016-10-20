#include "delay.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	   
#include "adc.h"
#include "stm32f10x_adc.h"


_adc_values	adcvols;

void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1, ENABLE );	  
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);   


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	
	ADC_DeInit(ADC1);  

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	ADC_InitStructure.ADC_NbrOfChannel = 5;	
	ADC_Init(ADC1, &ADC_InitStructure);	

  
	ADC_Cmd(ADC1, ENABLE);	
	
	ADC_ResetCalibration(ADC1);	
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	
	
	ADC_StartCalibration(ADC1);	 
 
	while(ADC_GetCalibrationStatus(ADC1));	
 

}			


u16 Get_Adc(u8 ch)   
{
  	
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ch, 2, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ch, 3, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ch, 4, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ch, 5, ADC_SampleTime_239Cycles5 );
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));

	return ADC_GetConversionValue(ADC1);	
}


u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 

void Update_SysVol(void)
{
	u16 adcvalue;
	adcvalue=Get_Adc_Average(ADC_Channel_0, 10);
	adcvols.PA0VOL= adcvalue*ADC_BASE/4096;
	adcvalue=Get_Adc_Average(ADC_Channel_1, 10);
	adcvols.PA1VOL= adcvalue*ADC_BASE/4096;
	adcvalue=Get_Adc_Average(ADC_Channel_2, 10);
	adcvols.PA2VOL= adcvalue*ADC_BASE/4096;
	adcvalue=Get_Adc_Average(ADC_Channel_3, 10);
	adcvols.PA3VOL= adcvalue*ADC_BASE/4096;
	adcvalue=Get_Adc_Average(ADC_Channel_4, 10);
	adcvols.PA4VOL= adcvalue*ADC_BASE/4096;
}

