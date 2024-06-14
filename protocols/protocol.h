/**
  ******************************************************************************
  * @file    protocol.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-06-11
  * @brief   第3代接线工具协议
  ******************************************************************************
  * @copy
  */
#ifndef		_PROTOCOL_3_H
#define 	_PROTOCOL_3_H

#include "stm32f10x.h"

#define		PRO_HEAD0	0xAA
#define		PRO_HEAD1 0x55

#define TOOL_TYPE_MANUL		0x80		//最高位为1表示手动工具
#define TOOL_TYPE_AUTO			0x00		//最高位为0表示自动工具

//串口通信模式定义
#define PROTOCOL_MODE				0		//协议模式
#define PASS_THROUTH_MODE		1		//透传模式

//定义工具类型
typedef enum
{
	TOOL_TYPE_CUT = 1,									//断线工具
	TOOL_TYPE_STRIP ,									//剥线工具
	TOOL_TYPE_GRIPPER ,								//电动夹爪
	TOOL_TYPE_CUTTER_REGULATOR,		//电动调刀
	TOOL_TYPE_WIRING_BX, 							//并沟线夹接线工具
	TOOL_TYPE_WIRING_JX, 							//J型线夹接线工具
	TOOL_TYPE_FAULT_INDICATOR,				//故障指示器安装工具
	TOOL_TYPE_SETMODE=0xFD,					//设置串口通信模式
	TOOL_TYPE_UPDATE=0xFE,						//bootloader升级程序
	TOOL_TYPE_SETTINGS=0xFF
}ToolTypeDef;

//定义工具电机编号，最多支持8个电机
//建议在工具头文件中重定义
typedef enum
{
	TOOL_MOTOR_1,
	TOOL_MOTOR_2,
	TOOL_MOTOR_3,
	TOOL_MOTOR_4,
	TOOL_MOTOR_5,
	TOOL_MOTOR_6,
	TOOL_MOTOR_7,
	TOOL_MOTOR_8
}ToolMotorDef;

//定义电机通用命令
//工具命令建议在工具同文件中重定义
typedef enum
{
	MOTOR_CMD_STOP,		//停止
	MOTOR_CMD_WORK,		//正转，使用配置参数
	MOTOR_CMD_HOME,		//反转，使用配置参数
	MOTOR_CMD_CW,			//正转，带参数
	MOTOR_CMD_CCW,		//反转，带参数
	MOTOR_CMD_REPORT,	//状态主动反馈标志5
	MOTOR_CMD_GET,			//电机状态查询命令
	MOTOR_CMD_SET,			//电机参数设置命令
	MOTOR_CMD_AUTO,
	MOTOR_CMD_UNLOCK,			//强制解锁
	MOTOR_CMD_USER1=100,		//自定义,强制复位
	MOTOR_CMD_USER2				//自定义，电调校准
}ToolMotorCommandDef;

//通用指令
#define DEVICE_CMD_INFO		200		//查询设备信息
#define DEVICE_CMD_UPGRADE	201		//固件升级
#define DEVICE_SET_NO			202		//设置出厂编号

//定义通用工具命令
typedef enum
{
	TOOL_CMD_STOP,			//急停
	TOOL_CMD_READY,		//准备
	TOOL_CMD_WORK,			//开始工作
	TOOL_CMD_HOME,			//一键复位3
	TOOL_CMD_REPORT=5,	//状态主动反馈标志
	TOOL_CMD_GET,			//工具查询命令
	TOOL_CMD_SET,			//工具参数设置命令
	TOOL_CMD_PAUSE,			//暂停8
	TOOL_CMD_REDO,			//重新开始
	TOOL_CMD_RESET,		//重启
	TOOL_CMD_SHUT,			//关机
	TOOL_CMD_UNLOCK,		//强制解锁12
	TOOL_CMD_HOLD_LINE,		//旋转锁线
	TOOL_CMD_FACTORY_RESET,	//恢复出厂设置
	TOOL_CMD_USER1=100		//自定义
}ToolCommandDef;

