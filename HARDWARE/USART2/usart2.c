#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	   

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					
#endif


#ifdef USART2_RX_EN   								//���ʹ���˽���   	  
//���ڽ��ջ����� 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.

u16 USART2_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	u8 res;	 
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART2->SR&(1<<5))//���յ�����
	{	 
		res=USART2->DR; 			 
		if(USART2_RX_STA<USART2_MAX_RECV_LEN)		//�����Խ�������
		{
			TIM4->CNT=0;         					//���������
			if(USART2_RX_STA==0)TIM4_Set(1);	 	//ʹ�ܶ�ʱ��4���ж� 
			USART2_RX_BUF[USART2_RX_STA++]=res;		//��¼���յ���ֵ	 
		}else 
		{
			USART2_RX_STA|=1<<15;					//ǿ�Ʊ�ǽ������
		} 
	}  			
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntExit();  											 
#endif
}   
//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void USART2_Init(u32 pclk1,u32 bound)
{  	 		 
	RCC->APB2ENR|=1<<8;   	//ʹ��PORTG��ʱ��  
 	GPIOG->CRH&=0XFFFFFF0F;	//IO״̬����
	GPIOG->CRH|=0X00000030;	//IO״̬����
	RCC->APB2ENR|=1<<2;   	//ʹ��PORTA��ʱ��  
	GPIOA->CRL&=0XFFFF00FF;	//IO״̬����
	GPIOA->CRL|=0X00008B00;	//IO״̬����	 
	RCC->APB1ENR|=1<<17;  	//ʹ�ܴ���ʱ�� 	 
	RCC->APB1RSTR|=1<<17;   //��λ����2
	RCC->APB1RSTR&=~(1<<17);//ֹͣ��λ	   	   
	//����������
 	USART2->BRR=(pclk1*1000000)/(bound);// ����������	 
	USART2->CR1|=0X200C;  	//1λֹͣ,��У��λ.
#ifdef USART2_RX_EN		  	//���ʹ���˽���
	//ʹ�ܽ����ж�
	USART2->CR1|=1<<8;    	//PE�ж�ʹ��
	USART2->CR1|=1<<5;    	//���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(2,3,USART2_IRQn,2);//��2��������ȼ� 
	TIM4_Init(99,7199);		//10ms�ж�
	USART2_RX_STA=0;		//����
	TIM4_Set(0);			//�رն�ʱ��4
#endif										  	
}

//��ʱ��4�жϷ������		    
void TIM4_IRQHandler(void)
{ 	
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntEnter();    
#endif
	if(TIM4->SR&0X01)//�Ǹ����ж�
	{	 			   
		USART2_RX_STA|=1<<15;	//��ǽ������
		TIM4->SR&=~(1<<0);		//����жϱ�־λ		   
		TIM4_Set(0);			//�ر�TIM4  
	}	
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntExit();  											 
#endif
}
//����TIM4�Ŀ���
//sta:0���ر�;1,����;
void TIM4_Set(u8 sta)
{
	if(sta)
	{
    	TIM4->CNT=0;         //���������
		TIM4->CR1|=1<<0;     //ʹ�ܶ�ʱ��4
	}else TIM4->CR1&=~(1<<0);//�رն�ʱ��4	   
}
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM4_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM4ʱ��ʹ��    
 	TIM4->ARR=arr;  	//�趨�������Զ���װֵ   
	TIM4->PSC=psc;  	//Ԥ��Ƶ��
 	TIM4->DIER|=1<<0;   //��������ж�				
 	TIM4->CR1|=0x01;  	//ʹ�ܶ�ʱ��4	  	   
   	MY_NVIC_Init(2,3,TIM4_IRQn,2);//��ռ2�������ȼ�3����2	��2�����ȼ����								 
}
#endif		 


void USART2_CMD(unsigned char *lb)
{
    while(*lb)
    {
        USART_SendData(USART2,*lb);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;
    }
}

void USART2_DATA(unsigned char *lb,unsigned int len)
{
   
    unsigned int i;
    for(i=0;i<len;i++)
   {
        USART_SendData(USART2,*lb);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        {
        }
        lb ++;

   }
    
}

void Usart2CommandAnalysis(void)
{
	u8 i,rxlen;
	u8 USART2_TXX_BUF[100]={0};
	if(USART2_RX_STA&0X8000)		
	{ 
		rxlen=USART2_RX_STA&0X7FFF;	
		for(i=0;i<rxlen;i++)USART2_TXX_BUF[i]=USART2_RX_BUF[i];	   
	 	USART2_RX_STA=0;		
		USART2_TXX_BUF[i]=0;
	}
}

