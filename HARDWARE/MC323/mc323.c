#include "sys.h"
#include "mc323.h"
#include "usart2.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"    
#include "includes.h"
#include "analysis.h"
#include "command.h"
#include "usart3.h"

const u8 *modetbl[2]={"UDP","TCP"};

volatile u8 InterCurLine=0;
volatile u8 IPCONNCET=0;
volatile u8 RtuSendIdentifier=0;
volatile u8 RtuSendBufNUll=0;
volatile u8 RtuSendFinish=0;
volatile u8 DataSensoCheck=0;

const u8* wifiap_ssid="KLL-ESP8266";	
const u8* wifiap_password="12345678"; 

void M35PowerOn(void)
{
	DRV_WIFI_ON;
	DRV_WIFI_CHPD_ONE;
	delay_ms(1500);
	if(SystemDebug==2)printf("M35PowerON\r\n");
}

void M35PowerOff(void)
{
	u8 i=0;
	DRV_WIFI_OFF;
	if(SystemDebug==2)printf("M35PowerOff\r\n");
	for(i=0;i<40;i++)delay_ms(1000);
}


u8* atk_8266_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

void atk_8266_at_response(u8 mode)
{
	if(USART3_RX_STA&0X8000)		
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;
		if(SystemDebug==2)printf("%s\r\n",USART3_RX_BUF);
		if(mode)USART3_RX_STA=0;
	} 
}






u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0;
	u8 buf[100]={0};
	OS_CPU_SR cpu_sr=0;
	USART3_RX_STA=0;
	sprintf((char*)buf,"%s\r\n",cmd);
	if(SystemDebug==2)printf("%s",buf);
	OS_ENTER_CRITICAL();
	USART3_CMD(buf);
	OS_EXIT_CRITICAL();
	if(ack&&waittime)		
	{
		while(--waittime)	
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)
			{
				if(atk_8266_check_cmd(ack))break;
				USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


u8 atk_8266_quit_trans(void)
{
	while((USART3->SR&0X40)==0);	
	USART3->DR='+';      
	delay_ms(15);					
	while((USART3->SR&0X40)==0);	
	USART3->DR='+';      
	delay_ms(15);					
	while((USART3->SR&0X40)==0);	
	USART3->DR='+';      
	delay_ms(500);					
	return atk_8266_send_cmd("AT","OK",20);
}

u8 atk_8266_consta_check(void)
{
	u8 *p;
	u8 res;
	if(atk_8266_quit_trans())return 0;			
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);
	p=atk_8266_check_cmd("+CIPSTATUS:");
	atk_8266_at_response(1);
	res=*p;		
	return res;
}


u8 atk_8266_apsta_check(void)
{
	if(atk_8266_quit_trans())return 0;			
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	
	if(atk_8266_check_cmd("+CIPSTATUS:0")&&
		 atk_8266_check_cmd("+CIPSTATUS:1")&&
		 atk_8266_check_cmd("+CIPSTATUS:2")&&
		 atk_8266_check_cmd("+CIPSTATUS:4"))
		return 0;
	else return 1;
}


void atk_8266_get_wanip(u8* ipbuf)
{
	u8 *p,*p1;
		if(atk_8266_send_cmd("AT+CIFSR","OK",50))
		{
			ipbuf[0]=0;
			return;
		}
		p=atk_8266_check_cmd("\"");
		p1=(u8*)strstr((const char*)(p+1),"\"");
		*p1=0;
		sprintf((char*)ipbuf,"%s",p+1);	
}

void atk_8266_wifiap_init(void)
{
	u8 tcent;
	u8 p[100];
	delay_ms(1000);
	tcent=20;
	while(atk_8266_send_cmd("AT","OK",20))
		{
			tcent--;
			delay_ms(300);
			atk_8266_quit_trans();
			atk_8266_send_cmd("AT+CIPMODE=0","OK",200); 
			atk_8266_at_response(1);
			if(tcent==0)
				{
					tcent=20;
					break;
				}
		}
	atk_8266_at_response(1);
	while(atk_8266_send_cmd("ATE0","OK",20));
	atk_8266_at_response(1);
	atk_8266_send_cmd("AT+CWMODE=2","OK",20);
	atk_8266_at_response(1);
	atk_8266_send_cmd("AT+RST","OK",20);
	atk_8266_at_response(1);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	sprintf((char*)p,"AT+CWSAP=\"%s\",\"%s\",1,4",wifiap_ssid,wifiap_password); 
	atk_8266_send_cmd(p,"OK",1000);
	atk_8266_at_response(1);
}

