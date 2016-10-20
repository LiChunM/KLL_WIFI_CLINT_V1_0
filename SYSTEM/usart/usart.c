#include "sys.h"
#include "usart.h"	  
#include "ucos_ii.h"
#include "delay.h"
#include "analysis.h"
#include "mc323.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif
#if 1

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

u16 USART_RX_STA=0;       //����״̬���	  
  
void USART1_IRQHandler(void)
{
	u8 res;	
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART1->SR&(1<<5))//���յ�����
	{	 
		res=USART1->DR; 
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}else //��û�յ�0X0D
			{	
				if(res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	}
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntExit();  											 
#endif
} 
#endif										 
//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������
//CHECK OK
//091209
void uart_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH&=0XFFFFF00F;//IO״̬����
	GPIOA->CRH|=0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ	   	   
	//����������
 	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.
#if EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<8;    //PE�ж�ʹ��
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//��2��������ȼ� 
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

