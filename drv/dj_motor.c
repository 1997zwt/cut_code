#include "dj_motor.h"

#if (defined DJ_MOTOR_NUM) && (DJ_MOTOR_NUM > 0)

//电机列表
static stDJMotorDef* gServoMotors[DJ_MOTOR_NUM];
//电机控制字列表
int32_t gMotorCtrlWord[DJ_MOTOR_NUM][SIZE_OF_CTRL_WORD] = {0};
static uint8_t gMotorIndex = 0;
uint32_t gGlobalTimer =  0;

OS_TMR *gTmr_1ms = NULL;

void exSetMotorControlWord(stDJMotorDef* motor, enMotorCrtlWord addr, int32_t val);
void _MotorControl_callback(void *ptmr, void *parg);
//以下为私有函数
static void _OsTimerConfig(void)
{
    uint8_t err;

	if (gTmr_1ms == NULL)
	{
		gTmr_1ms = OSTmrCreate(100, 1, OS_TMR_OPT_PERIODIC, _MotorControl_callback, NULL, NULL, &err);
		OSTmrStart(gTmr_1ms, &err);
	}
}
void _SetMotorState(stDJMotorDef* motor, uint8_t bit, bool val)
{
	if(val)
		gMotorCtrlWord[motor->motor_index][MOTOR_STATE] |= (1<<bit);
	else
		gMotorCtrlWord[motor->motor_index][MOTOR_STATE] &= (~(1<<bit));
}
void _SetMotorError(stDJMotorDef* motor, uint8_t bit, bool err)
{
	if (err)
	{
		gMotorCtrlWord[motor->motor_index][MOTOR_ERROR] |= (1<<bit);
	}
	else
	{
		gMotorCtrlWord[motor->motor_index][MOTOR_ERROR] &= (~(1<<bit));
	}
}
uint32_t _GetMotorState(stDJMotorDef* motor, uint8_t bit)
{
	return (uint32_t)gMotorCtrlWord[motor->motor_index][MOTOR_STATE]&((uint32_t)(1<<bit));
}

static void _SetMotorPWM(stDJMotorDef* motor, int16_t DutyRadio)
{
	DJ_SetMotorVoltage(motor->can_id,DutyRadio*motor->motor_desp.rotate_dir);
	exSetMotorControlWord(motor,OLP_ACTUAL_PWM,DutyRadio);
}
//直线加速方法
void _LinearAcceleration(int32_t  acc,int32_t target_val,int32_t* current_val)
{
	int32_t current;

	current = *current_val;
	acc /= 1000;  //每毫秒的增量

	if (ABS(current-target_val) < acc)
	{
		current = target_val;
	}
	else
	{
		if (current < target_val)
		{
			current += acc;
		}
		else if (current > target_val)
		{
			current -= acc;
		}
	}
	*current_val = current;
}
static void _Position_Loop_Control(stDJMotorDef* motor)
{
	int32_t  target_position = 0;
	int32_t  actual_position = 0;
	int32_t  profile_position = 0;
	int32_t  max_velocity = 0;
	int32_t err = 0;
	int32_t pid_out = 0;

	actual_position = DJ_GetMotorAngle(motor->can_id);
	exSetMotorControlWord(motor,PLP_ACTUAL_VAL,actual_position);

	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))	return;
	if(!_GetMotorState(motor,STA_P_LOOP))  return;

	target_position = exGetMotorControlWord(motor,PLP_TARGET_VAL);
	profile_position = exGetMotorControlWord(motor,PLP_PROFILED_VAL);
	max_velocity = (exGetMotorControlWord(motor,PLP_MAX_VEL)<<11)/15;
	_LinearAcceleration(max_velocity,target_position,&profile_position);
	err = profile_position - actual_position;
	exSetMotorControlWord(motor,PLP_PROFILED_VAL,profile_position);
	exSetMotorControlWord(motor,PLP_ERROR,err);
	pid_out = iPosPIDCal(&(motor->pids[PID_P]),err)>>10;
	if (ABS(err) < 1000)
	{
		pid_out = 0;
	}
	Limit(pid_out,exGetMotorControlWord(motor,PLP_MAX_VEL));
	exSetMotorControlWord(motor,VLP_TARGET_VAL,pid_out);

	return;
}

