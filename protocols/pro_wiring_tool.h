/**
  ******************************************************************************
  * @file    pro_strip_tool.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-05-12
  * @brief   接线工具协议
  ******************************************************************************
  * @copy
  */
#ifndef		_PRO_WIRING_TOOL_H
#define 	_PRO_WIRING_TOOL_H

#include "stm32f10x.h"

#define		PRO_HEAD0	0xAA
#define		PRO_HEAD1 0x55

//定义命令
typedef enum
{
	WIRING_SLEEVE_LOCK=0x0B,			//套筒锁定
	WIRING_SLEEVE_UNLOCK,				//套筒解锁
	WIRING_SLEEVE_STOP,	
	WIRING_CLAMP_UNLOCK,				//线夹解锁
	WIRING_CLAMP_LOCK,						//线夹锁定
	WIRING_WIRE_LOCK,							//支线锁定
	WIRING_WIRE_UNLOCK,
	WIRING_WIRE_STOP,
	WIRING_SCREW_START,					//开始拧螺丝
	WIRING_SCREW_HOME,					//复位
	WIRING_SCREW_STOP,						//停止
	WIRING_SCREW_REVERSE,				//反转
	WIRING_SCREW_AUTO=0x1D			//全自主
}enWiringCommands,WiringCommandsDef;

//定义锁定状态
typedef enum
{	
	STOP,
	UNLOCK,
	MOVING,
	LOCKED,				
}enLockState,LockStateDef;

//define the motion state
typedef enum
{
	SCREW_STOP,						//无动作
	SCREW_WORKING,				//工作中
	SCREW_FAILURE,				//拧断失败
	SCREW_SUCCEED				//拧断成功
}enScrewAction,ScrewActionDef;

typedef struct
{
	unsigned char	:7;
	unsigned char	wire_in_place:1;			//支线到位
	unsigned char	screw_homed:1;			//拧螺丝电机复位状态
	unsigned char	screw_action:2;			//拧螺丝电机动作状态
	unsigned char	wire_state:2;	
	unsigned char	clamp_locked:1;			//线夹锁定状态
	unsigned char	sleeve_state:2;				//套筒状态
	uint16_t state;
}stToolState, ToolStateDef;


#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
