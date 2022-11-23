/**
  ******************************************************************************
  * @file    smcl_motor.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-29
  * @brief   力特公司的485接口舵机的驱动程序，支持smcl系列和smbl系列
					2020.5.12  增加smbl系列电机支持，主要是控制模式地址不同
  ******************************************************************************
  * @copy
  */
#ifndef	 _SMCL_MOTOR_H
#define _SMCL_MOTOR_H

#include "stm32f10x.h"
#include "drivers_cfg.h"
#include "ucos_ii.h"   
#include <string.h>

//#define SMCL_MOTOR_NUM			1
// #define SMCL_CONTROL_PORT			UART4
// #define SMCL_BAUDRATE					115200
// #define SMCL_CONTROL_BUF			BUF_UART4

#define SMCL_TASK_STK_SIZE						300
extern OS_STK smcl_protol_task_stk[SMCL_TASK_STK_SIZE]; 

 typedef enum
 {
	 SMCL_MODE_POSITION,
	 SMCL_MODE_VELOCITY,
	 SMCL_MODE_TORQUE
 }enSmclOpMode, SmclOpModedef;

 typedef enum
 {
	SMCL_CMD_PING=1,
	SMCL_CMD_READ,
	SMCL_CMD_WRITE,				//普通写
	SMCL_CMD_RETWRITE,			//异步写
	SMCL_CMD_ACTION,
	SMCL_CMD_RESET,
	SMCL_CMD_SYCNWRITE=0x83	//同步写
 }enSmclCommand,SmclCmdDef;

 typedef struct
 {
	 uint8_t new_flg;		//更新标志
	 int16_t angle;			//当前位置
	 int16_t velocity;					//当前速度
	 int16_t load;						//当前负载
	 uint8_t voltage;					//当前电压
	 uint8_t temprature;
	 uint8_t ref_flg;
	 uint8_t error;						//错误状态
	 uint8_t motion_flg;				//移动标志
	 int16_t target_angle;			//目标角度
	 int16_t current;					//当前电流
 }stSmclState;

 typedef union
 {
	 stSmclState state;
	 uint8_t buf[15];
 }SmclStateDef;


#define  DIR_POSITIVE			1
#define  DIR_NGEATIVE		(-1)

typedef struct  
{
	uint8_t	motor_index;					//电机序号，取值0~SMCL_MOTOR_NUM-1
	uint8_t	com_id;							//电机ID
	int8_t dir_def;							//转向定义
	uint16_t max_pwm;				//最大速度(0.1度每秒)
	uint16_t max_speed;				//最大速度(0.1度每秒)
	int16_t min_angle;						//最小角度限制（0.1度）
	int16_t max_angle;						//最大角度限制（0.1度）
	uint8_t max_temperature;			//最大温度限制（1摄氏度）
	SmclOpModedef mode;				//控制模式
	SmclStateDef state;
}stSmclMotorDef;

//初始化smcl电机
bool Smcl_Motor_Init(stSmclMotorDef* motor);
//转矩模式，直接设置PWM，最大值1000
void Smcl_Motor_SetPwm(stSmclMotorDef*motor, int16_t pwm);
//速度模式 连续旋转模式
//velocity_0_1_dps:旋转速度，符号代表方向，单位：0.1度每秒
void Smcl_Motor_Move(stSmclMotorDef*motor, int16_t velocity_0_1_dps);
//位置模式
//velocity_0_1_dps:最大旋转速度，单位：0.1度每秒
//angle_0_1_d:目标角度，单位：0.1度
void Smcl_Motor_MoveTo(stSmclMotorDef*motor, int16_t velocity_0_1_dps, int16_t angle_0_1_d);
//设置控制模式
//mode:控制模式（0位置控制，1速度控制，2力矩控制）
void Smcl_Motor_SetMode(stSmclMotorDef*motor, SmclOpModedef mode);
//更新电机的状态信息
void Smcl_Motor_UpdateState(stSmclMotorDef*motor);
//smcl电机接收数据解析任务
void Task_SmclProtocol(void *p_arg);
#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
