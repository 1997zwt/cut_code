/**
  ******************************************************************************
  * @file    log.h
  * @author  lyq
  * @version V1.0.0
  * @date    2025-02-24
  * @brief   该程序实现日志的查询、读取、存储、数据结构等
  * 程序RTC时钟采用的内部LSI时钟源
  * 开机后未校时如果工具关机，则本次开关机信息会丢失。
  * 
  * 2025.4.14
  * 增加了铁电存储器，norflash全部用于存储日志，fram用于存储日志索引、最新日志信息等。
  ******************************************************************************
  * @copy
  */
#ifndef _LOG_H
#define _LOG_H

#include "../user_tasks.h"
#include "time.h"

// #define RTC_CLOCK_SOURCE_LSE //外部晶振
// #define RTC_CLOCK_SOURCE_LSI //内部40K晶振
#define RTC_CLOCK_SOURCE_HSE //内部高速晶振62500

typedef struct
{
	uint8_t year;	
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} stClodkDef;

//存储旧日期的日志关键信息
//8字节，一页可存储32天信息
typedef struct
{
	uint32_t date;		 //日期
	uint16_t first_page; //起始页码
	uint16_t last_page;	 //最新页码
} stPageDateIndexdef;

//存储最新日期的地址索引
typedef struct //日志地址索引
{
	stPageDateIndexdef this_page;
	uint32_t counts;
	uint32_t log_index;	   //当前页的日志索引
	uint32_t run_tm_this;  //本次运行时间
	uint32_t run_tm_day;  //当天运行时间
	uint32_t run_tm_total; //总运行时间
	uint32_t pwr_on_cnts; //总开机次数
}stNewLogDef;

#define LOG_TYPE_COMMAND		0xAA01
#define LOG_TYPE_POWER			0xAA02	
#define LOG_TYPE_FAULT			0xAA03
#define LOG_TYPE_INFO			0xAA04

//定义扇区编号
#define BLOCK_BEGIN_PAGE_ID 0  //norflash全部用于存储日志

//fram地址10开始存储日志索引表，全表存储，每个日志索引表占32字节，可以存储32天，占用1024字节
//其中0-30用于存储旧日志信息，按照日期存储
//31用于存储最新的日志索引
#define PAGE_LOG_TABLE USER_LOG_SAVE_BASE			
#define PAGE_NEW_LOG_INFO USER_LOG_SAVE_BASE + 992 //第3页的最后32个字节
#define PAGE_THIS_LOG	USER_LOG_SAVE_BASE + 1024 //日志必须满一页才能存，否则后面就存储不了了，因此不满一页的日志关机时暂存在这里

#define PAGE_PER_BLOCK	16

#define MAX_BLOCK_INDEX	2000
#define MAX_PAGE_INDEX MAX_BLOCK_INDEX *PAGE_PER_BLOCK

typedef struct
{
	uint16_t type;	//日志类型
	stClodkDef tm; //发生时间
	uint8_t log_info[8];//日志信息
} stLogDataDef;

//电源信息日志
typedef struct
{
	uint32_t flg;	  //标志：1-程序升级，2-程序启动，3-关机信息，4-电池电压低,5-程序重启
	uint32_t bat_vol; //电池电压
} stPowerLogDef;

//故障信息日志
typedef struct
{
	uint8_t module;		//模块代码
	uint8_t fault;		//故障码
	uint8_t tool_state; //工具状态
	uint8_t info_0;		//可变参数0
	int16_t info_1;		//可变参数1，与故障有关
	int16_t bat_vol;	//电池电压
} stFaultLogDef;

//常规信息日志
typedef struct
{
	uint8_t info_code;		//编码
	uint8_t tool_state; 	//工具状态
	int16_t param_0;		//
	int16_t param_1;		//
	int16_t param_2;		//
} stNomalInfoLogDef;

//命令信息日志
typedef struct
{
	uint8_t tool;		  //工具类型
	uint8_t obj;		  //控制对象
	uint8_t cmd;		  //命令
	uint8_t cmd1;	 	 //附加命令,比如修改参数时的地址
	uint32_t param;	  	 //参数，没有填0
} stCmdLogDef;

typedef struct 
{
	uint32_t date;
	uint32_t this_page;
	uint32_t start_page;
	uint32_t end_page;
	uint32_t log_index;
}stQueryDef;

//命令信息日志
typedef union 
{
	stPowerLogDef power_info;
	stFaultLogDef fault_info;
	stCmdLogDef cmd_info;
	stNomalInfoLogDef nomal_info;
	uint8_t buf[8]; 
}unLogInfoDef;

extern stFaultLogDef gFault;
extern stNomalInfoLogDef gNomalInfo;
void Log_init(void);
void PVD_Init(void);
void RTC_SetTime(uint8_t *clock);
void Log_RecordPowerInfo(uint8_t flg, uint16_t voltage, uint8_t tm_offset);
void Log_RecordFaultInfo(stFaultLogDef *fault);
void Log_RecordNomalInfo(stNomalInfoLogDef *info);
void Log_RecordCmdInfo(uint8_t *cmd);
void Log_QueryDate(uint8_t year, uint8_t month, uint8_t day);
void Log_QueryPage(uint16_t begin, uint16_t end);
void Log_GetSystemInfo(void);
void Log_GetSystemClock(void);
int16_t Log_SendData(void);
void Log_Debug(uint8_t param);
#endif