void atk_8266_wifiap_init2(void)
{
	u8 tcent;
	u8 p[100];
	delay_ms(1000);
	tcent=20;
	while(atk_8266_send_cmd("AT","OK",20))
		{
			tcent--;
			delay_ms(300);
			atk_8266_quit_trans();
			atk_8266_send_cmd("AT+CIPMODE=0","OK",200); 
			atk_8266_at_response(1);
			if(tcent==0)
				{
					tcent=20;
					break;
				}
		}
	atk_8266_at_response(1);
	while(atk_8266_send_cmd("ATE0","OK",20));
	atk_8266_at_response(1);
	atk_8266_send_cmd("AT+CWMODE=2","OK",20);
	atk_8266_at_response(1);
	atk_8266_send_cmd("AT+RST","OK",20);
	atk_8266_at_response(1);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	atk_8266_at_response(1);
	sprintf((char*)p,"AT+CWSAP=\"%s\",\"%s\",11,0",systemset.UserName,systemset.Passwd); 
	atk_8266_send_cmd(p,"OK",1000);
	atk_8266_at_response(1);
}


u8 atk_8266_wifiap_conncet(u8 mode,u8* ipaddr,u8* port)
{
	u8 p[40];
	u8 ipbuf[16];
	delay_ms(10); 
	atk_8266_send_cmd("ATE0","OK",20);
	atk_8266_at_response(1);
	if(mode==0)
		{
			atk_8266_send_cmd("AT+CIPMUX=0","OK",100);
			sprintf((char*)p,"AT+CIPSTART=\"UDP\",\"%s\",%s",ipaddr,(u8*)port); 
			atk_8266_send_cmd(p,"OK",500);
			atk_8266_send_cmd("AT+CIPMODE=1","OK",200);
		}
	if(mode==1)
		{
			atk_8266_send_cmd("AT+CIPMUX=0","OK",20); 
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipaddr,(u8*)port); 
			while(atk_8266_send_cmd(p,"OK",200))
				{
					
				}
			atk_8266_send_cmd("AT+CIPMODE=1","OK",200);
		}
	if(mode==2)
		{
			atk_8266_send_cmd("AT+CIPMUX=1","OK",20);
			atk_8266_at_response(1);
			sprintf((char*)p,"AT+CIPSERVER=1,%s",(u8*)port);
			atk_8266_send_cmd(p,"OK",20);
			atk_8266_at_response(1);
		}
	//atk_8266_get_wanip(ipbuf);
	//printf("ipbuf=%s\r\n",ipbuf);

	return 0;
}

u8 M35SendCmdCheckAck(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u8 buf[100]={0};
	OS_CPU_SR cpu_sr=0;
	sprintf((char*)buf,"%s\r\n",cmd);
	OS_ENTER_CRITICAL();
	USART2_CMD(buf);
	OS_EXIT_CRITICAL();
	if(ack&&waittime)		
	{
		while(--waittime)	
		{
			OSTimeDlyHMSM(0,0,0,10);
			if(RtuSendIdentifier)break;
		}
		if(waittime==0)res=1; 
	}
	RtuSendIdentifier=0;
	return res;
}

u8 M35SendCmdCheckBufAck(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u8 buf[100]={0};
	OS_CPU_SR cpu_sr=0;
	sprintf((char*)buf,"%s\r\n",cmd);
	OS_ENTER_CRITICAL();
	USART2_CMD(buf);
	OS_EXIT_CRITICAL();
	if(ack&&waittime)		
	{
		while(--waittime)	
		{
			OSTimeDlyHMSM(0,0,0,10);
			if(RtuSendBufNUll)break;
		}
		if(waittime==0)res=1; 
		if(RtuSendBufNUll==1)res=0;
		if(RtuSendBufNUll==2)res=2;
	}
	RtuSendBufNUll=0;
	return res;
}


u8 atk_8266_sendData(u8 *data,u16 length,u8 mode)
{
	u8 res=0; 
	u8 p[20];
 	if(mode==0)
 		{
 			//sprintf((char*)p,"AT+CIPSEND=%d",length);
 			//atk_8266_send_cmd(p,"OK",200); 
			//delay_ms(20);
			USART3_DATA(data,length);
 		}
	if(mode==2)
		{
			sprintf((char*)p,"AT+CIPSEND=0,%d",length);
			atk_8266_send_cmd(p,"OK",200); 
			//delay_ms(20);
			USART3_DATA(data,length);
		}
	//res=atk_8266DataCheckOK(400);
	return res;
}

u8 atk_8266DataCheckOK(u16 waittime)
{
	u8 res=0;
	while(--waittime)	
		{
			delay_ms(10);
			if(RtuSendFinish)break;
		}
	RtuSendFinish=0;
	if(waittime==0)res=1;
	return res;
}


void ProcessingTheirReply(u8 *RtuReplyBuf)
{
	u8 res=0;
	u8 *p1,*p2,*p3,*p4;
	p1=(u8*)strstr((const char *)RtuReplyBuf,":");
	if(p1!=NULL)
		{
			if(SystemFlow==10)
				{
					RtuSendBufNUll=1;
					DataSensoCheck=1;
				}

		}
	p2=(u8*)strstr((const char *)RtuReplyBuf,"SEND");
	if(p2!=NULL)
		{
			p3=(u8*)strstr((const char *)RtuReplyBuf,"SEND OK");
			if(p3!=NULL)RtuSendFinish=1;
		}

}



