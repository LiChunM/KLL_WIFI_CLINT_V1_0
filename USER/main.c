#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "includes.h"  
#include "flash_in.h"
#include "analysis.h"
#include "stm32f10x_adc.h"
#include "analysis.h"
#include "adc.h"


//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//USART1����
//�����������ȼ�
#define USART1_TASK_PRIO       			3 
//���������ջ��С
#define USART1_STK_SIZE  		    		512
//�����ջ
OS_STK USART1_TASK_STK[USART1_STK_SIZE];
//�����������û�����ʱ������
void usart1_task(void *pdata);

//USART2����
//�����������ȼ�
#define USART2_TASK_PRIO       		 	6
//���������ջ��С
#define USART2_STK_SIZE  				512
//�����ջ	
OS_STK USART2_TASK_STK[USART2_STK_SIZE];
//��������������CDMA����
void usart2_task(void *pdata);

//UART5����
//�����������ȼ�
#define UART5_TASK_PRIO    			5 
//���������ջ��С
#define UART5_STK_SIZE  		 		256
//�����ջ	
OS_STK UART5_TASK_STK[UART5_STK_SIZE];
//��������������GPS����
void uart5_task(void *pdata);


//������
//�����������ȼ�
#define MAIN_TASK_PRIO       			8
//���������ջ��С
#define MAIN_STK_SIZE  					512
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);


//Ԥ������
//�����������ȼ�
#define XXX_TASK_PRIO       			2 
//���������ջ��С
#define XXX_STK_SIZE  					64
//�����ջ	
OS_STK XXX_TASK_STK[XXX_STK_SIZE];
//������ʲôҲ����
void xxx_task(void *pdata);

//HAND ����
//�����������ȼ�
#define HAND_TASK_PRIO      			7 //���������������
//���������ջ��С
#define HAND_STK_SIZE  				128
//�����ջ	
OS_STK HAND_TASK_STK[HAND_STK_SIZE];
//������
void hand_task(void *pdata);	
 			   
//323����
//�����������ȼ�
#define  MC323_TASK_PRIO       			4
//���������ջ��С
#define MC323_STK_SIZE  					256
//�����ջ	
OS_STK MC323_TASK_STK[MC323_STK_SIZE];
//������ʲôҲ����
void MC323_task(void *pdata);

 			   
//323����
//�����������ȼ�
#define  MC323_reconnection_TASK_PRIO       			9
//���������ջ��С
#define MC323_reconnection_STK_SIZE  					128
//�����ջ	
OS_STK MC323_reconnection_TASK_STK[MC323_reconnection_STK_SIZE];
//������ʲôҲ����
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
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}							    
//��ʼ����
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
//usart1����

void usart1_task(void *pdata)
{
	u8 res,t;
	pdata = pdata;
	while(1)
		{
			delay_ms(10);
			Usart1CommandAnalysis();
			if(SystemFlow==2||SystemFlow==10)Usart3CommandAnalysis();
		}
}
//usart2����
void usart2_task(void *pdata)
{	
	u8 init=0;
	pdata = pdata;
	while(1)
	{
		delay_ms(10);
	}
}     
//gps����
void uart5_task(void *pdata)
{
	u16 time=0;
	pdata = pdata;
	while(1)
	{
		delay_ms(10);
		if(SW_CH3==0)
			{
				while(SW_CH3==0)
					{
						time++;
						delay_ms(100);
						if(time>20)DRV_LEDR_ON;
					}
				if(time>20)
					{
						time=0;
						while(SW_CH3)
							{
								time++;
								delay_ms(10);
								if(time>=500)break;
								
							}
						if(time<500)
							{
								time=0;
								while(SW_CH3==0)
								{
									delay_ms(10);
								}
								while(SW_CH3)
								{
									time++;
									delay_ms(10);
									if(time>=500)break;
									
								}
								if(time<500)
									{
										time=0;
										while(SW_CH3==0)
										{
											delay_ms(10);
										}
										while(SW_CH3)
										{
											time++;
											delay_ms(10);
											if(time>=500)break;
											
										}
										if(time<500)
											{
												time=0;
												while(SW_CH3==0)
												{
													delay_ms(10);
												}
												SystemFlow=1;
											}
									}
								
							}
						
					}
			}
		time=0;
		DRV_LEDR_OFF;
	}									 
}
//������
void main_task(void *pdata)
{	
	u8 res;
	u8 lens=0;
	M35PowerOn();
 	while(1)
	{
		delay_ms(10);
		if(SystemFlow==1)
			{	
				SystemFlow=3;
				res=Find_Car();
				if(res==2)SystemFlow=1;
				if(res==1)SystemFlow=2;
				if(res==0)SystemFlow=0;
			}
		if(SystemFlow==0)
			{
				res=Check_Car();
				if(res==0)
					{
						if(SystemFlow==0)SystemFlow=2;
					}
				if(res==1)SystemFlow=0;
				if(res==2)SystemFlow=1;
				if(res==3)SystemFlow=3;
			}
		if(SystemFlow==2)
			{
				Update_All();
				delay_ms(50);
				Command_HandData2(&lens);
				atk_8266_sendData(Hand_Data,lens,0);
				if(my_core_data.biandao==1&&SyssetCarinfo==0)
					{
						ComRoad_HandData(&lens,1);
						atk_8266_sendData(Hand_Data,lens,0);
						SyssetCarinfo=1;
					}
				if(my_core_data.biandao==0&&SyssetCarinfo==1)
					{
						ComRoad_HandData(&lens,0);
						atk_8266_sendData(Hand_Data,lens,0);
						SyssetCarinfo=0;
					}
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
   		    
//��������
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





