#include "sys.h"
#include "flash_in.h"
#include "includes.h"
#include "analysis.h"
#include "mc323.h"
#include "adc.h"

volatile u8 temps=0;
volatile u8 SystemDebug=0;
volatile u8 SystemFlow=0;
volatile u8 SyssetCarinfo=0;
_system_setings systemset;

void NVIC_Configuration(void)
{

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	

}

//����������ƫ�Ƶ�ַ
//NVIC_VectTab:��ַ
//Offset:ƫ����			 
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 	   	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//����NVIC��������ƫ�ƼĴ���
	//���ڱ�ʶ����������CODE��������RAM��
}
//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5�� 		   
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������
	temp&=0X0000F8FF; //�����ǰ����
	temp|=0X05FA0000; //д��Կ��
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	    	  				   
}
//����NVIC 
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����	   
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//���÷���
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//ȡ����λ  
	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IP[NVIC_Channel]|=temp<<4;//������Ӧ���ȼ����������ȼ�   	    	  				   
}

//�ⲿ�ж����ú���
//ֻ���GPIOA~G;������PVD,RTC��USB����������
//����:
//GPIOx:0~6,����GPIOA~G
//BITx:��Ҫʹ�ܵ�λ;
//TRIM:����ģʽ,1,������;2,�Ͻ���;3�������ƽ����
//�ú���һ��ֻ������1��IO��,���IO��,���ε���
//�ú������Զ�������Ӧ�ж�,�Լ�������   	    
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM) 
{
	u8 EXTADDR;
	u8 EXTOFFSET;
	EXTADDR=BITx/4;//�õ��жϼĴ�����ı��
	EXTOFFSET=(BITx%4)*4;
						   
	RCC->APB2ENR|=0x01;//ʹ��io����ʱ��

	AFIO->EXTICR[EXTADDR]&=~(0x000F<<EXTOFFSET);//���ԭ�����ã�����
	AFIO->EXTICR[EXTADDR]|=GPIOx<<EXTOFFSET;//EXTI.BITxӳ�䵽GPIOx.BITx
	
	//�Զ�����
	EXTI->IMR|=1<<BITx;//  ����line BITx�ϵ��ж�
	//EXTI->EMR|=1<<BITx;//������line BITx�ϵ��¼� (������������,��Ӳ�����ǿ��Ե�,��������������ʱ���޷������ж�!)
 	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;//line BITx���¼��½��ش���
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;//line BITx���¼��������ش���
} 	  
//����������ִ���������踴λ!�����������𴮿ڲ�����.		    
//������ʱ�ӼĴ�����λ		  
void MYRCC_DeInit(void)
{	
 	RCC->APB1RSTR = 0x00000000;//��λ����			 
	RCC->APB2RSTR = 0x00000000; 
	  
  	RCC->AHBENR = 0x00000014;  //˯��ģʽ�����SRAMʱ��ʹ��.�����ر�.	  
  	RCC->APB2ENR = 0x00000000; //����ʱ�ӹر�.			   
  	RCC->APB1ENR = 0x00000000;   
	RCC->CR |= 0x00000001;     //ʹ���ڲ�����ʱ��HSION	 															 
	RCC->CFGR &= 0xF8FF0000;   //��λSW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //��λHSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //��λHSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //��λPLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR = 0x00000000;     //�ر������ж�		 
	//����������				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x0);
#endif
}
//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//�ر������ж�
__asm void INTX_DISABLE(void)
{
	CPSID I;		  
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	CPSIE I;		  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

//�������ģʽ	  
void Sys_Standby(void)
{
	SCB->SCR|=1<<2;//ʹ��SLEEPDEEPλ (SYS->CTRL)	   
  	RCC->APB1ENR|=1<<28;     //ʹ�ܵ�Դʱ��	    
 	PWR->CSR|=1<<8;          //����WKUP���ڻ���
	PWR->CR|=1<<2;           //���Wake-up ��־
	PWR->CR|=1<<1;           //PDDS��λ		  
	WFI_SET();				 //ִ��WFIָ��		 
}	     
//ϵͳ��λ   
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 		 
//JTAGģʽ����,��������JTAG��ģʽ
//mode:jtag,swdģʽ����;00,ȫʹ��;01,ʹ��SWD;10,ȫ�ر�;	   
//#define JTAG_SWD_DISABLE   0X02
//#define SWD_ENABLE         0X01
//#define JTAG_SWD_ENABLE    0X00		  
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //��������ʱ��	   
	AFIO->MAPR&=0XF8FFFFFF; //���MAPR��[26:24]
	AFIO->MAPR|=temp;       //����jtagģʽ
} 
//ϵͳʱ�ӳ�ʼ������
//pll:ѡ��ı�Ƶ������2��ʼ�����ֵΪ16		 
void Stm32_Clock_Init(u8 PLL)
{
	unsigned char temp=0;   
	MYRCC_DeInit();		  //��λ������������
 	RCC->CR|=0x00010000;  //�ⲿ����ʱ��ʹ��HSEON
	while(!(RCC->CR>>17));//�ȴ��ⲿʱ�Ӿ���
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//����2����λ
	RCC->CFGR|=PLL<<18;   //����PLLֵ 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	FLASH->ACR|=0x32;	  //FLASH 2����ʱ����

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//�ȴ�PLL����
	RCC->CFGR|=0x00000002;//PLL��Ϊϵͳʱ��	 
	while(temp!=0x02)     //�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	{   
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}    
}		    


