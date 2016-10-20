#include "delay.h"
#include "usart3.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	   
#include "analysis.h"

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					
#endif


u8 USART3_RX_BUF[USART3_MAX_RECV_LEN];  
u16 USART3_RX_STA=0;    

void USART3_IRQHandler(void)
{
	u8 res; 
	 if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	 	{
	 		 res =USART_ReceiveData(USART3);
			  if(USART3_RX_STA<USART3_MAX_RECV_LEN)
			  	{
			  		 TIM_SetCounter(TIM2,0);
					  if(USART3_RX_STA==0)TIM2_Set(1);
					   USART3_RX_BUF[USART3_RX_STA++]=res;
			  	}
			  else
			  	{
			  		USART3_RX_STA|=1<<15;
			  	}
	 	}

}

void USART3_Init(u32 bound)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	USART_DeInit(USART3);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_Cmd(USART3, ENABLE); 
#ifdef USART3_RX_EN 
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM2_Init(999,7199); 
	USART3_RX_STA=0;
	TIM2_Set(0);
#endif
}

void TIM2_IRQHandler(void)
{
	 if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	 	{
	 		 USART3_RX_STA|=1<<15;
			 TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 
			 TIM2_Set(0); 
	 	}
}

void TIM2_Set(u8 sta)
{
	 if(sta)
	 	{
	 		TIM_SetCounter(TIM2,0);
			 TIM_Cmd(TIM2, ENABLE);
	 	}
	 else TIM_Cmd(TIM2, DISABLE);
}

void TIM2_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler =psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	 TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );
	  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	  NVIC_Init(&NVIC_InitStructure);

}

void Usart3CommandAnalysis(void)
{
	u8 lens,i;
	u8 buf[100]={0};
	if(USART3_RX_STA&0X8000)
		{
			lens=USART3_RX_STA&0X7FF;
			USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;
			for(i=0;i<lens;i++)buf[i]=USART3_RX_BUF[i];
			if(SystemDebug==2)printf("%s\r\n",USART3_RX_BUF);
			USART3_RX_STA=0;
			atk_8266_recive_data(buf,lens);
		}
}



void USART3_CMD(unsigned char *lb)
{
    while(*lb)
    {
        USART_SendData(USART3,*lb);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;
    }
}

void USART3_DATA(unsigned char *lb,unsigned int len)
{
   
    unsigned int i;
    for(i=0;i<len;i++)
   {
        USART_SendData(USART3,*lb);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;

   }
    
}


