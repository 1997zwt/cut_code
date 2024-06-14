#include "user_settings.h"
#include "iap.h"

//存储用户数据，上电从flash中读取
extern int32_t gWorkTimes;
int32_t  gUserSettings[USER_SETTING_SIZE] = {0};
void SaveUserSettings(void);
bool LoadUserSettings(void);

extern OS_EVENT* USART_MUTEX;
extern uint8_t m_err;
 
void InitUserSettings(uint8_t flg)
{
	uint8_t i=0;

	LoadUserSettings();

	if(flg == 0)
	{
		gUserSettings[SYS_FLGS] = -1;
		gUserSettings[VER_MODIFY_TIMES] = 0;
	}

	if ((!TSB(gUserSettings[SYS_FLGS],FLG_SAVE)) || (gUserSettings[SYS_FLGS] == -1))// 测试对应位是否1
	{
		for (i=0;i<USER_SETTING_SIZE;i++)
		{
			gUserSettings[i]=0;
		}
		
		gUserSettings[VER_SOFT] = 4222;//3.01
		gUserSettings[VER_SOFT_DATE] = 20240614;//2020.6.2
		gUserSettings[VER_SETTINGS] = 301;//
		gUserSettings[VER_SETTINGS_DATE] = 20200604;//
		gUserSettings[VER_MODIFY_TIMES] = 0;
#ifdef _CUT_TOOL_H
		gUserSettings[SYS_FLGS] = ((TOOL_TYPE_CUT|TOOL_OPERATE_TYPE)<<24);
		gUserSettings[CUT_ENABLE] = 1;
		gUserSettings[CUT_HOME_SPEED] = 6000;
//		gUserSettings[CUT_HOME_TIME] = 5500;        //
		gUserSettings[CUT_CUT_SPEED_1] = 10000;       //
		gUserSettings[CUT_CUT_TIME_1] = 0;
		gUserSettings[CUT_CUT_CURRENT_1] = 2500;    //
		gUserSettings[CUT_CUT_SPEED_2] = 0;
		gUserSettings[CUT_CUT_TIME_2] = 0;        //
		gUserSettings[CUT_CUT_CURRENT_2] = 0;
    gUserSettings[CUT_CUT_TIME_3] = 0;        //
        
		gUserSettings[GRIP_MOVE_SPEED] = 8000;     //
		gUserSettings[GRIP_WORK_CURRENT] = 1200;    //800有线的时候用的是电流
		gUserSettings[GRIP_HOME_CURRENT] = 1000;  //500第一次复位的时候用电流
		gUserSettings[GRIP_MOVE_OVERTIME] = 7000;
		gUserSettings[GRIP_POSITIVE_LIMIT] = 300;  //没线的时候用的是位置限位
		gUserSettings[GRIP_NEGATIVE_LIMIT] = 0;  //有线的时候用的是位置限位

#endif
#ifdef _STRIP_TOOL_H
		gUserSettings[SYS_FLGS] = ((TOOL_TYPE_STRIP|TOOL_OPERATE_TYPE)<<24);
		gUserSettings[CUTTER_REGULATOR_ENABLE] = 1;
		gUserSettings[CUTTER_REGULATOR_WORK_SPEED] = 10;
		gUserSettings[CUTTER_REGULATOR_HOME_SPEED] = 60;
		gUserSettings[CUTTER_REGULATOR_HOME_BACKTIME] = 2300;
		gUserSettings[CUTTER_REGULATOR_HOME_CURRENT] = 500;
		gUserSettings[CUTTER_REGULATOR_MOVE_OVERTIME] = 10000;
		gUserSettings[CUTTER_REGULATOR_REVERSE_SPEED] = 50;
		gUserSettings[CUTTER_REGULATOR_REVERSE_TIME] = 600;
		gUserSettings[STRIP_ENABLE] = 1;
		gUserSettings[GRIP_MOVE_SPEED] = 8000;
		gUserSettings[GRIP_WORK_CURRENT] = 1500;
		gUserSettings[GRIP_HOME_CURRENT] = 1000;
		gUserSettings[GRIP_MOVE_OVERTIME] = 5000;
		gUserSettings[GRIP_POSITIVE_LIMIT] = 2500000;
		gUserSettings[GRIP_NEGATIVE_LIMIT] = 200000;
//		gUserSettings[STRIP_CUTTER_MOVE_SPEED] = 80;
//		gUserSettings[STRIP_CUTTER_WORK_CURRENT] = 500;
//		gUserSettings[STRIP_CUTTER_HOME_CURRENT] = 500;
//		gUserSettings[STRIP_CUTTER_REVERSE_SPEED] = 40;
//		gUserSettings[STRIP_CUTTER_REVERSE_TIME] = 500;
//		gUserSettings[CUTTER_REGULATOR_TIME_1] = 500;		
//		gUserSettings[STRIP_AUTO_REPEAT] = 3;
//		gUserSettings[STRIP_SLIDER_WORK_SPEED] = 1;
//		gUserSettings[STRIP_STRIP_WORK_SPEED] = 2200;
//		gUserSettings[STRIP_STRIP_TURNS_0] = 2;
//		gUserSettings[STRIP_STRIP_TURNS_1] = 5;
//		gUserSettings[STRIP_STRIP_TURNS_2] = 2;
//		gUserSettings[STRIP_SLIDER_TIME_1] = 2000;
//		gUserSettings[STRIP_SLIDER_TIME_2] = 2000;
//		gUserSettings[STRIP_SLIDER_TIME_3] = 10000;
#endif
#ifdef _WIRING_TOOL_H
		gUserSettings[SYS_FLGS] = ((TOOL_TYPE_WIRING_BX|TOOL_OPERATE_TYPE)<<24);
		gUserSettings[WIRING_ENABLE] = 1;
#endif
#ifdef _CUTTER_REGULATOR_H
		gUserSettings[SYS_FLGS] = ((TOOL_TYPE_CUTTER_REGULATOR|TOOL_OPERATE_TYPE)<<24);
#endif
#ifdef USE_PROTOCOL_V2
		gUserSettings[SYS_FLGS] |= (2<<16);
#endif
#ifdef USE_PROTOCOL_V3
		gUserSettings[SYS_FLGS] |= (3<<16);
#endif
	gUserSettings[VER_DEVICE_ID] = iapUserSettings[IAP_VER_DEVICE_ID];
	gUserSettings[WORK_TIMES] = iapUserSettings[IAP_WORK_TIMES];
	if(gUserSettings[VER_DEVICE_ID]==-1 || gUserSettings[WORK_TIMES]==-1)
	{
		gUserSettings[VER_DEVICE_ID]=0;
		gUserSettings[WORK_TIMES] = 0;
	}
	
	}
		
	gUserSettings[POWER_LOW_LIMIT] = 10;		
	if(gUserSettings[VER_MODIFY_TIMES] == 0)
	{
		SaveUserSettings();
	}		
}