u8 waitforcenterrecall(u16 sdelaytime)
{	
	u8 res=0;
	while(--sdelaytime)	
		{
			delay_ms(10);
			if(DataSensoCheck)break;
		}
	DataSensoCheck=0;
	if(sdelaytime==0)res=1;
	return res;
}

void Get_phone_num(u8 *buf1,u8 *buf2)
{
	mid(buf2,buf1,11,3);
}


void Get_sys_ip(u8 *src,u8 *drc)
{
	u8 *p=NULL;
	p=(u8*)strstr((const char *)src,"!");
	if(p!=NULL)
		{
			while(*src!=0x20)src++;
			src++;
			while(*src!='!')
				{
					*drc=*src;
					src++;
					drc++;
				}
			*drc='\0';
		}
}



void send_mns_sucess(u8 *nums,u8 *data)
{
	
}


void atk_8266_recive_data(u8 *mc35data,u8 len)
{
	ProcessingTheirReply(mc35data);
}

u8 atk_8266ReviceCheckOK(u16 waittime)
{
	u8 res=0;
	while(--waittime)	
		{
			delay_ms(10);
			if(DataSensoCheck)break;
		}
	DataSensoCheck=0;
	if(waittime==0)res=1;
	return res;
}

u8 atk_8266ReviceInitOK(u16 waittime)
{
	u8 res=0;
	while(--waittime)	
		{
			delay_ms(10);
			if(RtuSendBufNUll)break;
		}
	RtuSendBufNUll=0;
	if(waittime==0)res=1;
	return res;
}



u8 Find_Car(void)
{
	u8 t=0;
	u8 res=0,length;
	u8 constate=0;
	u8 stime=0;
	atk_8266_wifiap_init();
	DRV_LEDR_OFF;
	atk_8266_wifiap_conncet(2,NULL,"8086");
ATK_8266_CHECK:
	constate=atk_8266_consta_check();
	if(constate=='+')res=1;
	else
		{
			t++;
			if(t>=20)return 0;
			delay_ms(500);
			if(SystemFlow==1)return 2;
			res=0;
			goto ATK_8266_CHECK;
		}
	t=0;
	res=0;
	constate=0;
	Sync_HandData1(&length);
	SystemFlow=10;
resend1:
	atk_8266_sendData(Hand_Data,length,2);
	res=atk_8266ReviceCheckOK(500);
	if(res==1)
		{
			if(SystemFlow==1)return 2;
			stime++;
			if(stime>=10)return 0;
			else goto resend1;
		}
	SystemFlow=3;
	if(res==0)
		{
			atk_8266_wifiap_init2();
			atk_8266_wifiap_conncet(2,NULL,"8086");
ATK_8266_CHECK2:
			constate=atk_8266_consta_check();
			if(constate=='+')res=1;
			else
			{
				if(SystemFlow==1)return 2;
				delay_ms(500);
				res=0;
				goto ATK_8266_CHECK2;
			}
			res=0;
			constate=0;
			SystemFlow=10;
			res=atk_8266ReviceInitOK(1000);
			if(res)
				{
					if(SystemDebug==2)printf("Can not Recive Car Init Data\r\n");
					return 0;
				}
			Sync_HandData2(&length);
resend2:
			atk_8266_sendData(Hand_Data,length,2);
			res=atk_8266ReviceCheckOK(500);
			if(res==1)
				{
					if(SystemFlow==1)return 2;
					stime++;
					if(stime>=10)return 0;
					else goto resend2;
				}
			SystemFlow=3;
		}
	return 1;
}

u8 Check_Car(void)
{
	u8 t=0;
	u8 res=0,length;
	u8 constate=0;
	u8 stime=0;
	atk_8266_wifiap_init2();
	atk_8266_wifiap_conncet(0,"255.255.255.255","6060,6060,0");
ATK_8266_CHECK3:
	t=0;
	constate=atk_8266_consta_check();
	if(constate=='+')res=1;
	else
		{
			while(t<50)
				{
					t++;
					delay_ms(10);
					if(SW_CH3==0)return 3;
				}
			if(SystemFlow==0)goto ATK_8266_CHECK3;
		}
	atk_8266_send_cmd("AT+CIPSEND","OK",200);
	//SystemFlow=10;
	//res=atk_8266ReviceInitOK(1000);
	//if(res)
	//{
		//if(SystemDebug==2)printf("Can not Recive Car Init Data\r\n");
	//	return 1;
	//}
	//Sync_HandData2(&length);
//resend3:
	//atk_8266_sendData(Hand_Data,length,2);
	//atk_8266ReviceCheckOK(500);
	//if(res==1)
		//{
			//if(SystemFlow==1)return 2;
		//	stime++;
		//	if(stime>=10)return 1;
		//	else goto resend3;
	//	}
	//SystemFlow=0;
	return 0;
}
