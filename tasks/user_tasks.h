/**
  ******************************************************************************
  * @file    user_tasks.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-23
  * @brief   配置用户任务的优先级，启动用户任务
  ******************************************************************************
  * @copy
  */
#ifndef		_USER_TASKS_H
#define 	_USER_TASKS_H

#include "user_define.h"
#include "ucos_ii.h"         //ucosII system
#include "os_cfg.h"			 //ucos ii 配置函数

/************************************************************/
//以下内容不要修改
#define SysTick_init() SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC)

#define STARTUP_TASK_PRIO						3
#define STARTUP_TASK_STK_SIZE               100
extern OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE]; 
void Task_Start(void *p_arg);

#define PROTO_TASK_PRIO								5
#define PROTO_TASK_STK_SIZE						300
extern OS_STK protol_task_stk[PROTO_TASK_STK_SIZE]; 
//用户的协议解析函数，需要用户自己定义
extern void Task_Protocol(void *p_arg);

//用户初始化函数，需要用户自己定义
extern void InitAllDevice(void);

extern void SendDeviceInfo(void);
/************************************************************/
//以下内容需要用户修改
/*包含用户应用程序的头文件*/

#include "user_settings.h"					//提供用户配置功能
#include "cut_tool/cut_tool.h"						//断线工具
//#include "strip_tool/strip_tool_a_v4.h"				//剥线工具，带自动调刀功能,新电路板
//#include "strip_tool/strip_tool_m.h"					//手动剥线工具，带自动调刀功能，CAN总线通讯
//#include "wiring_tool/wiring_tool.h"				//接线工具
//#include "wiring_tool/wiring_tool_m.h"				//接线工具
//#include "cutter_regulator/cutter_regulator_v4.h"			//调刀工具

/*定义用户应用程序的优先级*/
#define TOOL_TASK_PRIO									4
#define CAN_TASK_PRIO										6
#define SMCL_TASK_PRIO									10
#define MUTEX_TASK_PRIO                 1


#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
