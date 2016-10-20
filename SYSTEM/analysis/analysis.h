#ifndef _ANALYSIS_H_
#define _ANALYSIS_H_
#include "sys.h"

#define isPrime(year) ((year%4==0&&year%100!=0)||(year%400==0))


typedef struct 
{
	u8 speed;
	u8 fangxiang;
	u8 frontled;     //弯道减速开关
	u8 bhandled;    //正反运行
	u8 waningled;	//刹车灯
	u8 maxsepeed;
	u8 jiansepeed;
	u8 voice;
	u8 biandao;
	u8 ledstatu;
	u8 back[6];
}Core_data;


extern Core_data my_core_data;

extern u8 Hand_Data[60];

typedef struct 
{
	u8 MyYear;
	u8 MyMonth;
	u8 MyData;
}Mydatastrcut;

extern volatile u8 DataBiteInfo;


#endif
