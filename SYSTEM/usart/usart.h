#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 


#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����


extern u8  USART_RX_BUF[USART_REC_LEN]; 
extern u16 USART_RX_STA;         		

void uart_init(u32 pclk2,u32 bound);
void User_Command_Analysis(u8 *buf);
void Usart1CommandAnalysis(void);
#endif


