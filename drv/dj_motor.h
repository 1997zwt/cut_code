/**
  ******************************************************************************
  * @file    dj_motor.h
  * @author  lyq
  * @version V1.0.0
  * @date    2021-01-06
  * @brief   大疆CAN总线电机的驱动程序。
  ******************************************************************************
  * @copy
  */
#ifndef	 _DJ_MOTOR_H
#define  _DJ_MOTOR_H

#include "stm32f10x.h"
#include "drv_motocan.h"
#include "drivers_cfg.h"
#include "PID.h"
#include <string.h>

#define DJ_TYPE_2610		1
#define DJ_TYPE_2620		2

/***************直流伺服电机**************/
//使能引脚的极性
typedef enum
{
	NO_ENABLE_PIN,		//没有使能引脚
	LOW_ENABLE,
	HIGH_ENABLE
}enEnablePolarity;

//方向引脚的极性
typedef enum
{
	NO_DIR_PIN,		//没有方向引脚
	LOW_CW,
	HIGH_CW
}enDirectionPolarity;

/***************直流伺服电机**************/
//旋转方向定义
typedef enum
{
	DIR_CW = 1,
	DIR_CCW = -1
}enDirectionDef;

//PID参数编号
typedef enum
{
	PID_I,
	PID_V,
	PID_P
}enPIDIndex;

//控制模式
typedef enum
{
	MODE_OPEN_LOOP,
	MODE_CURRENT_LOOP=1,
	MODE_VELOCITY_LOOP=2,
	MODE_POSITION_LOOP=4
}enCtrlModeDef;

//全局错误标志
//值越低，优先级越高
//低7位的错误在清除之前驱动器将停止工作
typedef enum
{
	ERR_NO_PWM_FUN,						//未定义PWM接口函数
    ERR_CURRENT_OVERLOAD=1,			//过流
	ERR_OVER_SPEED,							//速度超限
	ERR_OVER_NPOS,							//负位置超限
	ERR_OVER_PPOS,								//正位置超限
		ERR_ILOOP_SATURATION, //电流环积分饱和
	ERR_VLOOP_SATURATION  //速度环积分饱和
}enErrCodeDef;
//全局状态标志
typedef enum
{
	STA_I_LOOP,						//电流环使能
	STA_V_LOOP,					//速度环使能
	STA_P_LOOP,					//位置环使能,如果前三位为0则处于开环状态
    
	STA_HAVE_I_SENSOR=4,				//有电流反馈
	STA_HAVE_V_SENSOR,					//有速度反馈
	STA_HAVE_P_SENSOR,					//有位置反馈

	STA_TIME_CONTROL=8,		//时间控制模式
	STA_CURRENT_SHUT_CONTROL,	//电流关断模式

	STA_CURRENT_REACH=12,		//达到设定电流
	STA_SPEED_REACH,		  //达到设定速度			
	STA_POSITION_REACH,   //到达设定位置
	STA_TIME_REACH,			//达到指定时间

	STA_SOFT_ENABLE=28,		//软件使能
    STA_HARD_ENABLE,		//硬件使能
	STA_ERROR=31		//硬件错误
}enStateCodeDef;


//电机的控制引脚定义
typedef struct  
{
	uint16_t	 enable_pin;						//使能引脚
	GPIO_TypeDef *  enable_pin_port;				//使能引脚在哪个端口上
	enEnablePolarity		 enable_pin_polarity;		//使能引脚的极性（高电平使能OR低电平使能）		
	uint16_t	 dir_pin;						//方向引脚
	GPIO_TypeDef *  dir_pin_port;				//方向引脚在哪个端口上
	uint16_t	 dir_pin2;						//L298有两个方向引脚
	GPIO_TypeDef *  dir_pin2_port;				//方向引脚在哪个端口上
	enDirectionPolarity		dir_pin_polarity;		//方向引脚的极性（高电平正转OR低电平正转）		
}stMotorCtrlPinDef;


