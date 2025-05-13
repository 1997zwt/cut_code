/**
  ******************************************************************************
  * @file    user_can.c 
  * @author  lyq
  * @version V1.0.0
  * @date    2015-12-17
  * @brief   实现CAN总线的初始化、CAN总线数据的接收和发送，如果数据需要按照一定的格式
  发送或者接收，需要在应用程序中根据协议对数据进行解析或者打包
  ******************************************************************************
  * @copy
  */ 

#include "drv_motocan.h"

uint8_t  CAN_Burd_Arry[4][4] = {
	{CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_3tq,4},			//1M
	{CAN_SJW_1tq,CAN_BS1_7tq,CAN_BS2_4tq,6},			//500K
	{CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_4tq,8},			//250K
	{CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_4tq,16} };		//125K

CanRxMsg	 RxMessage;
uint8_t	CanDataBuf[20][8] = {0}; //可缓存20组数据
uint8_t ReadPtr = 0, WritePtr = 0;
uint8_t CAN_RxStatus = 0;
uint16_t This_ID = 0;        //本机ID
MotorInfoDef gMotors[8];

void CanBus_Init(uint16_t selfID,uint8_t burd)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE 
	NVIC_InitTypeDef  NVIC_InitStructure;
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

	//初始化CAN的TX引脚
	GPIO_InitStructure.GPIO_Pin = CAN_TX_pin; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(CAN_PORT, &GPIO_InitStructure); 
	//初始化CAN的RX引脚
	GPIO_InitStructure.GPIO_Pin = CAN_RX_pin; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		
	GPIO_Init(CAN_PORT, &GPIO_InitStructure);

	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	//禁止时间触发模式
	CAN_InitStructure.CAN_TTCM = DISABLE;
	//出错管理，离线恢复模式设置
	CAN_InitStructure.CAN_ABOM = DISABLE;
	//自动唤醒模式设置
	//睡眠模式通过清除CAN_MCR寄存器的SLEEP位，由软件唤醒
	CAN_InitStructure.CAN_AWUM = DISABLE;
	//禁止报文自动重发（出错丢失）
	CAN_InitStructure.CAN_NART = ENABLE;
	//报文不锁定，如果FIFO中报文未读出，新的覆盖旧的
	CAN_InitStructure.CAN_RFLM = DISABLE;
	//多个报文同时等待发送时，优先级由标示符决定
	CAN_InitStructure.CAN_TXFP = DISABLE;
	//正常模式
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	//波特率设置
	//BaudRate = APB1/((SJW + BS1 + BS2)*Prescaler)
	CAN_InitStructure.CAN_SJW = CAN_Burd_Arry[burd][0];
	CAN_InitStructure.CAN_BS1 =  CAN_Burd_Arry[burd][1];
	CAN_InitStructure.CAN_BS2 =  CAN_Burd_Arry[burd][2];
	CAN_InitStructure.CAN_Prescaler =  (uint16_t)CAN_Burd_Arry[burd][3];
	CAN_Init(CAN1, &CAN_InitStructure);

	/* CAN filter init */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;	//选择过滤组0
	//屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	//过滤方式为32位的列表模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	//标示符寄存器的高11为为标准ID,为0的位忽略，为1的位必须匹配
	CAN_FilterInitStructure.CAN_FilterIdHigh = selfID<<5;   //CAN_FilterIdHigh--16位寄存器
	//第2位为RTR，第3位为IDE，只接收标准数据帧
	CAN_FilterInitStructure.CAN_FilterIdLow = CAN_ID_STD | CAN_RTR_DATA;
	//同样设置第二个标示符寄存器
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = selfID<<5;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_ID_STD | CAN_RTR_DATA;		
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	This_ID = selfID;

#if CAN_RX0_INT_ENABLE 	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    //抢占优先级0 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//子优先级为0 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
#endif

	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);	
}
uint8_t CanBus_Send_Msg(uint16_t ID, uint8_t *msg, uint8_t len)
{
	uint8_t mbox;
	uint16_t i = 0;
	CanTxMsg TxMessage;

	TxMessage.StdId = ID;     // 标准标识符为0
	TxMessage.ExtId = ID;     // 设置扩展标示符（29位）
	TxMessage.IDE = 0;        // 使用扩展标识符
	TxMessage.RTR = 0;        // 消息类型为数据帧，一帧8位
	TxMessage.DLC = len;      // 数据长度
	for (i = 0; i < len; i++) // 装载发送数据
		TxMessage.Data[i] = msg[i];

	i = 0xFFF;
	do
	mbox = CAN_Transmit(CAN1, &TxMessage);         //发送数据，返回使用的邮箱
	while ((mbox == CAN_TxStatus_NoMailBox) && (i--)); //若邮箱已满，则超时重发

	if (mbox == CAN_TxStatus_NoMailBox)
		return 2; //邮箱始终满载，发送失败

	i = 0;
	while ((CAN_TransmitStatus(CAN1, mbox) == CAN_TxStatus_Failed) && (i < 0XFFF))
		i++; //等待发送结束
	if (i >= 0XFFF)
		return 1; //发送超时返回
	return 0;
}
uint8_t CanBus_GetRxData(uint8_t* rxBuf)
{
	uint8_t i =0;

	if(ReadPtr == WritePtr)
	{
		return 0;
	}

	for(i=0;i<CAN_RxStatus;i++)
		rxBuf[i] = CanDataBuf[ReadPtr][i];

	ReadPtr++;
	ReadPtr %= 20;

	return CAN_RxStatus;
}

