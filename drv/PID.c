//============================================================================//
//							    	PID.c									  //
//============================================================================//
//PID算法源文件，提供两种PID算法：
//	1)积分分离的位置式PID算法
//	2)增量式PID算法
//============================================================================//

#include "PID.h"

#define ABS(x) ((x)<0 ? -1*(x) : (x))			// 取数据的绝对值

//初始化PID参数
void InitPID(istPID* pid)
{
	pid->Kp = 0;
	pid->Ki = 0;
	pid->Kd = 0;
	pid->ts = 1;
	pid->errlim = 0;
	pid->ei = 0;
	pid->pre_err = 0;
	pid->pen_err = 0;
}
/*******************************************
//u(k) = Kp*error(k) 
//     + β*Ki*[error(0) + ... + error(k)]*T
//     + Kd*[error(k) - error(k-1)]/T
********************************************/
//积分分离的位置式PID算法(integration separation PID Controller)
//输入：PID参数，当前偏差
//输出：PID控制算法结果
//说明：1)需根据实际计算出实际偏差作为输入
//		2)输出无限幅，需根据实际对结果进行限幅
float fPosPIDCal(fstPID pid,float error)
{
	float beta = 0;			//积分使能
	float Uo = 0;			//输出

	//积分分离
	if (ABS(error) <= pid.errlim)
	{
		beta = 1;
		pid.ei += pid.Ki*error;
	} 
	else
	{
		beta = 0;
		pid.ei = 0;
	}
	Uo = pid.Kp*error + pid.ei*beta + pid.Kd*(error - pid.pre_err);
	pid.pre_err = error;
	
	return Uo;
}

/***********************************************
//△u(k) = Kp*[error(k)-error(k-1)]
//       + Ki*error(k) 
//       + Kd*(error(k)-2*error(k-1)+error(k-2))
***********************************************/
//增量式PID算法(Increment PID Controller)
float fIncPIDCal(fstPID pid,float error)
{
	float Uo = 0;			//输出

	Uo = pid.Kp*(error - pid.pre_err) + pid.Ki*error + pid.Kd*(error - 2*pid.pre_err + pid.pen_err);

	pid.pen_err = pid.pre_err;
	pid.pre_err = error;

	return Uo;
}

//位置式PID算法，整形运算
int iPosPIDCal(istPID* pid,int error)
{
	int Uo = 0;			//输出
	int beta = 1;			//积分使能

	//积分分离
 	if (ABS(error) <= pid->errlim)
 	{
 		beta = 1;
 		pid->ei += pid->Ki*error;
 	} 
 	else
 	{
 		beta = 0;
 		pid->ei = 0;
 	}

	Uo = pid->Kp*error + pid->ei*beta + pid->Kd*(error - pid->pre_err);
	pid->pre_err = error;

	return Uo;
}

//增量式PID算法(Increment PID Controller)，整形运算
int iIncPIDCal(istPID pid,int error)
{
	int Uo = 0;			//输出

	Uo = pid.Kp*(error - pid.pre_err) + pid.Ki*error + pid.Kd*(error - 2*pid.pre_err + pid.pen_err);
	pid.pen_err = pid.pre_err;
	pid.pre_err = error;

	return Uo;
}


