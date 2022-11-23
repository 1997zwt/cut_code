/**
  ******************************************************************************
  * @file    PID.h
  * @author  lyq
  * @version V1.0.0
  * @date    2016-6-17
  * @brief   PID算法头文件，提供两种PID算法：
					1)积分分离的位置式PID算法
					2)增量式PID算法
******************************************************************************
* @copy
*/
#include "stm32f10x.h"

#ifndef _PID_H
#define _PID_H

//定义PID参数设定结构体，float类型
typedef struct PID_Struct_float
{
	float Kp;
	float Ki;
	float Kd;

	float ts;		//采样周期
	float errlim;	//积分分离阈值
	float ei;			//误差积分
	float pre_err;//上一次误差
	float pen_err;//倒数第二次误差
}fstPID;

//定义PID参数设定结构体，int类型
typedef struct PID_Struct_int
{
	int Kp;
	int Ki;
	int Kd;
	int ts;		//采样周期
	int errlim;	//积分分离阈值
	int ei;			//误差积分
	int pre_err;//上一次误差
	int pen_err;//倒数第二次误差
}istPID;

typedef union PID_Union
{
	fstPID fpid;
	istPID ipid;
	uint8_t hex[20];
}unPID;

void InitPID(istPID* pid);
float fPosPIDCal(fstPID pid,float error);
float fIncPIDCal(fstPID pid,float error);

int iIncPIDCal(istPID pid,int error);
int iPosPIDCal(istPID* pid,int error);
#endif


//===========================================================================
// No more.
//===========================================================================