static void _Velocity_Loop_Control(stDJMotorDef* motor)
{	
	int32_t  target_velocity = 0;//目标速度
	int32_t  actual_velocity = 0;//当前速度
	int32_t  profile_velocity = 0;
	int32_t acc = 0;
	int32_t err = 0;
	int32_t pid_out = 0;

	//速度采样
	actual_velocity = DJ_GetMotorSpeed(motor->can_id)*motor->motor_desp.rotate_dir;
	exSetMotorControlWord(motor,VLP_ACTUAL_VAL,actual_velocity);

	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))	return;
	if(!_GetMotorState(motor,STA_V_LOOP))  return;
	//速度闭环控制
	target_velocity = exGetMotorControlWord(motor,VLP_TARGET_VAL);
	profile_velocity = exGetMotorControlWord(motor,VLP_PROFILED_VAL);
	acc = exGetMotorControlWord(motor,VLP_ACC_LIMIT);
	_LinearAcceleration(acc,target_velocity,&profile_velocity);

	exSetMotorControlWord(motor,VLP_PROFILED_VAL,profile_velocity);
	err = profile_velocity - actual_velocity;
	exSetMotorControlWord(motor,VLP_ERROR,err);
	pid_out = iPosPIDCal(&(motor->pids[PID_V]),err)>>10;
	if (ABS(target_velocity) < 10)
	{
		target_velocity = 0;
		profile_velocity = 0;
		pid_out = 0;
	}
	if(ABS(profile_velocity) < 10)
	{
		motor->pids[PID_V].ei >>=1;
	}
	_SetMotorPWM(motor,pid_out);
//	if (ABS(pid_out) > DJ_MOTOR_VOLTAGE_LIMIT && actual_velocity == 0)
//	{
//		DJ_Motor_QuickStop(motor);
//	}

	return;
}
static void _Current_Loop_Control(stDJMotorDef* motor)
{
	int32_t  actual_current = 0;

	if(motor->type == DJ_TYPE_2610)
		actual_current = DJ_GetMotorCurrent(motor->can_id)>>2;
	if(motor->type == DJ_TYPE_2620)
		actual_current = DJ_GetMotorCurrent(motor->can_id)/5;
	//电流采样
	exSetMotorControlWord(motor,CLP_ACTUAL_VAL,actual_current);
	//低通滤波计算平均电流
	actual_current = ((actual_current<<3) + (exGetMotorControlWord(motor,CLP_CONTINUOUS_VAL)<<1))/10;
	exSetMotorControlWord(motor,CLP_CONTINUOUS_VAL,actual_current);		
//	if (ABS(actual_current) > motor->motor_desp.current_max_ma)//电流超限幅报警
//	{
//		_SetMotorState(motor,STA_ERROR,TRUE);
//		_SetMotorError(motor,ERR_CURRENT_OVERLOAD,TRUE);
//		_SetMotorPWM(motor,0);
//	}
	if (_GetMotorState(motor,STA_TIME_CONTROL))
	{
		if(gMotorCtrlWord[motor->motor_index][MOTOR_TM1]++ > exGetMotorControlWord(motor,OLP_MOVE_TIME))
		{
			DJ_Motor_QuickStop(motor);
			exSetMotorControlWord(motor,MOTOR_TM1,0);
			_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
			_SetMotorState(motor,STA_TIME_REACH,TRUE);
		}
	}
	else
	{
		exSetMotorControlWord(motor,MOTOR_TM1,0);
	}
	if (_GetMotorState(motor,STA_CURRENT_SHUT_CONTROL))
	{
		if (ABS(actual_current) > exGetMotorControlWord(motor,CLP_LIMIT))
		{
			DJ_Motor_QuickStop(motor);
			_SetMotorState(motor,STA_CURRENT_REACH,TRUE);	
			_SetMotorState(motor,STA_CURRENT_SHUT_CONTROL,FALSE);	
		}
	}
	
	return;
}
static void _MotorControl_callback(void *ptmr, void *parg)
{
	for (;gMotorIndex<DJ_MOTOR_NUM;gMotorIndex++)
	{
		if (!_GetMotorState(gServoMotors[gMotorIndex],STA_SOFT_ENABLE))
		{
			continue;
		}
		_Position_Loop_Control(gServoMotors[gMotorIndex]);
		_Velocity_Loop_Control(gServoMotors[gMotorIndex]);
		_Current_Loop_Control(gServoMotors[gMotorIndex]);

		gServoMotors[gMotorIndex]->error = exGetMotorControlWord(gServoMotors[gMotorIndex],MOTOR_ERROR);
		gServoMotors[gMotorIndex]->state = exGetMotorControlWord(gServoMotors[gMotorIndex],MOTOR_STATE);
	}
	DJ_UpdateCanMotors();
	gMotorIndex = 0;
	gGlobalTimer++;
}
//以下为接口函数
void DJ_Motor_Default(stDJMotorDef* motor)
{
	memset(motor,0,sizeof(stDJMotorDef));	
}

