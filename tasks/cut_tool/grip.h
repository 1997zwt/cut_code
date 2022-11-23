/**
  ******************************************************************************
  * @file    grip.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-12-31
  * @brief   抱紧模块的控制程序
  说明：抱紧模块第一次动作之前必须复位，否则存在憋死的风险。
  ******************************************************************************
  * @copy
  */
#ifndef _GRIP_H
#define _GRIP_H

#include "../user_tasks.h"

void Grip_Init(uint8_t index);
ToolMotorStateDef Grip_GetMotorState(void);
extern ToolMotorStateDef gGripMotorState;
CommandReturn Grip_Control(uint8_t cmd, int8_t data);
void Grip_Update(void);
uint8_t Grip_GetModuleState(void);

#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