//电机控制参数
typedef enum
{
	MOTOR_STATE,
	MOTOR_ERROR,
	MOTOR_TM1,					//用于时间控制的计时
	MOTOR_TM2,					//用于平均电流计时
	OLP_TARGET_PWM,		//开环模式给定PWM
	OLP_ACTUAL_PWM,		//开环模式当前PWM
	OLP_STEP,							//开环PWM步长
	OLP_MOVE_TIME,			//开环运动时间(ms)

	CLP_ENABLE,						//电流环使能(0禁止/1使能) 8
	CLP_LIMIT,							//超限电流限制(mA)
	CLP_OVERLOAD_TIME,		//电流超限报警时间（ms，超过该时间报错）	
	CLP_CONTINUOUS_VAL,		//超限时间内的平均电流(mA)
	CLP_TARGET_VAL,				//目标电流(mA)
	CLP_ACTUAL_VAL,				//实际电流(mA)
	CLP_MOVE_TIME,				//电流运动时间(ms)
	CLP_ERROR,							//电流误差(mA)

	VLP_ENABLE ,						//速度环使能(0禁止/1使能) 16
	VLP_VEL_LIMIT,					//速度限制(rpm)
	VLP_ACC_LIMIT,					//加速度限制(rpm/s)
	VLP_TARGET_VAL,				//目标速度(rpm)
	VLP_PROFILED_VAL,			//给定速度，加速度限制后的目标速度(rpm)
	VLP_ACTUAL_VAL,				//实际速度(rpm)
	VLP_ERROR,							//速度误差(rpm)
    VLP_VEL_LIMIT_1,				//速度限制(cnt/s)
	VLP_ACC_LIMIT_1,				//加速度限制(cnt/ss)
	VLP_TARGET_VAL_1,			//目标速度(cnt/s)
	VLP_PROFILED_VAL_1,		//给定速度，加速度限制后的目标速度(cnt/s)
	VLP_ACTUAL_VAL_1,			//实际速度(cnt/s)
	VLP_ERROR_1,						//速度误差(cnt/s)

	PLP_ENABLE ,						//位置环使能(0禁止/1使能) 29
	PLP_MAX_VEL,						//位置环模式下的速度限制(cnt/s)	
	PLP_MAX_POS,						//最大位置限制(cnt)
	PLP_MIN_POS,						//最小位置限制(cnt)
	PLP_HOME_POS,					//零位位置(cnt)
	PLP_TARGET_VAL,					//目标位置(cnt)
	PLP_PROFILED_VAL,			//给定位置,加速度和速度限制后的目标位置(cnt)
	PLP_ACTUAL_VAL,				//实际位置(cnt)
	PLP_ERROR,							//位置误差(cnt)
	PLP_ACC_METHOD,				//加速方法（0-直线加速;1-S曲线加速）			
	PLP_S_CURVE_ACC,				//S曲线加速的加速度限制

	SIZE_OF_CTRL_WORD
}enMotorCrtlWord;

//电机的特征描述
typedef struct
{
	int32_t  current_max_ma;				//最大电流(毫安)
	int16_t  rotate_dir;						//旋转方向
	int16_t  velocity_max_rpm;			//最大速度(转每分)
	int16_t  acc_max;							//最大加速度
	int32_t  posotion_positive_max;	//最大正位置
	int32_t  posotion_negative_max;	//最大负位置
    int16_t  encoder_resolution;			//编码器线数
}stMotorDescription;



typedef struct  
{
	uint8_t	motor_index;					//电机序号，取值0~SERVO_MOTOR_NUM-1
	uint8_t can_id;
	uint8_t type;
	stMotorDescription	motor_desp;
	uint8_t ctrl_mode;
	uint32_t state;
	uint32_t error;
	istPID	pids[3];								//电流、速度、位置闭环的PID
}stDJMotorDef;
void DJ_Motor_Default(stDJMotorDef* motor);
bool DJ_Motor_Init(stDJMotorDef* motor);
//使能电机,可清除错误
void DJ_Motor_Enable(stDJMotorDef* motor, bool val);

//设置目标速度，用于速度环控制
//velocity_rpm:转每分转速
void DJ_Motor_SetTargetVelocity(stDJMotorDef* motor, int16_t velocity_rpm);

