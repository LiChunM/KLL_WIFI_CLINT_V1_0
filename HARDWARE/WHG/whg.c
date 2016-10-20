#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"    
#include "includes.h"
#include "whg.h"
#include "stm32f10x_iwdg.h"

volatile u8 WORKNORMAL=0;


void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����
	
	IWDG_SetPrescaler(prer);  //����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ64
	
	IWDG_SetReload(rlr);  //����IWDG��װ��ֵ
	
	IWDG_ReloadCounter();  //����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
	
	IWDG_Enable();  //ʹ��IWDG
}



void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload										   
}

