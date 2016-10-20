#include "sys.h"
#include "usart.h"	  
#include "ucos_ii.h"
#include "delay.h"
#include "analysis.h"
#include "mc323.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif
#if 1

#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

u16 USART_RX_STA=0;       //接收状态标记	  
  
void USART1_IRQHandler(void)
{
	u8 res;	
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntEnter();    
#endif
	if(USART1->SR&(1<<5))//接收到数据
	{	 
		res=USART1->DR; 
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}else //还没收到0X0D
			{	
				if(res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	}
#ifdef OS_CRITICAL_METHOD 	//如果OS_CRITICAL_METHOD定义了,说明使用ucosII了.
	OSIntExit();  											 
#endif
} 
#endif										 
//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率
//CHECK OK
//091209
void uart_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟  
	RCC->APB2ENR|=1<<14;  //使能串口时钟 
	GPIOA->CRH&=0XFFFFF00F;//IO状态设置
	GPIOA->CRH|=0X000008B0;//IO状态设置
		  
	RCC->APB2RSTR|=1<<14;   //复位串口1
	RCC->APB2RSTR&=~(1<<14);//停止复位	   	   
	//波特率设置
 	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C;  //1位停止,无校验位.
#if EN_USART1_RX		  //如果使能了接收
	//使能接收中断
	USART1->CR1|=1<<8;    //PE中断使能
	USART1->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//组2，最低优先级 
#endif
}


void Get_Str_Use(u8 *ipstr,u8 *buf)
{
	memset(ipstr,'\0',sizeof(ipstr));
	while(*buf!=0x20)buf++;
	buf++;
	strcpy((char*)ipstr,(char*)buf);
}

u8 Get_Str2_Use(u8 *ipstr1,u8 *ipstr2,u8*buf)
{
	memset(ipstr1,'\0',sizeof(ipstr1));
	memset(ipstr2,'\0',sizeof(ipstr2));
	while(*buf!=0x20)buf++;
	buf++;
	while(*buf!=0x20)
		{
			*ipstr1=*buf;
			buf++;
			ipstr1++;
		}
	buf++;
	strcpy((char*)ipstr2,(char*)buf);
	return 1;
}




void Usart1CommandAnalysis(void)
{
	u8 res=0,t=0;
	u8 USART_TX_BUF[30]={0};
	if(USART_RX_STA&0x8000)	
	{
		res=USART_RX_STA&0x3FFF;
		if(res<=30)
			{
					for(t=0;t<res;t++)
					{
						USART_TX_BUF[t]=USART_RX_BUF[t];
						while((USART1->SR&0X40)==0);
					}
			}
	      USART_RX_STA=0;
	      USART_TX_BUF[t]=0;
	      User_Command_Analysis((u8*)USART_TX_BUF);					   
	}
}


void User_Command_Analysis(u8 *buf)
{
	u8 mybuf[5];
	u8 pataner[5];
	u8 *p1;
	delay_ms(10);
	p1=(u8*)strstr((const char*)buf,"$setaddr");
	if(p1!=NULL)
		{
			  mymemset(mybuf,0,sizeof(mybuf));
			  Get_Str_Use(mybuf,p1);
			  systemset.Addrnum=strtol((const char*)mybuf,NULL,10);
			  sysset_save_para(&systemset);
			  printf("+Addrnum %02d\r\n",systemset.Addrnum);

		}
	p1=(u8*)strstr((const char*)buf,"$setuptime");
	if(p1!=NULL)
		{
			  mymemset(mybuf,0,sizeof(mybuf));
			  Get_Str_Use(mybuf,p1);
			  systemset.HandInter=strtol((const char*)mybuf,NULL,10);
			  sysset_save_para(&systemset);
			  printf("+HandInter %02d\r\n",systemset.HandInter);

		}
	p1=(u8*)strstr((const char*)buf,"$setupspeed");
	if(p1!=NULL)
		{
			  mymemset(mybuf,0,sizeof(mybuf));
			  Get_Str_Use(mybuf,p1);
			  systemset.speedlimit=strtol((const char*)mybuf,NULL,10);
			  sysset_save_para(&systemset);
			  printf("+speedlimit %02d\r\n",systemset.speedlimit);

		}
	p1=(u8*)strstr((const char*)buf,"$setdelaytime");
	if(p1!=NULL)
		{
			  mymemset(mybuf,0,sizeof(mybuf));
			  Get_Str_Use(mybuf,p1);
			  systemset.delaytime=strtol((const char*)mybuf,NULL,10);
			  sysset_save_para(&systemset);
			  printf("+delaytime %02d\r\n",systemset.delaytime);

		}
	p1=(u8*)strstr((const char*)buf,"$setusername");
	if(p1!=NULL)
		{
			mymemset(systemset.UserName,0,sizeof(systemset.UserName));
			Get_Str_Use(systemset.UserName,p1);
			sysset_save_para(&systemset);
			printf("+UserName %s\r\n",systemset.UserName);
		}
	p1=(u8*)strstr((const char*)buf,"$setpawssd");
	if(p1!=NULL)
		{
			mymemset(systemset.Passwd,0,sizeof(systemset.Passwd));
			Get_Str_Use(systemset.Passwd,p1);
			sysset_save_para(&systemset);
			printf("+Pawssd %s\r\n",systemset.Passwd);
		}
	p1=(u8*)strstr((const char*)buf,"$setinfo");
	if(p1!=NULL)
		{
			printf("KLL_HAND_WIFI_V1.0.0\r\n");
			printf("HandInter:%d\r\n",systemset.HandInter);
			printf("SpedLmt:%d\r\n",systemset.speedlimit);
			printf("Delaytime:%d\r\n",systemset.delaytime);
			printf("Addrnum:%d\r\n",systemset.Addrnum);
			printf("UserName:%s\r\n",systemset.UserName);
			printf("Passwd:%s\r\n",systemset.Passwd);
		}
	p1=(u8*)strstr((const char*)buf,"$$debug 2");
	if(p1!=NULL)
		{
			SystemDebug=2;
			printf("+debug 2\r\n");
		}
	p1=(u8*)strstr((const char*)buf,"$$debug 0");
	if(p1!=NULL)
		{
			SystemDebug=0;
			printf("+debug 0\r\n");
		}
	p1=(u8*)strstr((const char*)buf,"$$debug 3");
	if(p1!=NULL)
		{
			SystemDebug=3;
			printf("+debug 3\r\n");
		}
}

