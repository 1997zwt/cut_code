/**
  ******************************************************************************
  * @file    steering_motor.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-05-12
  * @brief   航模舵机驱动程序，
				（1）转角舵机控制频率为330Hz，采用占空比控制舵机转角。
				舵机转角范围为0~270度，对应周期0ms~3ms，即1ms脉宽对应90度
				（2）转速舵机控制频率为330Hz，采用占空比控制舵机速度
				周期范围1ms~2ms，对应占空比33.3%~66.7%
  ******************************************************************************
  * @copy
  */
#ifndef	 _STEERING_MOTOR_H
#define _STEERING_MOTOR_H

#include "stm32f10x.h"
#include "drivers_cfg.h"

typedef enum
{
	STEERING_MOTOR_TYPE_SPEED,
	STEERING_MOTOR_TYPE_ANGLE
}enSteeringMotorType,SteeringMotorTypeDef;

typedef struct  
{
	uint8_t	index;					//电机序号，取值0~STEERING_MOTOR_NUM-1
	SteeringMotorTypeDef type;
	int16_t min;						//最小角度限制（度）或最小速度限制
	int16_t max;						//最大角度限制（度）或最大速度限制
	int16_t offset;					//角度偏移，即用户0度对应的舵机角度，用于重定义舵机转角范围；或速度偏移，速度0对应的PWM
	int16_t origin;					//初始角度（度）或初始速度
	int16_t current;				//当前角度（度）或当前速度
	void (*SetPwmFun)(uint16_t);		//设置PWM的函数指针
}stSteeringMotorDef;

//初始化舵机
bool Steering_Motor_Init(stSteeringMotorDef* motor);
//舵机速度控制
//speed_1_dps：目标速度，单位：度每秒
void Steering_Motor_Move(stSteeringMotorDef* motor, int16_t speed_1_dps);
//舵机角度控制
//angle_1_d：目标角度，单位：度
void Steering_Motor_MoveTo(stSteeringMotorDef* motor, int16_t angle_1_d);
#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
