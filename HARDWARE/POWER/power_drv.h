#ifndef _POWER_DRV_H_
#define _POWER_DRV_H_
#include "sys.h"

#define DRV_WIFI_ON	GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define DRV_WIFI_OFF	GPIO_ResetBits(GPIOA,GPIO_Pin_8)

#define DRV_LEDL_OFF		GPIO_SetBits(GPIOB,GPIO_Pin_15)
#define DRV_LEDL_ON		GPIO_ResetBits(GPIOB,GPIO_Pin_15)

#define DRV_LEDR_OFF		GPIO_SetBits(GPIOB,GPIO_Pin_14)
#define DRV_LEDR_ON		GPIO_ResetBits(GPIOB,GPIO_Pin_14)


#define DRV_WIFI_RST_ONE		GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define DRV_WIFI_RST_ZERO	GPIO_ResetBits(GPIOB,GPIO_Pin_12)

#define DRV_WIFI_CHPD_ONE		GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define DRV_WIFI_CHPD_ZERO	GPIO_ResetBits(GPIOB,GPIO_Pin_13)


#define  SW_CH3 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_14)
#define  SW_CH1 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)

#define  SW_IN 	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)
#define  SW_CH2 	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)




extern u8 Config_Byte;
void power_drv_init(void);
void ACC_Init(void);


#endif

