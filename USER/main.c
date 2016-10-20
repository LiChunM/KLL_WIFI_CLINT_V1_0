#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "includes.h"  
#include "flash_in.h"
#include "analysis.h"
#include "stm32f10x_adc.h"
#include "analysis.h"
#include "adc.h"


//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//USART1任务
//设置任务优先级
#define USART1_TASK_PRIO       			3 
//设置任务堆栈大小
#define USART1_STK_SIZE  		    		512
//任务堆栈
OS_STK USART1_TASK_STK[USART1_STK_SIZE];
//任务函数处理用户的随时的输入
void usart1_task(void *pdata);

//USART2任务
//设置任务优先级
#define USART2_TASK_PRIO       		 	6
//设置任务堆栈大小
#define USART2_STK_SIZE  				512
//任务堆栈	
OS_STK USART2_TASK_STK[USART2_STK_SIZE];
//任务函数用来处理CDMA数据
void usart2_task(void *pdata);

//UART5任务
//设置任务优先级
#define UART5_TASK_PRIO    			5 
//设置任务堆栈大小
#define UART5_STK_SIZE  		 		256
//任务堆栈	
OS_STK UART5_TASK_STK[UART5_STK_SIZE];
//任务函数用来接受GPS数据
void uart5_task(void *pdata);


//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			8
//设置任务堆栈大小
#define MAIN_STK_SIZE  					512
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);


//预留任务
//设置任务优先级
#define XXX_TASK_PRIO       			2 
//设置任务堆栈大小
#define XXX_STK_SIZE  					64
//任务堆栈	
OS_STK XXX_TASK_STK[XXX_STK_SIZE];
//任务函数什么也不做
void xxx_task(void *pdata);

//HAND 任务
//设置任务优先级
#define HAND_TASK_PRIO      			7 //进行握手任务操作
//设置任务堆栈大小
#define HAND_STK_SIZE  				128
//任务堆栈	
OS_STK HAND_TASK_STK[HAND_STK_SIZE];
//任务函数
void hand_task(void *pdata);	
 			   
//323任务
//设置任务优先级
#define  MC323_TASK_PRIO       			4
//设置任务堆栈大小
#define MC323_STK_SIZE  					256
//任务堆栈	
OS_STK MC323_TASK_STK[MC323_STK_SIZE];
//任务函数什么也不做
void MC323_task(void *pdata);

 			   
//323任务
//设置任务优先级
#define  MC323_reconnection_TASK_PRIO       			9
//设置任务堆栈大小
#define MC323_reconnection_STK_SIZE  					128
//任务堆栈	
OS_STK MC323_reconnection_TASK_STK[MC323_reconnection_STK_SIZE];
//任务函数什么也不做
void MC323_reconnection_task(void *pdata);




 int main(void)
 {	 
 	JTAG_Set(0x02);
  	power_drv_init();
	ACC_Init();
	DRV_LEDR_OFF;
 	delay_init();	    	
	NVIC_Configuration();
	uart_init(72,9600);
	USART3_Init(115200);
	Adc_Init();
	IWDG_Init(4,625);
	SYS_Parameter_Init();
	OSInit();
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}							    
//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 	
	OSStatInit();					
 	OS_ENTER_CRITICAL();			    
 	OSTaskCreate(usart1_task,(void *)0,(OS_STK*)&USART1_TASK_STK[USART1_STK_SIZE-1],USART1_TASK_PRIO);						   
 	OSTaskCreate(usart2_task,(void *)0,(OS_STK*)&USART2_TASK_STK[USART2_STK_SIZE-1],USART2_TASK_PRIO);	 				   
 	OSTaskCreate(uart5_task,(void *)0,(OS_STK*)&UART5_TASK_STK[UART5_STK_SIZE-1],UART5_TASK_PRIO);
	OSTaskCreate(hand_task,(void *)0,(OS_STK*)&HAND_TASK_STK[HAND_STK_SIZE-1],HAND_TASK_PRIO);	 				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	
	OSTaskCreate(MC323_task,(void *)0,(OS_STK*)&MC323_TASK_STK[MC323_STK_SIZE-1],MC323_TASK_PRIO);	 				   
 	OSTaskCreate(xxx_task,(void *)0,(OS_STK*)&XXX_TASK_STK[XXX_STK_SIZE-1],XXX_TASK_PRIO);
	OSTaskCreate(MC323_reconnection_task,(void *)0,(OS_STK*)&MC323_reconnection_TASK_STK[MC323_reconnection_STK_SIZE-1],MC323_reconnection_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);
	OS_EXIT_CRITICAL();
}
//usart1任务

void usart1_task(void *pdata)
{
	u8 res,t;
	pdata = pdata;
	while(1)
		{
			delay_ms(10);
			Usart1CommandAnalysis();
		}
}
//usart2任务
void usart2_task(void *pdata)
{	
	u8 init=0;
	pdata = pdata;
	while(1)
	{
		delay_ms(10);
	}
}     
//gps任务
void uart5_task(void *pdata)
{
	pdata = pdata;
	while(1)
	{
		delay_ms(10);
	}									 
}
//主任务
void main_task(void *pdata)
{	
	u8 res;
	u8 lens=0;
	M35PowerOn();
 	while(1)
	{
		delay_ms(10);
		if(SystemFlow==0)
			{	
				Conecet2TheHandFromUdp();
				SystemFlow=1;
			}
		if(SystemFlow==1)
			{
				Usart3Command2Hex();
			}
		
	}
}	

void hand_task(void *pdata)
{
	u8 i;
	pdata=pdata;
	while(1)
		{
			delay_ms(100);	
		}
}
void MC323_task(void *pdata)
{
	
	pdata=pdata;
	while(1)
		{
			delay_ms(100);
			
			
		}
}
   		    
//监视任务
void xxx_task(void *pdata)
{	
	u8 i;
	pdata=pdata;
	while(1)
	{
		delay_ms(10);
		DRV_LEDL_ON;
		delay_ms(200);
		DRV_LEDL_OFF;
		for(i=0;i<20;i++)delay_ms(500);
		
	}
}

void MC323_reconnection_task(void *pdata)
{
	pdata=pdata;
	while(1)
		{
			delay_ms(20);
			IWDG_Feed();
		}
	
}


void HardFault_Handler(void)
{
	u32 temp;
	temp=SCB->CFSR;					
 	printf("CFSR:%8X\r\n",temp);	
	temp=SCB->HFSR;					
 	printf("HFSR:%8X\r\n",temp);	
 	temp=SCB->DFSR;					
 	printf("DFSR:%8X\r\n",temp);	
   	temp=SCB->AFSR;					
 	printf("AFSR:%8X\r\n",temp);
}





