#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	   

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					
#endif


#ifdef USART2_RX_EN   								//如果使能了接收   	  
//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.

u16 USART2_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	u8 res;	 
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntEnter();    
#endif
	if(USART2->SR&(1<<5))//接收到数据
	{	 
		res=USART2->DR; 			 
		if(USART2_RX_STA<USART2_MAX_RECV_LEN)		//还可以接收数据
		{
			TIM4->CNT=0;         					//计数器清空
			if(USART2_RX_STA==0)TIM4_Set(1);	 	//使能定时器4的中断 
			USART2_RX_BUF[USART2_RX_STA++]=res;		//记录接收到的值	 
		}else 
		{
			USART2_RX_STA|=1<<15;					//强制标记接收完成
		} 
	}  			
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntExit();  											 
#endif
}   
//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void USART2_Init(u32 pclk1,u32 bound)
{  	 		 
	RCC->APB2ENR|=1<<8;   	//使能PORTG口时钟  
 	GPIOG->CRH&=0XFFFFFF0F;	//IO状态设置
	GPIOG->CRH|=0X00000030;	//IO状态设置
	RCC->APB2ENR|=1<<2;   	//使能PORTA口时钟  
	GPIOA->CRL&=0XFFFF00FF;	//IO状态设置
	GPIOA->CRL|=0X00008B00;	//IO状态设置	 
	RCC->APB1ENR|=1<<17;  	//使能串口时钟 	 
	RCC->APB1RSTR|=1<<17;   //复位串口2
	RCC->APB1RSTR&=~(1<<17);//停止复位	   	   
	//波特率设置
 	USART2->BRR=(pclk1*1000000)/(bound);// 波特率设置	 
	USART2->CR1|=0X200C;  	//1位停止,无校验位.
#ifdef USART2_RX_EN		  	//如果使能了接收
	//使能接收中断
	USART2->CR1|=1<<8;    	//PE中断使能
	USART2->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(2,3,USART2_IRQn,2);//组2，最低优先级 
	TIM4_Init(99,7199);		//10ms中断
	USART2_RX_STA=0;		//清零
	TIM4_Set(0);			//关闭定时器4
#endif										  	
}

//定时器4中断服务程序		    
void TIM4_IRQHandler(void)
{ 	
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntEnter();    
#endif
	if(TIM4->SR&0X01)//是更新中断
	{	 			   
		USART2_RX_STA|=1<<15;	//标记接收完成
		TIM4->SR&=~(1<<0);		//清除中断标志位		   
		TIM4_Set(0);			//关闭TIM4  
	}	
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntExit();  											 
#endif
}
//设置TIM4的开关
//sta:0，关闭;1,开启;
void TIM4_Set(u8 sta)
{
	if(sta)
	{
    	TIM4->CNT=0;         //计数器清空
		TIM4->CR1|=1<<0;     //使能定时器4
	}else TIM4->CR1&=~(1<<0);//关闭定时器4	   
}
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM4_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2;	//TIM4时钟使能    
 	TIM4->ARR=arr;  	//设定计数器自动重装值   
	TIM4->PSC=psc;  	//预分频器
 	TIM4->DIER|=1<<0;   //允许更新中断				
 	TIM4->CR1|=0x01;  	//使能定时器4	  	   
   	MY_NVIC_Init(2,3,TIM4_IRQn,2);//抢占2，子优先级3，组2	在2中优先级最低								 
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