bool DJ_Motor_Init(stDJMotorDef* motor)
{
	if(motor->motor_index < DJ_MOTOR_NUM)
		gServoMotors[motor->motor_index] = motor;
	else
		return FALSE;		

	/**************************************/
	/*以下数据不对用户开放，但是可根据需要进行修改*/	
	exSetMotorControlWord(motor,CLP_LIMIT,motor->motor_desp.current_max_ma);
	exSetMotorControlWord(motor,VLP_VEL_LIMIT,motor->motor_desp.velocity_max_rpm);
	exSetMotorControlWord(motor,VLP_ACC_LIMIT,motor->motor_desp.acc_max);
	exSetMotorControlWord(motor,PLP_MAX_POS,motor->motor_desp.posotion_positive_max);
	exSetMotorControlWord(motor,PLP_MIN_POS,motor->motor_desp.posotion_negative_max);

	if (motor->ctrl_mode&MODE_CURRENT_LOOP)
	{
		exSetMotorControlWord(motor,CLP_ENABLE,TRUE);
		_SetMotorState(motor,STA_I_LOOP,TRUE);
	}
	if (motor->ctrl_mode&MODE_VELOCITY_LOOP)
	{
		exSetMotorControlWord(motor,VLP_ENABLE,TRUE);
		_SetMotorState(motor,STA_V_LOOP,TRUE);
	}

	if (motor->ctrl_mode&MODE_POSITION_LOOP)
	{
		exSetMotorControlWord(motor,PLP_ENABLE,TRUE);
		_SetMotorState(motor,STA_P_LOOP,TRUE);
	}
	
	_OsTimerConfig();

	return TRUE;
}

void DJ_Motor_Enable(stDJMotorDef* motor, bool val)
{
	_SetMotorState(motor, STA_SOFT_ENABLE,val);
	_SetMotorState(motor, STA_ERROR,FALSE);
	gMotorCtrlWord[motor->motor_index][MOTOR_ERROR] = 0;
}

void DJ_Motor_SetTargetVelocity(stDJMotorDef* motor, int16_t velocity_rpm)
{
	_SetMotorState(motor,STA_V_LOOP,TRUE);
	velocity_rpm = velocity_rpm;//*motor->motor_desp.rotate_dir;
	Limit(velocity_rpm, exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,VLP_TARGET_VAL,velocity_rpm);
	_SetMotorState(motor,STA_SPEED_REACH,FALSE);
}
void DJ_Motor_QuickStop(stDJMotorDef* motor)
{
	exSetMotorControlWord(motor,PLP_TARGET_VAL,exGetMotorControlWord(motor,PLP_ACTUAL_VAL));
	exSetMotorControlWord(motor,PLP_PROFILED_VAL,exGetMotorControlWord(motor,PLP_ACTUAL_VAL));
	exSetMotorControlWord(motor,PLP_MAX_VEL,0);
	exSetMotorControlWord(motor,VLP_TARGET_VAL,0);
	exSetMotorControlWord(motor,VLP_PROFILED_VAL,0);
	exSetMotorControlWord(motor,OLP_ACTUAL_PWM,0);
	_SetMotorPWM(motor,0);
	DJ_Motor_ClearState(motor,STA_P_LOOP);
	DJ_Motor_ClearState(motor,STA_V_LOOP);
}
void DJ_Motor_SetTargetPosition(stDJMotorDef* motor, int16_t vel_limit, int32_t position)
{
	_SetMotorState(motor,STA_P_LOOP,TRUE);
	_SetMotorState(motor,STA_V_LOOP,TRUE);
	Range(position, exGetMotorControlWord(motor,PLP_MIN_POS), exGetMotorControlWord(motor,PLP_MAX_POS));
	Limit(vel_limit,exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,PLP_MAX_VEL,vel_limit);
	exSetMotorControlWord(motor,PLP_TARGET_VAL,position);
	_SetMotorState(motor,STA_POSITION_REACH,FALSE);
}

void DJ_Motor_SetRelativePosition(stDJMotorDef* motor, int16_t vel_limit, int32_t position)
{
	_SetMotorState(motor,STA_P_LOOP,TRUE);
	_SetMotorState(motor,STA_V_LOOP,TRUE);
	position = exGetMotorControlWord(motor,PLP_ACTUAL_VAL) + position;
	Range(position, exGetMotorControlWord(motor,PLP_MIN_POS), exGetMotorControlWord(motor,PLP_MAX_POS));
	Limit(vel_limit,exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,PLP_MAX_VEL,vel_limit);
	exSetMotorControlWord(motor,PLP_TARGET_VAL,position);
	_SetMotorState(motor,STA_POSITION_REACH,FALSE);
}

void DJ_Motor_ClearState(stDJMotorDef* motor, uint8_t bit)
{
	_SetMotorState(motor,bit,FALSE);
}
void exSetMotorControlWord(stDJMotorDef* motor, enMotorCrtlWord addr, int32_t val)
{
	gMotorCtrlWord[motor->motor_index][addr] = val;
}
int32_t exGetMotorControlWord(stDJMotorDef* motor, enMotorCrtlWord addr)
{
	return gMotorCtrlWord[motor->motor_index][addr];
}

#endif
