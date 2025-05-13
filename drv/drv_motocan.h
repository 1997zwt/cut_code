/**
  ******************************************************************************
  * @file    drv_motocan.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-12-29
  * @brief   大疆电机的驱动
  ******************************************************************************
  * @copy
  */
#ifndef  DRV_MOTOCAN_H
#define  DRV_MOTOCAN_H

#include "stm32f10x.h"
#include "../tasks/user_tasks.h"

#define DJ_MOTOR_VOLTAGE_LIMIT      10000
//CAN接收RX0中断使能
//0,不使能;1,使能.	
#define CAN_RX0_INT_ENABLE		1		

#define CAN_BURD_1M		0
#define CAN_BURD_500K	1
#define CAN_BURD_250K	2
#define CAN_BURD_125K	3

#define CAN_RX_pin  GPIO_Pin_11
#define CAN_TX_pin  GPIO_Pin_12
#define CAN_PORT		GPIOA

typedef struct
{
	int16_t speed_rpm;
	int16_t real_current;
	int16_t	set_voltage;    

	uint16_t angle;
	uint16_t last_angle;	//abs angle range:[0,8191]
	uint16_t offset_angle;
	int32_t	round_cnt;
	int32_t	total_angle;

	uint16_t msg_cnt;
}
MotorInfoDef;

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
uint8_t  CanBus_Send_Msg(uint16_t ID,uint8_t* data,uint8_t datalen);
//获取接收到的新数据
//返回接收到的数据的ID,如果没有接收到新的数据返回0
//接收到的数据存储在rxBuf中
uint8_t CanBus_GetRxData(uint8_t* rxBuf);

void DJ_SetMotorPos(uint8_t moto_id,int32_t pos);
void DJ_SetMotorVoltage(uint8_t moto_id, int16_t val);
int32_t DJ_GetMotorAngle(uint8_t moto_id);
int32_t DJ_GetMotorSpeed(uint8_t moto_id);
int16_t DJ_GetMotorCurrent(uint8_t moto_id);
void DJ_UpdateCanMotors(void);
int16_t DJ_GetMsgCounts(uint8_t moto_id);
#endif

