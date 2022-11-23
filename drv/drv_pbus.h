/**
  ******************************************************************************
  * @file    drv_pbus.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-11-13
  * @brief   电源总线驱动
  2021.1.28 增加了发送前的协议检测，防止发送帧错误；限制重发次数为5次，防止
  一直重发导致总线死机。
  ******************************************************************************
  * @copy
  */
#ifndef  DRV_PBUS_H
#define  DRV_PBUS_H

#include "tasks/user_tasks.h"

//CAN接收RX0中断使能
//0,不使能;1,使能.	
#define CAN_RX0_INT_ENABLE		1		

void PowerBus_Init(void);
void PowerBus_WriteTxBuf(uint8_t* data,uint8_t len, uint8_t flg);
uint8_t GetPowerBusData(uint8_t* data);
void PowerBus_ClearRxBuf(void);

//电源总线
#define PBUS_TASK_PRIO								8
#define PBUS_TASK_STK_SIZE						100
extern OS_STK pbus_task_stk[PBUS_TASK_STK_SIZE];
void Task_PowerBus(void *p_arg);
#endif

