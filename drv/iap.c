#include "iap.h"
#include "user_settings.h"

int32_t  iapUserSettings[IAP_SETTING_SIZE] = {0};
/************************************************************************/

void IAP_WriteFlag(uint16_t flag) //旧IAP程序写的标志位
{
#if (USE_BKP_SAVE_FLAG == 1)
	PWR->CR |= PWR_CR_DBP;
	BKP_WriteBackupRegister(IAP_FLAG_ADDR, flag);
	PWR->CR &= ~PWR_CR_DBP;
#else 
	FLASH_Unlock();
	DRV_FLASH_Write(IAP_FLAG_ADDR, &flag, 1);
	FLASH_Lock();
#endif	
}

void IAP_JUMP_WriteFlag(int32_t flag) //跳转时要写进rom的标志位
{
	iapUserSettings[IAP_VER_DEVICE_ID] = ReadUserSettings(VER_DEVICE_ID);
	iapUserSettings[IAP_WORK_TIMES] = ReadUserSettings(WORK_TIMES);
	iapUserSettings[IAP_JUMPFUN_ADDR] = flag;
	iapUserSettings[POWEROFISOK] = POWER_OFF_ISOK;
	FLASH_Unlock();
	DRV_FLASH_Write(IAP_FLAG_ADDR_NEW, (uint16_t*)iapUserSettings, 2*IAP_SETTING_SIZE);
	FLASH_Lock();
}

void IAP_ReadALLFlag(void) //上电读取所有IAP标志位到ram
{
	DRV_FLASH_Read(IAP_FLAG_ADDR_NEW, (uint16_t*)iapUserSettings, 2*IAP_SETTING_SIZE);
}


//主控程序成功运行写标志位1，电源标志位写2
void IAP_FUN_WriteFlag(uint8_t func,int32_t flag1,int32_t flag2) 
{
	if(func == IAP_WRITE_RUNNING)
	{
		iapUserSettings[FINISH_RUNNING_ADDR] = flag1;
	}
	else if(func == IAP_WRITE_POWER)
	{
		iapUserSettings[POWERON_ADDR] = flag2;
		if(flag2 == POWER_OFF_DATA)
		{
			iapUserSettings[POWEROFISOK] = POWER_OFF_ISOK;
		}
	}
	else if(func == IAP_WRITE_ALL)
	{
		iapUserSettings[FINISH_RUNNING_ADDR] = flag1;
		iapUserSettings[POWERON_ADDR] = flag2;
		if(flag2 == POWER_OFF_DATA)
		{
			iapUserSettings[POWEROFISOK] = POWER_OFF_ISOK;
		}
	}
	FLASH_Unlock();
	DRV_FLASH_Write(IAP_FLAG_ADDR_NEW, (uint16_t*)iapUserSettings, 2*IAP_SETTING_SIZE);
	FLASH_Lock();
}

/*读取某一个IAP标志位*****************************************************/
int32_t IAP_ReadSingleFlag(uint8_t addr)
{
	return iapUserSettings[addr];
}

/************************************************************************/

void IAP_Init(void)
{
#if (USE_BKP_SAVE_FLAG == 1)
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN , ENABLE); 
#endif
	NVIC_SetVectorTable(STM32_FLASH_BASE, IAP_FLASH_SIZE);//设置中断向量表
}

void IAP_Handle(u8 * cmd)
{
	if(strcmp((char *)cmd, "update") == 0)
	{
		IAP_JUMP_WriteFlag(UPDATE_FLAG_DATA);
		IAP_WriteFlag(UPDATE_FLAG_DATA);
		NVIC_SystemReset();
	}
	else if(strcmp((char *)cmd, "erase") == 0)
	{
		IAP_JUMP_WriteFlag(ERASE_FLAG_DATA);
		IAP_WriteFlag(ERASE_FLAG_DATA);
		NVIC_SystemReset();		
	}
	else if(strcmp((char *)cmd, "menu") == 0)
	{
		IAP_JUMP_WriteFlag(INIT_FLAG_DATA);
		IAP_WriteFlag(INIT_FLAG_DATA);
		NVIC_SystemReset();	
	}
	else if(strcmp((char *)cmd, "runapp") == 0)//reset
	{
		NVIC_SystemReset();	
	}
	else
	{
		//printf("指令有误\r\n");
		NVIC_SystemReset();	
	}
}

