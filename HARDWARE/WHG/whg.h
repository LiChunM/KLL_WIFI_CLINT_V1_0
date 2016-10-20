#ifndef _WHG_H_
#define _WHG_H_

#include "sys.h"

extern volatile u8 WORKNORMAL;

void IWDG_Init(u8 prer,u16 rlr);

void IWDG_Feed(void);



#endif
