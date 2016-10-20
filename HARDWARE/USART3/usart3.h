#ifndef __USART3_H
#define __USART3_H	 
#include "sys.h"  


#define USART3_MAX_RECV_LEN  800
#define USART3_RX_EN		1

extern u16 USART3_RX_STA;
extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN];
void USART3_Init(u32 bound);
void TIM2_Set(u8 sta);
void TIM2_Init(u16 arr,u16 psc);
void Usart3CommandAnalysis(void);
void USART3_CMD(unsigned char *lb);
void USART3_DATA(unsigned char *lb,unsigned int len);
#endif
