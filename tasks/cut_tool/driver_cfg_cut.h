/**
  ******************************************************************************
  * @file    driver_cfg_cut.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-23
  * @brief  断线工具的驱动，用户需要根据自己的硬件配置修改该文件
  ******************************************************************************
  * @copy
  */

#ifndef  _DRIVER_CFG_CUT_H
#define _DRIVER_CFG_CUT_H

#include "stm32f10x.h"
#include "ucos_ii.h"        
#include "../user_define.h"

//定义手动工具自动工具
//#define TOOL_OPERATE_TYPE	TOOL_TYPE_MANUL 
#define TOOL_OPERATE_TYPE	TOOL_TYPE_AUTO	 

//定义使用的协议版本
//#define USE_PROTOCOL_V2
#define USE_PROTOCOL_V3   //瑞嘉工具

#define USE_DOG
#ifdef	USE_DOG
#include "../drv/drv_dog.h"
#endif

#define USE_FLASH	
#ifdef	USE_FLASH
#include "../drv/drv_flash.h"
#endif

#define USE_BOOT_LOADER
#ifdef	 USE_BOOT_LOADER
#include "../drv/iap.h"				 //boot loader
#endif

//配置串口
#define USE_USART1
//#define USE_USART2
//#define USE_USART3
//#define USE_UART4
//#define USE_UART5

#if (defined USE_USART1) || (defined USE_USART2) || (defined USE_USART3) || (defined USE_UART4) || (defined USE_UART5)
#include "../drv/drv_usart.h"
#endif

//配置ADC，目前只支持ADC1
#define  ADC_CHANNEL_NUM		2				//使用的模拟通道数量
#define ADC1_IN3		0
#define ADC1_IN4		1
#if (defined ADC_CHANNEL_NUM) && (ADC_CHANNEL_NUM > 0)
#include "../drv/drv_adc1.h"
#endif

//配置PWM
//#define USE_TIME1_PWM
//#define ENABLE_TIM1_CH1
//#define ENABLE_TIM1_CH2
//#define ENABLE_TIM1_CH3
//#define ENABLE_TIM1_CH4
//#ifdef USE_TIME1_PWM
//#define		TIME1_PWM_FREQUENCY			20000	//PWM频率20K(不要改)
//#include "../drv/drv_time1_pwm.h"
//#endif
#define GRIP_DIR_DEF -1
#define USE_TIME3_PWM
//#define ENABLE_TIM3_CH1
 #define ENABLE_TIM3_CH2
//#define ENABLE_TIM3_CH3
//#define ENABLE_TIM3_CH4
#ifdef USE_TIME3_PWM
#define		TIME3_PWM_FREQUENCY			330	//PWM频率20K(不要改)
#include "../drv/drv_time3_pwm.h"
#endif

#define DJ_MOTOR_NUM			1
#if (defined DJ_MOTOR_NUM) && (DJ_MOTOR_NUM > 0)
#define DJ_CONTROL_ID				0x200
#include "../drv/dj_motor.h"
#define ID_GRIP_L_MOTOR	    3    //抱紧电机ID
#define GRIP_GEAR_RATIO     18
#define GRIP_MAX_DISTENCE 345 
#define GRIP_SIN_ANGLE 897/1000
#endif


#define SERVO_MOTOR_NUM			1
#if (defined SERVO_MOTOR_NUM) && (SERVO_MOTOR_NUM > 0)
#include "../drv/dj_motor.h"
#endif

#define STEERING_MOTOR_NUM			1
#if (defined STEERING_MOTOR_NUM) && (STEERING_MOTOR_NUM > 0)
#define		STEERING_MOTOR_SPEED_RANGE			30000		//当前舵机的速度范围,针对速度控制舵机
#define    STEERING_MOTOR_ANGLE_RANGE     18000
#include "../drv/steering_motor.h"
#endif






#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
