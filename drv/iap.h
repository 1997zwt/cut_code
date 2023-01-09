#ifndef __IAP_H_
#define __IAP_H_

#include "stm32f10x.h"
#include "drv_flash.h"
#include <string.h>

//IAP用工具类型
#define IAP_TOOL_NUM		0x01

/* Define if use bkp save flag  -------------------------------*/
#define USE_BKP_SAVE_FLAG     0

#if (USE_BKP_SAVE_FLAG == 1)
  #define INIT_FLAG_DATA      0x0000  //默认标志的数据(空片子的情况)
#else
  #define INIT_FLAG_DATA      0xFFFF   //默认标志的数据(空片子的情况)
#endif
#define UPDATE_FLAG_DATA      0xEEEE   //下载标志的数据
#define UPLOAD_FLAG_DATA      0xDDDD   //上传标志的数据
#define ERASE_FLAG_DATA       0xCCCC   //擦除标志的数据
#define APPRUN_FLAG_DATA      0x5A5A   //APP不需要做任何处理，直接运行状态
#define FINISI_INTO_BOOT_DATA	0x3131   //进入boot时清空程序正确标志位

#define POWER_ON_DATA				0x0001
#define POWER_OFF_DATA			0x0000

#define POWER_OFF_ISOK			0x0001
#define POWER_OFF_NOTOK			0x0000

/* IAP 中保存的参数------------------------------------------------ */

#define IAP_SETTING_SIZE	100
typedef enum
{
	SKIP,
	IAP_VER_DEVICE_ID,
	IAP_WORK_TIMES,
	IAP_JUMPFUN_ADDR = 90, 	//IAP跳转
	FINISH_FLAG_ADDR,			//程序写入完整
	FINISH_RUNNING_ADDR,	//程序运行成功
	POWERON_ADDR,					//开关机标志位
	POWEROFISOK						//正常关机标志位						
}iapUserSettingsDef;

typedef enum
{
	IAP_WRITE_RUNNING,
	IAP_WRITE_POWER,
	IAP_WRITE_ALL
}iapWriteFunDef;

extern int32_t  iapUserSettings[IAP_SETTING_SIZE];

/* IAP command------------------------------------------------ */
#define IAP_FLAG_ADDR   		(uint32_t)(ApplicationAddress - 1024 * 2)//App区域和Bootloader区域共享信息的地址(暂定大小为2K)
#define IAP_FLAG_ADDR_NEW   (uint32_t)(ApplicationAddress - 1024 * 2 + 1024)
#define SUCC_RUNNING_DATA		IAP_TOOL_NUM

#define IAP_FLASH_SIZE  0x3000//Bootloader区域大小
#define ApplicationAddress  (STM32_FLASH_BASE + IAP_FLASH_SIZE)//应用程序地址

void IAP_JUMP_WriteFlag(int32_t flag);//跳转时要写进rom的标志位
void IAP_ReadALLFlag(void); //上电读取所有IAP标志位到ram
void IAP_FUN_WriteFlag(uint8_t func,int32_t flag1,int32_t flag2);
int32_t IAP_ReadSingleFlag(uint8_t addr);



void IAP_Init(void);
void IAP_Handle(u8 * cmd);
#endif
