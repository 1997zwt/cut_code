#include "drv_pbus.h"

#ifdef USE_POWER_BUS

OS_STK pbus_task_stk[PBUS_TASK_STK_SIZE];

vu8 PbusTxdata[100] = {0};
vu8 PbusRxData[100] = {0};
uint8_t lastFrame[10] = {0};//存储最近的一条指令
vu8 pWrite = 0, pRead = 0;
vu8 frameFinishFlg = 1;//0未完成，1完成，2超时
vu8 PbusRxDataLen = 0;
vu8 bReSend = 0;

void PowerBus_Init(void)
{
	USART_Config(POWER_BUS_PORT,9600);
	OSTaskCreate(Task_PowerBus, NULL, (OS_STK *)&pbus_task_stk[PBUS_TASK_STK_SIZE - 1], PBUS_TASK_PRIO);
}
void PowerBus_WriteTxBuf(uint8_t* data,uint8_t len, uint8_t flg)
{  
	uint8_t i=0;

	if(pWrite == (pRead - 1))     /*防止套圈*/
	{
		return;
	}

	bReSend = flg;
	for(i=0;i<len;i++)
	{
		PbusTxdata[pWrite] = *(data+i);
		pWrite++;
		pWrite %= 100;
	}
}
void PowrBus_SendData(void)
{
	static uint8_t len = 0;
	static uint8_t timeout = 0;
  static uint8_t re_time = 0;
	uint8_t check = 0,framelen = 0;

	if (frameFinishFlg == 0)
	{
		timeout += 10;
		if (timeout > 100)
		{
			frameFinishFlg = 2;
			if(re_time++ > 5)//重发5次
			{
				re_time = 0;
				frameFinishFlg = 1;
			}
		}
		return;
	}
	timeout = 0;
	if (frameFinishFlg == 1)
	{
		if(pRead == pWrite) return;
		len = 0;
		while(pRead != pWrite)
		{
			lastFrame[len] = PbusTxdata[pRead];
			if (len == 0)
			{
				if (lastFrame[len] == PRO_HEAD0)	len++;
			}
			else if (len == 1)
			{
				if (lastFrame[len] == PRO_HEAD1)	len++;
			}
			else if (len == 2)
			{
				check = lastFrame[len];
				framelen = lastFrame[len] + 3;
				len++;
				if (framelen > 10) 
					return;
			}
			else if (len < framelen-1)
			{
				check += lastFrame[len++];
			}
			else if (len == framelen-1)
			{
				if (check == lastFrame[len] && lastFrame[3] == 4)
				{
					len++;
					break;
				}
				else
					return;
			}
			pRead++;
			pRead %= 100;
		}
	}
	UART_SendBuf(POWER_BUS_PORT,lastFrame,len);

	if(bReSend) 
		frameFinishFlg = 0;
	else 
		frameFinishFlg = 1;
}
uint8_t GetPowerBusData(uint8_t* data)
{
	uint8_t i=0;

	if(!PbusRxDataLen) return 0;
	for(i=0;i<PbusRxDataLen;i++)
	{
		data[i] = PbusRxData[i];
	}
	return PbusRxDataLen;
}
void PowerBus_ClearRxBuf(void)
{
	PbusRxDataLen = 0;
}
void Task_PowerBus(void *p_arg)
{
	uint8_t  check=0;
	uint16_t len=0, index=0;
	uint8_t  data=0;

	for(;;)
	{
		OSSemPend((OS_EVENT *)gUsartGetDataSem[POWER_BUS_BUF], 10,&err);
		if(USART_FrameBufRead(POWER_BUS_BUF, &data))
		{
			PbusRxData[index] = data;
			if(index > 50) index = 0;
			if (index == 0)
			{
				if(data == PRO_HEAD0)	index++;
				else index = 0;
			}
			else if (index == 1)
			{
				if(data == PRO_HEAD1)	index++;
				else index = 0;
			}
			else if (index == 2)
			{
				len = data;
				check = data;
				index ++;
			}
			else if (index == 3)
			{
				if (data == TOOL_TYPE_SETTINGS)
				{
					len = len*4+7;
				}
				else
				{
					len = len + 3;
				}
				check += data;
				index ++;
			}
			else if (index < len - 1)
			{
				check += data;
				index++;
			}
			else if(index == len - 1)
			{
				if (check == data)
				{
					frameFinishFlg = 1;
					PbusRxDataLen = len;
//  					if (PbusRxData[5] == MOTOR_CMD_GET)//状态反馈
//  					{
//  						gCutterState = PbusRxData[6];
//  						gRegulState = PbusRxData[7];
// 						gSensorState = PbusRxData[8];
//  					}
// 					else	//命令反馈
// 					{
// 						StripTool_CommandReply(PbusBuf[4],PbusBuf[5],PbusBuf[6]);
// 					}
				}
				index = 0;
			}
			else
			{
				index = 0;
			}
		}
		else
		{
			PowrBus_SendData();
		}
	}
}
#endif
