/**
  ******************************************************************************
  * @file    drv_can.h
  * @author  lyq
  * @version V1.0.0
  * @date    2015-12-17
  * @brief   实现CAN总线的初始化，CAN总线数据的接收和发送，如果数据需要按照一定的格式
                   发送或者接收，需要在应用程序中根据协议对数据进行解析或者打包
				   2020.8.25修改，增加数据缓冲，通过系统消息发送数据接收状态
  ******************************************************************************
  * @copy
  */
#ifndef  DRV_CAN_H
#define  DRV_CAN_H

#include "tasks/user_tasks.h"

//CAN接收RX0中断使能
//0,不使能;1,使能.	
#define CAN_RX0_INT_ENABLE		1		

#define CAN_BURD_1M		0
#define CAN_BURD_500K	1
#define CAN_BURD_250K	2
#define CAN_BURD_125K	3

extern OS_EVENT* gCanGetDataSem;
extern uint8_t err;
#define WaitCanDataUpdate()	 OSSemPend((OS_EVENT *)gCanGetDataSem, 0,&err);
//初始化CAN总线
//selfID:本设备的ID
//burd-波特率：		0-1M
//									1-500K
//									2-250K
//									3-125K
//mode-工作模式：	CAN_Mode_LoopBack(1)-回环模式
//									CAN_Mode_Normal(0)-普通模式
void CanBus_Init(uint16_t selfID,uint8_t burd/*,uint8_t mode*/);
//can总线根据参数表进行初始化
//void CanBus_Init(void);

//CAN总线发送数据
/*
ID			接收数据的can设备的ID
data			要发送的数据
datalen	要发送的数据长度
*/
bool  CanBus_Send_Msg(uint16_t ID,uint8_t* data,uint8_t datalen);
//获取接收到的新数据
//返回接收到的数据的ID,如果没有接收到新的数据返回0
//接收到的数据存储在rxBuf中
uint8_t CanBus_GetRxData(uint8_t* rxBuf);


#define CAN_TASK_STK_SIZE		300
extern OS_STK can_task_stk[CAN_TASK_STK_SIZE]; 
void Task_Canbus(void *p_arg);
#endif

