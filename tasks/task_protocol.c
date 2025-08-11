#include "ucos_ii.h"    
#include "user_tasks.h"
#include "driver_cfg_cut.h"

OS_STK protol_task_stk[PROTO_TASK_STK_SIZE]; 

uint8_t gTransMode = PROTOCOL_MODE;

#ifdef	USE_PROTOCOL_V2
void Task_Protocol(void *p_arg)
{
	uint8_t databuf[256] = {0};
	uint8_t  check=0;
	uint16_t cmd=0, len=0, index=0;
	uint8_t  motor = 0,data=0;

	for(;;)
	{
		WaitUsartDataUpdate(TOOL_CONTROL_BUF); 		
		if(USART_FrameBufRead(TOOL_CONTROL_BUF, &data))
		{	
			if (gTransMode == PASS_THROUTH_MODE)
			{
		#ifdef  _STRIP_TOOL_ZK_H
//				UART_Sendbyte(POWER_BUS_PORT,data);
		#endif
			}
			else
			{
				databuf[index] = data;
				if (index > 200) index = 0;
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
						if (databuf[3] == TOOL_TYPE_SETMODE)
						{
							gTransMode = databuf[4];
						}
	#ifdef _USER_SETTINGS_H
						if (databuf[3] == TOOL_TYPE_SETTINGS)
						{
							cmd = databuf[4];
							ConfigUserSettings(cmd,&databuf[5]);
						}
	#endif
	#ifdef	__IAP_H_
						if (databuf[3] == TOOL_TYPE_UPDATE)
						{
							if (databuf[4] == 1)	//升级程序
							{
						//		IAP_Handle("update");
							}
						}
						else if (databuf[3] == 00)
						{
							if (databuf[4] == 0x30 && databuf[4] == 0x03)	//升级程序
							{
								IAP_Handle("menu");
							}
						}
	#endif
	#ifdef _CUT_TOOL_H
						//第二代协议转换为第三代协议
						motor = databuf[4];
						data = 0;
						if (motor == 0x22)
						{
							CutTool_Control(0,TOOL_CMD_READY,&data);
						}
						else if (motor == 0x21)
						{
							CutTool_Control(0,TOOL_CMD_WORK,&data);
						}
						else if (motor == 0x23)
						{
							CutTool_Control(0,TOOL_CMD_HOME,&data);
						}
						else if (motor == 0x20)
						{
							CutTool_Control(0,TOOL_CMD_STOP,&data);
						}
						else if (motor == 0x2a)
						{
							cmd = databuf[5];
							CutTool_Control(1,cmd,&data);
						}
						else if (motor == 0x2b)
						{
							cmd = databuf[5];
							CutTool_Control(2,cmd,&data);
						}
	#endif
	#ifdef  _STRIP_TOOL_ZK_H
						//第二代协议转换为第三代协议
						motor = databuf[4];
						if (motor == 0x83)
						{
							if(databuf[5] == 3) databuf[5] = MOTOR_CMD_WORK;
							else databuf[6] = 0;
							StripTool_Control((1<<TOOL_MOTOR_5),databuf[5],&databuf[6]);
						}
						else if (motor == 0x88)
						{
							StripTool_Control((1<<TOOL_MOTOR_6),databuf[5],&databuf[6]);
						}
						else if (motor == 0x87)
						{
							StripTool_Control((1<<TOOL_MOTOR_2),databuf[5],&databuf[6]);
						}
						else if (motor == 0x80)
						{
							StripTool_Control((1<<TOOL_MOTOR_4),databuf[5],&databuf[6]);
						}
						else if (motor == 0x82)
						{
							if(databuf[5] == 3) databuf[5] = MOTOR_CMD_WORK;
							else databuf[6] = 0;
							StripTool_Control((1<<TOOL_MOTOR_1),databuf[5],&databuf[6]);
						}
						else if (motor == 0x81)//剥线反转
						{
							cmd = databuf[5];
							//反转，方向反了修改这个地方
							if (cmd == 2)
							{
								cmd = MOTOR_CMD_CCW;
							}
							else if (cmd == 3)
							{
								cmd = MOTOR_CMD_WORK;
							}
							else
							{
								databuf[6] = 0;
							}
							StripTool_Control((1<<TOOL_MOTOR_3),cmd,&databuf[6]);
						}
						else if (motor == 0x84)//剥线复位
						{
							StripTool_Control((1<<TOOL_MOTOR_3),MOTOR_CMD_HOME,&databuf[6]);
						}
						else if (motor == 0x85)//开始剥线
						{
							StripTool_Control(0,TOOL_CMD_WORK,&databuf[6]);
						}
						else if (motor == 0x86)
						{
							StripTool_Control(0,TOOL_CMD_STOP,&databuf[6]);
						}
						else if (motor == 0x89)
						{
							StripTool_Control(0,TOOL_CMD_HOME,&databuf[6]);
						}
						else if (motor == 0x90)
						{
							StripTool_Control(0,TOOL_CMD_READY,&databuf[6]);
						}
	#endif
	#ifdef  _WIRING_TOOL_H
						//第二代协议转换为第三代协议
						switch(databuf[4])
						{
						case 0x0B:
							WiringTool_Control((1<<TOOL_MOTOR_3),MOTOR_CMD_WORK,&data);
							break;
						case 0x0C:
							WiringTool_Control((1<<TOOL_MOTOR_3),MOTOR_CMD_HOME,&data);
							break;
						case 0x0D:
							WiringTool_Control((1<<TOOL_MOTOR_3),MOTOR_CMD_STOP,&data);
							break;
						case 0x0E:
							WiringTool_Control((1<<TOOL_MOTOR_4),MOTOR_CMD_HOME,&data);
							break;
						case 0x0F:
							WiringTool_Control((1<<TOOL_MOTOR_4),MOTOR_CMD_WORK,&data);
							break;
						case 0x10:
							WiringTool_Control((1<<TOOL_MOTOR_1),MOTOR_CMD_WORK,&data);
							break;
						case 0x11:
							WiringTool_Control((1<<TOOL_MOTOR_1),MOTOR_CMD_HOME,&data);
							break;
						case 0x12:
							WiringTool_Control((1<<TOOL_MOTOR_1),MOTOR_CMD_STOP,&data);
							break;
						case 0x13:
							WiringTool_Control((1<<TOOL_MOTOR_5),MOTOR_CMD_WORK,&data);
							break;
						case 0x14:
							WiringTool_Control((1<<TOOL_MOTOR_5),MOTOR_CMD_HOME,&data);
							break;
						case 0x15:
							WiringTool_Control((1<<TOOL_MOTOR_5),MOTOR_CMD_STOP,&data);
							break;
						case 0x16:
							data = 50;
							WiringTool_Control((1<<TOOL_MOTOR_5),MOTOR_CMD_CCW,&data);
							break;
						case 0x1D:
							WiringTool_Control(0,TOOL_CMD_WORK,0);
							break;
						case 0x1E:
							WiringTool_Control((1<<TOOL_MOTOR_2),MOTOR_CMD_WORK,&data);
							break;
						case 0x20:
							WiringTool_Control((1<<TOOL_MOTOR_2),MOTOR_CMD_STOP,0);
							break;
						case 0x1F:
							WiringTool_Control((1<<TOOL_MOTOR_2),MOTOR_CMD_HOME,0);
							break;
						}
	#endif
	#ifdef  _STRIP_TOOL_TD_H
						cmd = databuf[4];
						CutterRegulator_Control(databuf[4],databuf[5],&databuf[6]);
	#endif
					}
					index = 0;
				}	
				else
				{
					index = 0;
				}
			}
		}
	}
}

