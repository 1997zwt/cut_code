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

typedef struct 
{
  int16_t diameter;  //电缆外径0.1mm
  int16_t skin;      //电缆皮厚0.1mm
  int16_t cut_gap;   //抱紧后刀尖与电缆的距离估计0.1mm
}stCableDef;

typedef struct 
{
  ModuleStateDef state;
  ToolMotorStateDef motor_state;  
  uint8_t mode;
  uint8_t step;
	int16_t motor_current;
  int16_t motor_speed;
  int32_t motor_pos;
  int16_t pos;
  int32_t aim_speed;
  int32_t aim_pos;
  uint32_t timer;
}stGripDef;

extern stCableDef gCable;
extern stGripDef gGrip;

void Grip_Init(uint8_t index);
ToolMotorStateDef Grip_GetMotorState(void);
CommandReturn Grip_Control(uint8_t cmd, int8_t data);
void Grip_Update(void);
int32_t Grip_GetPos(void);
uint8_t Grip_GetModuleState(void);
stCableDef* GetCableInfo(void);


#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