void mymemset(void *s,u8 c,u32 count)  
{  
    u8 *xs = s;  
    while(count--)*xs++=c;  
}	   


void sysset_read_para(_system_setings * sysset)
{
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)sysset,sizeof(_system_setings));
}

void sysset_save_para(_system_setings * sysset)
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	STMFLASH_Write(FLASH_SAVE_ADDR, (u16*)sysset, sizeof(_system_setings));
	OS_EXIT_CRITICAL();	

}

void SYS_Parameter_Init(void)
{
	u8 i;
	sysset_read_para(&systemset);
	if(systemset.saveflag!=0X0A)
		{
			systemset.HandInter=250;
			systemset.speedlimit=200;
			systemset.delaytime=1;
			systemset.Addrnum=0x01;
			sprintf((char*)systemset.UserName,"KLL_APMODEAAA");
			sprintf((char*)systemset.Passwd,"11111111");
			systemset.saveflag=0x0A;
			sysset_save_para(&systemset);
			printf("This is the first boot.Please set the parameters\r\n");
		}
	else
		{
			printf("KLL_CAR_WIFI_V1.0.0\r\n");
			printf("HandInter:%d\r\n",systemset.HandInter);
			printf("SpedLmt:%d\r\n",systemset.speedlimit);
			printf("Delaytime:%d\r\n",systemset.delaytime);
			printf("Addrnum:%d\r\n",systemset.Addrnum);
			printf("UserName:%s\r\n",systemset.UserName);
			printf("Passwd:%s\r\n",systemset.Passwd);
		}
	
}



void Update_All(void)
{
	
	Update_SysVol();
	if(SW_CH2)my_core_data.frontled=0;
	else my_core_data.frontled=1;
	if(SW_CH1)my_core_data.bhandled=1;
	else my_core_data.bhandled=0;
			
	if(adcvols.PA0VOL<=500)my_core_data.ledstatu=0x00;
	if(adcvols.PA0VOL>=800&&adcvols.PA0VOL<=1200)my_core_data.ledstatu=0x01;
	if(adcvols.PA0VOL>=1800&&adcvols.PA0VOL<=2200)my_core_data.ledstatu=0x02;
	if(adcvols.PA0VOL>=3000)my_core_data.ledstatu=0x03;

#if 0			
	if(adcvols.PA1VOL<=500)my_core_data.jiansepeed=0x00;
	if(adcvols.PA1VOL>=800&&adcvols.PA1VOL<=1200)my_core_data.jiansepeed=0x01;
	if(adcvols.PA1VOL>=1800&&adcvols.PA1VOL<=2200)my_core_data.jiansepeed=0x02;
	if(adcvols.PA1VOL>=3000)my_core_data.jiansepeed=0x03;
			
	if(adcvols.PA2VOL<=500)my_core_data.maxsepeed=0x00;
	if(adcvols.PA2VOL>=800&&adcvols.PA2VOL<=1200)my_core_data.maxsepeed=0x01;
	if(adcvols.PA2VOL>=1800&&adcvols.PA2VOL<=2200)my_core_data.maxsepeed=0x02;
	if(adcvols.PA2VOL>=3000)my_core_data.maxsepeed=0x03;
#endif

	my_core_data.jiansepeed=adcvols.PA1VOL*100/3300;
	my_core_data.maxsepeed=adcvols.PA2VOL*100/3300;
	

	if(adcvols.PA3VOL>1300)my_core_data.speed=(adcvols.PA3VOL-1300)*255/1700;
	else	my_core_data.speed=0;
	
	my_core_data.voice=0x00;
	my_core_data.waningled=0x00;
	if(my_core_data.speed>=temps)
		{
			if(my_core_data.speed>(temps+20))my_core_data.voice=0x02;
		}
	else
		{
			if(my_core_data.speed<(temps+5))
				{
					my_core_data.waningled=0x01;
					my_core_data.voice=0x03;
				}
		}
	temps=my_core_data.speed;
			
	if(adcvols.PA4VOL>3000||adcvols.PA4VOL<400)
		{
			my_core_data.biandao=1;
		}
	
	else	
		{
			my_core_data.biandao=0;
		}
}






