#elif defined USE_PROTOCOL_V3
void Task_Protocol(void *p_arg)
{
	uint8_t databuf[256] = {0};
	uint8_t data=0, check=0;
	uint16_t  len=0, index=0;

	for(;;)
	{
		WaitUsartDataUpdate(TOOL_CONTROL_BUF); 		
		if(USART_FrameBufRead(TOOL_CONTROL_BUF, &data))
		{			
			if (gTransMode == PASS_THROUTH_MODE)
			{
	#ifdef  _STRIP_TOOL_ZK_H			
//				UART_Sendbyte(POWER_BUS_PORT,data);
	#endif
			}
			else
			{
				databuf[index] = data;
				if (index > 200) index = 0;
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
						if(len <= 7) databuf[6] = 0;
						if (databuf[3] == TOOL_TYPE_SETMODE)
						{
							gTransMode = databuf[4];
						}						
						if (databuf[3] == DEVICE_CMD_INFO)   //查询工具信息
						{
							SendDeviceInfo();
							if(len==11)
							{
								RTC_SetTime(databuf+4);
							}
						}			
						if (databuf[3] == DEVICE_SET_NO)
						{
							WriteUserSettings(VER_DEVICE_ID,databuf[4] + (databuf[5]<<8) + (databuf[6]<<16) + (databuf[7]<<24));
							SaveUserSettings();
						}						
	#ifdef _USER_SETTINGS_H
						if (databuf[3] == TOOL_TYPE_SETTINGS)
						{
							ConfigUserSettings(databuf[4],&databuf[5]);
						}
	#endif
	#ifdef	__IAP_H_
						if (databuf[3] == DEVICE_CMD_UPGRADE)
						{
							IAP_Handle("menu");
						}
	#endif
	#ifdef _CUT_TOOL_H
						if ((databuf[3]&0x7F) == TOOL_TYPE_CUT)
						{
							CutTool_Control(databuf[4],databuf[5],&databuf[6]);
						}
	#endif
	#if  (defined _STRIP_TOOL_ZK_H || defined _STRIP_TOOL_BXM_H)				
						if ((databuf[3]&0x7F) == TOOL_TYPE_STRIP)
						{
							StripTool_Control(databuf[4],databuf[5],&databuf[6]);
						}
	#endif
	#ifdef  _WIRING_TOOL_H
						if ((databuf[3]&0x7F) == TOOL_TYPE_WIRING_BX)
						{
							WiringTool_Control(databuf[4],databuf[5],&databuf[6]);
						}
	#endif
	#ifdef  _STRIP_TOOL_TD_H					
						if ((databuf[3]&0x7F) == TOOL_TYPE_CUTTER_REGULATOR)
						{
							CutterRegulator_Control(databuf[4],databuf[5],&databuf[6]);
						}
	#endif
#ifdef USE_LOG
						databuf[len - 1] = 0;
						databuf[len] = 0;
						databuf[len+1] = 0;
						databuf[len+2] = 0;
						Log_RecordCmdInfo(&databuf[3]);
	#endif
					}
					index = 0;
				}	
				else
				{
					index = 0;
				}
			}
		}
	}
}
#else
void Task_Protocol(void *p_arg)
{	
	for(;;)
	{
		OSTimeDly(5000);
	}
}
#endif


