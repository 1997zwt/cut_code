/**
  ******************************************************************************
  * @file    user_settings.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-05-06
  * @brief   用户设置数据的读取、写入、存储
					2020.5.12 增加了工具类型定义
					2020.5.26 增加了电动调刀的定义
					2020.5.27 修改了剥线工具的控制逻辑地址定义
					2020.6.18 修改了协议，长度字节代表数据个数而不是字节个数
  ******************************************************************************
  * @copy
  */
#ifndef		_USER_SETTINGS_H
#define 	_USER_SETTINGS_H

#include "stm32f10x.h"
#include "user_define.h"
#include "drv/drv_flash.h"
#include "drv/drivers_cfg.h"

//////////////////////////////////////////////////////////////

#define		PRO_HEAD0	0xAA
#define		PRO_HEAD1 0x55

typedef enum
{
	USER_FLASH_WRITE=1,
	USER_FLASH_WRITE_N,//连续写
	USER_FLASH_WRITE_SUPER,//超级写权限，可写只读地址
	USER_FLASH_READ,
	USER_FLASH_READ_N,//连续读
	USER_FLASH_DOWNLOAD,
	USER_FLASH_LOAD,
	USER_FLASH_SAVE
}enUserCfgCommands,UserCfgCommandsDef;
//定义用户读写FLASH区的首地址
#define  USER_FLASH_SAVE_BASE		STM32_FLASH_BASE+(STM32_FLASH_SIZE-1)*1024

typedef enum
{
	FLG_SAVE,				//参数配置过
	FLG_LOAD	,			//参数加载过
	
	FLG_PROTOCOL_VER=16, //2第二版协议，3第3版协议
	FLG_TOOL_TYPE=24	//最高8位表示工具类型,第31位表示手动或自动工具，1手动，0自动
}enSysFlgs,SysFlgDef;

typedef enum
{
	SYS_FLGS,					//保存系统的一些标志位
	//版本控制2-10
	VER_DEVICE_ID,	//设备出厂编号
	VER_SOFT,				//软件版本
	VER_SOFT_DATE,	//软件修改日期
	VER_SETTINGS,		//参数版本
	VER_SETTINGS_DATE,	//参数修改日期，
	VER_MODIFY_TIMES,	//参数修改次数

	
	GRIP_MOVE_SPEED=10,				//夹紧电机速度(rpm)
	GRIP_WORK_CURRENT,		//夹紧电机夹紧关断电流(mA)
	GRIP_HOME_CURRENT,			//夹紧电机复位关断电流(mA)
	GRIP_MOVE_OVERTIME,		//夹紧电机超时时间(ms)
	GRIP_POSITIVE_LIMIT,			//夹紧电机正位置限位(mm)
	GRIP_NEGATIVE_LIMIT,			 //夹紧电机负位置限位(mm)
	
	//断线工具设置11-20
	CUT_ENABLE=80,	
	CUT_HOME_SPEED,		//复位速度    81

	//以下是自动断线流程的设置
	//完整动作如下：低速运动直到接触导线->加速运动一段时间完成断线->断线完成，电流降低，结束
	CUT_CUT_SPEED_1,		//断线第一段速度   82
	CUT_CUT_TIME_1,			//断线第一段时间
	CUT_CUT_CURRENT_1,	//断线第一段截止电流   84
	CUT_CUT_SPEED_2,		//断线第二段速度
	CUT_CUT_TIME_2,			//断线第二段时间   
	CUT_CUT_CURRENT_2,	//断线第二段截止电流
	CUT_CUT_TIME_3,			//断线第二段时间   

	WIFI_NET_CONFIG = 95,
	WORK_TIMES= 96,
	POWER_ON_FLG=97,//重启标志	
	POWER_LOW_LIMIT=98,			//低电压限幅
	USER_SETTING_CHECK=99,	//校验
	USER_SETTING_SIZE
}enUserSettings,UserSettingsDef;

void InitUserSettings(uint8_t flg);
int32_t ReadUserSettings(uint8_t addr);
void ConfigUserSettings(uint8_t cmd, uint8_t* data);
void WriteUserSettings(uint8_t addr, int32_t val);
void SaveUserSettings(void);
#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
