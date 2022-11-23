/**
  ******************************************************************************
  * @file    pro_strip_tool.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-29
  * @brief   剥线工具协议
  ******************************************************************************
  * @copy
  */
#ifndef		_PRO_STRIP_TOOL_H
#define 	_PRO_STRIP_TOOL_H

#include "stm32f10x.h"

#define		PRO_HEAD0	0xAA
#define		PRO_HEAD1 0x55

typedef enum
{
	STRIP_TOOL,
	MOTOR_GRIP_L   = 0x83,		//左边抱紧
	MOTOR_GRIP_R  = 0x88,		//右边抱紧
	MOTOR_SLIDER  = 0x80,		//滑台电机
	MOTOR_CUTTER = 0x82,	//进刀电机
	MOTOR_STRIP		= 0x81,	//剥线电机
	MOTOR_STRIP_HOME  = 0x84,	//剥线复位
	MOTOR_STRIP_START = 0x85,	//剥线开始
	MOTOR_ALL_STOP = 0x86,		//一键停止(剥线、进刀、滑台)
	MOTOR_ALL_HOME = 0x87	//一键复位
}enStripToolMotors,StripToolMotorDef;

//定义命令
//为了通用，不进行专门定义
typedef enum
{
	STRIP_CMD_0,
	STRIP_CMD_1,
	STRIP_CMD_2
}enStripCommands,StripCommandsDef;

//定义剥线状态
typedef enum
{
	STATE_STRIPPING,		
	STATE_STRIP_SUCCEED			
}enToolStripState,ToolStripStateDef;

//define the motion state
typedef enum
{
	MOTOR_UNKNOWN,
	MOTOR_HOMED,					//初始位置
	MOTOR_WORKING,				//工作中
	MOTOR_HOMING,				//复位中
	MOTOR_STOP_UNEXCEPT,		//意外停止(如工作或复位工程中被停止)
	MOTOR_WORK_REACH		//到达目标位置
}enToolMotorState,ToolMotorStateDef;

typedef struct
{
	unsigned char	grip_r_motor:3;
	unsigned char	cutter_motor:3;
	unsigned char	grip_l_motor:3;
	unsigned char	strip_state:1;
	unsigned char	strip_motor:3;
	unsigned char	slider_motor:3;
	uint16_t state;
}stToolState, ToolStateDef;


#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