#if CAN_RX0_INT_ENABLE 
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	uint16_t id = 0;
	uint8_t i = 0;

	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	id = RxMessage.StdId;
	OSIntEnter();
	if(id < 100)
	{	
		CAN_RxStatus = RxMessage.DLC;
		for(i=0;i<CAN_RxStatus;i++)
			CanDataBuf[WritePtr][i] = RxMessage.Data[i];

		WritePtr++;
		WritePtr %= 20;
	}
	else if(id > 0x200)
	{
		id -= 0x201;
		if (id < 8)    //ID匹配
		{
			gMotors[id].speed_rpm = (uint16_t)(RxMessage.Data[2]<< 8 | RxMessage.Data[3]);
			gMotors[id].real_current = (uint16_t)(RxMessage.Data[4]<< 8 | RxMessage.Data[5]);
			gMotors[id].angle = (uint16_t)(RxMessage.Data[0]<< 8 | RxMessage.Data[1]);

			if (gMotors[id].msg_cnt++ <= 50)
			{
				gMotors[id].offset_angle = gMotors[id].angle;
			}
			else
			{
				if(gMotors[id].angle - gMotors[id].last_angle > 4096)
					gMotors[id].round_cnt --;
				else if (gMotors[id].angle - gMotors[id].last_angle < -4096)
					gMotors[id].round_cnt ++;
			}
			 gMotors[id].last_angle = gMotors[id].angle;
		}
	}
	OSIntExit();
}
#endif
void DJ_SetMotorPos(uint8_t moto_id,int32_t pos)
{
	gMotors[moto_id-1].round_cnt = pos/8192;
	gMotors[moto_id-1].offset_angle = gMotors[moto_id-1].angle;
}
void DJ_SetMotorVoltage(uint8_t moto_id, int16_t val)
{
	Limit(val, DJ_MOTOR_VOLTAGE_LIMIT);
	gMotors[moto_id-1].set_voltage = val;
}
int32_t DJ_GetMotorAngle(uint8_t moto_id)
{
	gMotors[moto_id-1].total_angle = gMotors[moto_id-1].round_cnt * 8192 + gMotors[moto_id-1].angle - gMotors[moto_id-1].offset_angle;
	return gMotors[moto_id-1].total_angle;
}
int32_t DJ_GetMotorSpeed(uint8_t moto_id)
{
	return gMotors[moto_id-1].speed_rpm;
}
int16_t DJ_GetMotorCurrent(uint8_t moto_id)
{
	return gMotors[moto_id-1].real_current;
}
int16_t DJ_GetMsgCounts(uint8_t moto_id)
{
	return gMotors[moto_id - 1].msg_cnt;
}
void DJ_UpdateCanMotors(void)
{
	uint8_t tx_data[8] = {0};

	if (DJ_CONTROL_ID == 0x200)
	{
		tx_data[0] = BYTE1(gMotors[0].set_voltage);
		tx_data[1] = BYTE0(gMotors[0].set_voltage);
		tx_data[2] = BYTE1(gMotors[1].set_voltage);
		tx_data[3] = BYTE0(gMotors[1].set_voltage);
		tx_data[4] = BYTE1(gMotors[2].set_voltage);
		tx_data[5] = BYTE0(gMotors[2].set_voltage);
		tx_data[6] = BYTE1(gMotors[3].set_voltage);
		tx_data[7] = BYTE0(gMotors[3].set_voltage);
	}
	else if (DJ_CONTROL_ID == 0x1FF)
	{
		tx_data[0] = BYTE1(gMotors[4].set_voltage);
		tx_data[1] = BYTE0(gMotors[4].set_voltage);
		tx_data[2] = BYTE1(gMotors[5].set_voltage);
		tx_data[3] = BYTE0(gMotors[5].set_voltage);
		tx_data[4] = BYTE1(gMotors[6].set_voltage);
		tx_data[5] = BYTE0(gMotors[6].set_voltage);
		tx_data[6] = BYTE1(gMotors[7].set_voltage);
		tx_data[7] = BYTE0(gMotors[7].set_voltage);
	}

	CanBus_Send_Msg(DJ_CONTROL_ID,tx_data,8);
}