//设置绝对目标位置，用于位置环控制
//position:绝对目标位置，用户单位
void DJ_Motor_SetTargetPosition(stDJMotorDef* motor, int16_t vel_limit, int32_t position);

//设置相对目标位置，用于位置环控制
//position:相对目标位置，用户单位
void DJ_Motor_SetRelativePosition(stDJMotorDef* motor, int16_t vel_limit, int32_t position);

//清除状态位
void DJ_Motor_ClearState(stDJMotorDef* motor, uint8_t bit);

void DJ_Motor_QuickStop(stDJMotorDef* motor);

void exSetDJMotorControlWord(stDJMotorDef* motor, enMotorCrtlWord addr, int32_t val);
int32_t exGetDJMotorControlWord(stDJMotorDef* motor, enMotorCrtlWord addr);

typedef struct  
{
	uint8_t	motor_index;					//电机序号，取值0~SERVO_MOTOR_NUM-1
	stMotorDescription	motor_desp;
	stMotorCtrlPinDef	ctrl_pins;
	uint8_t ctrl_mode;
	uint32_t state;
	uint32_t error;
	istPID	pids[3];								//电流、速度、位置闭环的PID
	int16_t (*GetCurrentFun)(void);	//获取电流的函数指针
	int32_t (*GetVelocityFun)(void);	//获取速度的函数指针
	int32_t (*GetPositionFun)(void);	//获取位置的函数指针
	void (*SetPwmFun)(int16_t);		//设置PWM的函数指针
}stServoMotorDef;

void Servo_Motor_Default(stServoMotorDef* motor);
bool Servo_Motor_Init(stServoMotorDef* motor);
//使能电机,可清除错误
void Servo_Motor_Enable(stServoMotorDef* motor, bool val);

//设置开环速度，用于开环控制
//speed_rpm: 转每分转速
void Servo_Motor_SetOpenVelocity(stServoMotorDef* motor, int16_t velocity_rpm);

//开环速度时间模式，开环模式下指定运行时间
//speed_rpm: 转每分转速
//time_ms:运动时间(ms)
void Servo_Motor_SetOpenVelocityTime(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t time_ms);

//开环速度电流关断模式，开环模式下达到指定电流自动关断
//speed_rpm: 转每分转速
//current_ma:毫安电流
void Servo_Motor_SetOpenVelocityCurrent(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t current_ma);

//开环速度电流时间模式，开环模式下达到指定电流或指定时间自动关断，先到者为准
//speed_rpm: 转每分转速
//current_ma:毫安电流
//time_ms:运动时间(ms)
void Servo_Motor_SetOpenVelocityCurrentTime(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t current_ma, uint16_t time_ms);

//设置目标电流，用于电流环控制
//current_ma:毫安电流
void Servo_Motor_SetTargetCurrent(stServoMotorDef* motor, int16_t current_ma);

//电流时间模式，电流模式下指定运行时间
//current_ma:毫安电流
//time_ms:运动时间(ms)
void Servo_Motor_SetTargetCurrentTime(stServoMotorDef* motor, int16_t current_ma, uint16_t time_ms);

//设置目标速度，用于速度环控制
//velocity_rpm:转每分转速
void Servo_Motor_SetTargetVelocity(stServoMotorDef* motor, int16_t velocity_rpm);

//设置绝对目标位置，用于位置环控制
//position:绝对目标位置，用户单位
void Servo_Motor_SetTargetPosition(stServoMotorDef* motor, int16_t vel_limit, int32_t position);

//设置相对目标位置，用于位置环控制
//position:相对目标位置，用户单位
void Servo_Motor_SetRelativePosition(stServoMotorDef* motor, int16_t vel_limit, int32_t position);

//清除状态位
void Servo_Motor_ClearState(stServoMotorDef* motor, uint8_t bit);

void Servo_Motor_QuickStop(stServoMotorDef* motor);
//设置电机控制字，可实现高级功能
void exSetMotorControlWord(stServoMotorDef* motor, enMotorCrtlWord addr, int32_t val);
int32_t exGetMotorControlWord(stServoMotorDef* motor, enMotorCrtlWord addr);


#endif
