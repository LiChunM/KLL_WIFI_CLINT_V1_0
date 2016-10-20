#include "power_drv.h"
#include "sys.h"
#include "includes.h"

u8 Config_Byte;

void power_drv_init(void)
{
     	GPIO_InitTypeDef  GPIO_InitStructure;
     	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);					
 	GPIO_ResetBits(GPIOA,GPIO_Pin_8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;			
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
 	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
}


void ACC_Init(void)
{
     GPIO_InitTypeDef  GPIO_InitStructure;
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14;			
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;			
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

