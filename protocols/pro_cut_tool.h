/**
  ******************************************************************************
  * @file    pro_cut_tool.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-23
  * @brief   断线工具协议
  ******************************************************************************
  * @copy
  */
#ifndef		_PRO_CUT_TOOL_H
#define 	_PRO_CUT_TOOL_H

#include "stm32f10x.h"

#define		PRO_HEAD0	0xAA
#define		PRO_HEAD1 0x55

#define		CUT_GET_STATE		0x000A		//查询状态
#define		CUT_CMD_START	0x0028
#define		CUT_CMD_HOME		0x0029
#define		CUT_CMD_STOP		0x002A
//补充协议
#define		CUT_CMD_MOVE_CW		0x00F0
#define		CUT_CMD_MOVE_CCW	0x00F1

//define the cut state
typedef enum
{
	STATE_UN_DEF,				//undefined
	STATE_CUTTING,				//is cutting
	STATE_CUT_FINISH			//cut wire successful
}enCutState,ToolCutStateDef;

//define the home state
typedef enum
{
	STATE_NOT_HOME,				//not on the home position	
	STATE_HOMED						//on the home position
}enHomeState, ToolHomeStateDef;

//define the motion state
typedef enum
{
	MOTION_STOPPED,				//the tool is stopped	
	MOTION_CUTTING,				//the tool is cutting
	MOTION_HOMING,				//the tool is homing
	MOTION_MOVING
}enMotionState, ToolMotionStateDef;

typedef struct
{
	unsigned char	:3;
	unsigned char	MotionState:2;
	unsigned char	HomeState:1;
	unsigned char	CutState:2;
}stToolState;

typedef union
{
	stToolState StateDef;
	uint8_t state;
}unToolState, ToolStateDef;


#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
