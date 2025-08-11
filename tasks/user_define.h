/**
  ******************************************************************************
  * @file    user_define.h
  * @author  lyq
  * @version V1.0.0
  * @date    creat:2016-1-29 / modify:2020-4-23 
  * @brief   该文件定义了程序中常用的数据类型以及函数等（官方库不包含的部分）
  ******************************************************************************
  * @copy
  */
#ifndef   _USER_DEFINE_H
#define  _USER_DEFINE_H

#include "stm32f10x.h"

typedef  unsigned char		 bool;   

typedef union 
{
	float			F_32;
	uint32_t		U_32;
	int32_t		I_32;
	uint8_t	   BYTE[4];
	uint16_t	  WORD[2];
}un_data32;


#ifndef NULL
#define NULL                          (void *)0
#define null								NULL
#endif

#ifndef TRUE
#define FALSE                         0
#define TRUE                          (!FALSE)
#define true								TRUE
#define false							FALSE
#endif

#ifndef PI
#define PI						3.1415926
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef SIG
#define SIG(x) ((x)>=0 ? 1 : -1)					// 数据非负判别
#endif

#ifndef SYM
#define SYM(x) ((x)!=0 ? -1 : 1)					// 符号位判别
#endif

#ifndef ABS
#define ABS(x) ((x)<0 ? -1*(x) : (x))			// 取数据的绝对值
#endif

#ifndef STB
#define STB(x,y) (x |= (1<<y))                 // 寄存器相应位置1
#endif

#ifndef CLB
#define CLB(x,y) (x &= ~(1<<y))				// 寄存器相应位置0
#endif

#ifndef TSB
#define TSB(x,y) (x & (1<<y))					// 测试对应位是否1
#endif

#ifndef FPB
#define FPB(x,y) (x ^= (1<<y))					// 对应位状态翻转
#endif


#ifndef DeadZone
#define DeadZone(x,y)  (x)>(y)?((x) -= (y)):((x)<(0-(y))?((x) += (y)):((x) = 0))					//设置死区，使:-y<x<y时，x=0
#endif

#ifndef RLimit
#define RLimit(x,y)  	 (x)>(y)?((x) = (y)):((x)>0?((x) = (x)):((x) =  0))								//右侧限幅，使:0<x<y
#endif

#ifndef LLimit
#define LLimit(x,y)  	 (x)>0?((x) = 0):((x)<(y))?((x) =  (y)):((x) = (x))								//左侧限幅，使:y<x<0
#endif

#ifndef Limit
#define Limit(x,y)  		(x)>(y)?((x) = (y)):((x)<(0-(y))?((x) =  (-y)):((x) = (x)))					//上下限幅，使:-y<x<y
#endif

#ifndef Range
#define Range(x,a,b)		(x) < (a)?((x) = (a)):(((x)>(b))?((x)=(b)):((x) = (x)))							//设置范围，使:a<x<b
#endif

#ifndef BYTESET
#define BYTESET
#define BYTE0(x)		(unsigned char)(x)							//获取第1个字节
#define BYTE1(x)		(unsigned char)((x)>>8)				//获取第2个字节
#define BYTE2(x)		(unsigned char)((x)>>16)				//获取第3个字节
#define BYTE3(x)		(unsigned char)((x)>>24)				//获取第4个字节

#endif

#define SYSCLK_FREQ  72000000
#define TIMEOUT_1US	 (SYSCLK_FREQ/1000000)
#define TIMEOUT_1MS	 (SYSCLK_FREQ/1000)

#endif