//定义命令反馈
typedef enum
{
	RT_OK,
	RT_TOOL_LOW_BAT,		//电池电量低
	RT_TOOL_WORKING,		//工具正在工作2
	RT_TOOL_NOT_READY,		//工具未准备好
	RT_TOOL_NOT_MANUL,		//工具非手动控制模式
	RT_TOOL_NOT_HOME,		//工具未复位5
	RT_SRTIP_NOT_HOME,		//剥线旋转未复位
	RT_SLIDER_NOT_HOME,		//滑台未复位	
	RT_TOOL_NOT_PAUSE,		//断线工具工作未确认（工作暂停后才能继续断线）
	RT_MOTOR_NOT_MANUL=10,	//电机非手动控制模式
	RT_MOTOR_NOT_STOP,		//电机未停止
	RT_MOTOR_IS_LIMIT,		//电机已在限位位置12
	RT_MOTOR_IS_HOME,		//电机已在零位位置
	RT_CAMERA_ON_LOW_BAT,		//摄像头已开启
	RT_CAMERA_OFF_LOW_BAT		//摄像头已关闭
}CommandReturn;
//定义电机通用状态
//定义运动状态
typedef enum
{
	MOTION_STATE_STOPPED,
	MOTION_STATE_CW,//正转
	MOTION_STATE_CCW,
	MOTION_STATE_ERROR
}MotorMotionStateDef;
//定义工作状态
typedef enum
{
	WORK_STATE_IDLE,
	WORK_STATE_WORK, //动作或锁定
	WORK_STATE_HOME,  //复位或解锁
	WORK_STATE_ERROR
}MotorWorkStateDef;
//定义通用工具状态
typedef enum
{
	TOOL_STATE_READY,			//准备好
	TOOL_STATE_WORK,				//工作中
	TOOL_STATE_SUCCEED,		//工作成功
	TOOL_STATE_FAILURE,			//工作失败
	TOOL_STATE_MANUL,			//手动控制中4
	TOOL_STATE_HOMED,			//复位完成
	TOOL_STATE_HOMING,			//复位中
	TOOL_STATE_PREPARING,		//准备中
	TOOL_STATE_ERROR1,			//主线抱紧未抱线
	TOOL_STATE_PAUSE,			//暂停
	TOOL_STATE_STOPPING,	//停止过程中10
	TOOL_STATE_ON_HOLD,		//旋转锁线过程中
	TOOL_STATE_HOLD_END,	//旋转锁线完成
	TOOL_STATE_UNLOCKING,	//解锁中13
	TOOL_STATE_UNLOCKED,	//解锁成功14
	TOOL_STATE_HOMEFAILURE,
	TOOL_STATE_MIDDLE_0=100,	//中间自定义状态
	TOOL_STATE_MIDDLE_1
}ToolStateDef;


typedef enum
{
	MODULE_STATE_UNINIT,		//未初始化
	MODULE_STATE_NOMAL,			//非自动状态
	MODULE_STATE_WORKING,		//工作中  
	MODULE_STATE_HOMING,		//复位中
	MODULE_STATE_P_LIM,				//软件位置正限位
	MODULE_STATE_N_LIM,			//软件位置负限位
	MODULE_STATE_HOME,			//传感器或堵转零位6
	MODULE_STATE_MAX,				//传感器或工作极限7
	MODULE_STATE_ERROR,			//模块错误
	MODULE_STATE_POS_REACH		//模块位置控制到位
}ModuleStateDef;

typedef struct
{
	unsigned char	motion_state:2;				//运动状态：0停止，1正转，2反转，3错误
	unsigned char	work_state:2;					//工作状态：0空闲，1动作，2复位，3错误
	unsigned char	:4;									//预留
}ToolMotorStateDef;

extern uint8_t gTransMode;
#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