void WriteUserSettings(uint8_t addr, int32_t val)
{
	gUserSettings[addr] = val;
}

//接口函数
int32_t ReadUserSettings(uint8_t addr)
{
	return gUserSettings[addr];
}
void SaveUserSettings(void)
{
	uint8_t i=0;

	STB(gUserSettings[SYS_FLGS],FLG_SAVE);
	CLB(gUserSettings[SYS_FLGS],FLG_LOAD);
	gUserSettings[USER_SETTING_CHECK] = 0;
	gUserSettings[VER_MODIFY_TIMES]++;

	for(i=0;i<USER_SETTING_SIZE-1;i++)
	{
		gUserSettings[USER_SETTING_CHECK] += gUserSettings[i];
	}

	DRV_FLASH_Write(USER_FLASH_SAVE_BASE, (uint16_t*)gUserSettings, USER_SETTING_SIZE*2);
}

bool LoadUserSettings(void)
{
	uint8_t i=0;
	int32_t check = 0;

	DRV_FLASH_Read(USER_FLASH_SAVE_BASE, (uint16_t*)gUserSettings, USER_SETTING_SIZE*2);

	if (!TSB(gUserSettings[SYS_FLGS],FLG_SAVE)) return FALSE;// 测试对应位是否1

	for(i=0;i<USER_SETTING_SIZE-1;i++)
	{
		check += gUserSettings[i];
	}

	if (check == gUserSettings[USER_SETTING_CHECK])  
	{
		STB(gUserSettings[SYS_FLGS],FLG_LOAD);// 寄存器相应位置1
		return TRUE;
	}
	
	return FALSE;
}

static uint8_t SendBuf[20] = {0xAA,0x55,0x01,TOOL_TYPE_SETTINGS,USER_FLASH_READ,0x00,0x00,0x00,0x00,0x00,0x00};
void SendUserSettings(uint8_t addr)
{
	int32_t val;

	val = ReadUserSettings(addr);
	SendBuf[5] = addr;
	SendBuf[6] = BYTE3(val);
	SendBuf[7] = BYTE2(val);
	SendBuf[8] = BYTE1(val);
	SendBuf[9] = BYTE0(val);
	SendBuf[10] = SendBuf[2] + SendBuf[3] + SendBuf[4] + SendBuf[5] + SendBuf[6] + SendBuf[7] + SendBuf[8] + SendBuf[9];
	OSMutexPend(USART_MUTEX,100,&m_err);
	if (OS_ERR_NONE == m_err)
	{
		UART_SendBuf(TOOL_CONTROL_PORT,SendBuf,11);
		OSMutexPost(USART_MUTEX);	
	}
}
void SendAllSettings(void)
{
	uint8_t buf[USER_SETTING_SIZE*4+6] = {0xAA,0x55,USER_SETTING_SIZE ,TOOL_TYPE_SETTINGS,USER_FLASH_LOAD,0};
	//这里的数据长度与协议不对应，USER_SETTING_SIZE*4+3才是定义的长度，要注意
	uint16_t i = 0, len = 0;

	if (!TSB(gUserSettings[SYS_FLGS],FLG_LOAD))
	{
		LoadUserSettings();
	}
	//发送的数据是低字节在前，与查询不同
	memcpy(buf+6,gUserSettings,USER_SETTING_SIZE*4);

	len = USER_SETTING_SIZE*4+7;
	for (i=2;i<len-1;i++)
	{
		buf[len-1] += buf[i];
	}
	OSMutexPend(USART_MUTEX,100,&m_err);
	if (OS_ERR_NONE == m_err)
	{
	UART_SendBuf(TOOL_CONTROL_PORT,buf,len);
		OSMutexPost(USART_MUTEX);
	}
}

void ConfitAllSettings(uint8_t* data)
{
	uint16_t addr=0;
	int32_t val;
	//前7为只读，不可覆盖
	for (addr=0;addr<USER_SETTING_SIZE;addr++)
	{
		if (addr <= VER_MODIFY_TIMES)
		{
		}
		else
		{
			val = CombineU32_HF(data+addr*4);
			WriteUserSettings(addr,val);
		}
	}
	#ifdef USE_BOOTLOADER
	WriteIAPSettings();
	#endif
}

void WriteIAPSettings(void)
{
	iapUserSettings[IAP_VER_DEVICE_ID] = gUserSettings[VER_DEVICE_ID];
	iapUserSettings[IAP_WORK_TIMES] = gUserSettings[WORK_TIMES];
	gWorkTimes = iapUserSettings[IAP_WORK_TIMES];
}

void ConfigUserSettings(uint8_t cmd, uint8_t* data)
{
	uint8_t addr = 0;
	int32_t val = 0;

	switch(cmd)
	{
	case USER_FLASH_WRITE:
		addr = data[0];
		if (addr <= VER_MODIFY_TIMES)
		{
			//只读
		}
		else
		{
			val = CombineU32_HF(data+1);
			WriteUserSettings(addr,val);
			#ifdef USE_BOOTLOADER
			WriteIAPSettings();
			#endif
		}
		break;
	case USER_FLASH_READ:
		addr = data[0];
		SendUserSettings(addr);
		break;
	case USER_FLASH_LOAD:
		SendAllSettings();
		break;
	case USER_FLASH_DOWNLOAD:
		ConfitAllSettings(data+1);
		break;
	case USER_FLASH_SAVE:
		SaveUserSettings();
		break;
	case USER_FLASH_WRITE_SUPER:		//超级权限
		addr = data[0];
		if (addr <= VER_MODIFY_TIMES )
		{
			val = CombineU32_HF(data+1);
			WriteUserSettings(addr,val);
			#ifdef USE_BOOTLOADER
			WriteIAPSettings();
			#endif
		}
		break;
	}
}
