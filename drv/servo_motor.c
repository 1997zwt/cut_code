#include "dj_motor.h"

#if (defined SERVO_MOTOR_NUM) && (SERVO_MOTOR_NUM > 0)

//电机列表
static stServoMotorDef* gServoMotors[SERVO_MOTOR_NUM];
//电机控制字列表
int32_t gMotorCtrlWord[SERVO_MOTOR_NUM][SIZE_OF_CTRL_WORD] = {0};
static uint8_t gMotorIndex = 0;

OS_TMR *gTmr_1ms = NULL;

void exSetMotorControlWord(stServoMotorDef* motor, enMotorCrtlWord addr, int32_t val);
//以下为私有函数
void _MotorControl_callback(void *ptmr, void *parg);
static void _OsTimerConfig(void)
{
    uint8_t err;

	if (gTmr_1ms == NULL)
	{
		gTmr_1ms = OSTmrCreate(100, 1, OS_TMR_OPT_PERIODIC, _MotorControl_callback, NULL, NULL, &err);
		OSTmrStart(gTmr_1ms, &err);
	}
}

static void _SetMotorDirection(stServoMotorDef* motor, enDirectionDef dir)
{
	if (motor->ctrl_pins.dir_pin_polarity == NO_DIR_PIN)
		return;
	else if (motor->ctrl_pins.dir_pin_polarity == LOW_CW)
	{
		if (dir == DIR_CW)
		{
			GPIO_ResetBits(motor->ctrl_pins.dir_pin_port, motor->ctrl_pins.dir_pin);
			if (motor->ctrl_pins.dir_pin2_port)
			{
				GPIO_SetBits(motor->ctrl_pins.dir_pin2_port, motor->ctrl_pins.dir_pin2);
			}
		}
		else
		{
			GPIO_SetBits(motor->ctrl_pins.dir_pin_port, motor->ctrl_pins.dir_pin);
			if (motor->ctrl_pins.dir_pin2_port)
			{
				GPIO_ResetBits(motor->ctrl_pins.dir_pin2_port, motor->ctrl_pins.dir_pin2);
			}
		}
	}
	else if (motor->ctrl_pins.dir_pin_polarity == HIGH_CW)
	{
		if (dir == DIR_CW)
		{
			GPIO_SetBits(motor->ctrl_pins.dir_pin_port, motor->ctrl_pins.dir_pin);
			if (motor->ctrl_pins.dir_pin2_port)
			{
				GPIO_ResetBits(motor->ctrl_pins.dir_pin2_port, motor->ctrl_pins.dir_pin2);
			}
		}
		else
		{
			GPIO_ResetBits(motor->ctrl_pins.dir_pin_port, motor->ctrl_pins.dir_pin);
			if (motor->ctrl_pins.dir_pin2_port)
			{
				GPIO_SetBits(motor->ctrl_pins.dir_pin2_port, motor->ctrl_pins.dir_pin2);
			}
		}
	}
}

void _SetMotorState(stServoMotorDef* motor, uint8_t bit, bool val)
{
	if(val)
		gMotorCtrlWord[motor->motor_index][MOTOR_STATE] |= (1<<bit);
	else
		gMotorCtrlWord[motor->motor_index][MOTOR_STATE] &= (~(1<<bit));
}
void _SetMotorError(stServoMotorDef* motor, uint8_t bit, bool err)
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
uint32_t _GetMotorState(stServoMotorDef* motor, uint8_t bit)
{
	return (uint32_t)gMotorCtrlWord[motor->motor_index][MOTOR_STATE]&((uint32_t)(1<<bit));
}

static void _SetMotorPWM(stServoMotorDef* motor, int16_t DutyRadio)
{
	Limit(DutyRadio, PWM_DUTY_RANGE_DEF);

	if (DutyRadio > 0)
	{
		_SetMotorDirection(motor,DIR_CW);
		motor->SetPwmFun(DutyRadio);
	}
	else
	{
		_SetMotorDirection(motor,DIR_CCW);
		motor->SetPwmFun(-DutyRadio);
	}
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
static void _Position_Loop_Control(stServoMotorDef* motor)
{
	int32_t  target_position = 0;
	int32_t  actual_position = 0;
	int32_t  profile_position = 0;
	int32_t  max_velocity = 0;
	int32_t err = 0;
	int32_t pid_out = 0;

	if (_GetMotorState(motor,STA_HAVE_P_SENSOR))
	{
		actual_position = motor->GetPositionFun();
		exSetMotorControlWord(motor,PLP_ACTUAL_VAL,actual_position);
	}

	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))	return;
	if(!_GetMotorState(motor,STA_P_LOOP))  return;

	target_position = exGetMotorControlWord(motor,PLP_TARGET_VAL);
	profile_position = exGetMotorControlWord(motor,PLP_PROFILED_VAL);
	max_velocity = exGetMotorControlWord(motor,PLP_MAX_VEL);
	_LinearAcceleration(max_velocity,target_position,&profile_position);
	err = profile_position - actual_position;
	exSetMotorControlWord(motor,PLP_PROFILED_VAL,profile_position);
	exSetMotorControlWord(motor,PLP_ERROR,err);
	pid_out = iPosPIDCal(&(motor->pids[PID_P]),err)>>10;

	if (_GetMotorState(motor,STA_V_LOOP))
	{
		Limit(pid_out,exGetMotorControlWord(motor,PLP_MAX_VEL));
		exSetMotorControlWord(motor,VLP_TARGET_VAL_1,pid_out);
		exSetMotorControlWord(motor,VLP_TARGET_VAL,pid_out*15/motor->motor_desp.encoder_resolution);
	} 
	else	if (_GetMotorState(motor,STA_I_LOOP))//没有速度环，输出直接到电流环
	{
		Limit(pid_out,exGetMotorControlWord(motor,CLP_LIMIT));
		exSetMotorControlWord(motor,CLP_TARGET_VAL,pid_out);
	}
	else
	{
		Limit(pid_out,4000);
		if (ABS(err) < 200)
		{
			_SetMotorState(motor,STA_POSITION_REACH,TRUE);	
			pid_out = 0;
		}
		_SetMotorPWM(motor,pid_out);
	}

	return;
}

static void _Velocity_Loop_Control(stServoMotorDef* motor)
{	
	int32_t  target_velocity = 0;
	int32_t  actual_velocity = 0;
	int32_t  profile_velocity = 0;
	int32_t acc = 0;
	int32_t err = 0;
	int32_t pid_out = 0;

	//速度采样
	if (_GetMotorState(motor,STA_HAVE_V_SENSOR))
	{
		actual_velocity = motor->GetVelocityFun();//cnt/s
		exSetMotorControlWord(motor,VLP_ACTUAL_VAL_1,actual_velocity);
		exSetMotorControlWord(motor,VLP_ACTUAL_VAL,actual_velocity*15/motor->motor_desp.encoder_resolution);
	}

	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))	return;
	if(!_GetMotorState(motor,STA_V_LOOP))  return;
	//速度闭环控制
	target_velocity = exGetMotorControlWord(motor,VLP_TARGET_VAL_1);//cnt/s
	profile_velocity = exGetMotorControlWord(motor,VLP_PROFILED_VAL_1);
	acc = exGetMotorControlWord(motor,VLP_ACC_LIMIT_1);
	_LinearAcceleration(acc,target_velocity,&profile_velocity);

	exSetMotorControlWord(motor,VLP_PROFILED_VAL_1,profile_velocity);
	exSetMotorControlWord(motor,VLP_PROFILED_VAL,profile_velocity*15/motor->motor_desp.encoder_resolution);
	err = profile_velocity - actual_velocity;
	if (ABS(err) < motor->motor_desp.encoder_resolution)	//误差小于15rpm
	{
		_SetMotorState(motor,STA_SPEED_REACH,TRUE);	
	}
	exSetMotorControlWord(motor,VLP_ERROR_1,err);
	exSetMotorControlWord(motor,VLP_ERROR,err*15/motor->motor_desp.encoder_resolution);
	pid_out = iPosPIDCal(&(motor->pids[PID_V]),err)>>10;
	if (ABS(target_velocity) < (motor->motor_desp.encoder_resolution))//15rpm
	{
		target_velocity = 0;
		profile_velocity = 0;
		pid_out = 0;
	}
	if(ABS(profile_velocity) < motor->motor_desp.encoder_resolution)
	{
		motor->pids[PID_V].ei >>=1;
	}
	if (_GetMotorState(motor,STA_I_LOOP))
	{
		Limit(pid_out,exGetMotorControlWord(motor,CLP_LIMIT));
		exSetMotorControlWord(motor,CLP_TARGET_VAL,pid_out);
	}
	else
	{
		_SetMotorPWM(motor,pid_out);
	}

	return;
}
static void _Current_Loop_Control(stServoMotorDef* motor)
{
	int32_t  target_current = 0;
	int32_t  actual_current = 0;
	int32_t err = 0;
	int32_t pid_out = 0;

	if(!_GetMotorState(motor,STA_HAVE_I_SENSOR)) return;
	//电流采样
	actual_current = motor->GetCurrentFun();
	exSetMotorControlWord(motor,CLP_ACTUAL_VAL,actual_current);
	//计算超时时间内的平均电流
	if (gMotorCtrlWord[motor->motor_index][MOTOR_TM2]++ < exGetMotorControlWord(motor,CLP_OVERLOAD_TIME))
	{
		actual_current = (actual_current + exGetMotorControlWord(motor,CLP_CONTINUOUS_VAL))>>1;
	}
	else
	{
		exSetMotorControlWord(motor,MOTOR_TM2,0);
	}
	exSetMotorControlWord(motor,CLP_CONTINUOUS_VAL,actual_current);		
	if (ABS(actual_current) > motor->motor_desp.current_max_ma)//电流超限幅报警
	{
		_SetMotorState(motor,STA_ERROR,TRUE);
		_SetMotorError(motor,ERR_CURRENT_OVERLOAD,TRUE);
		_SetMotorPWM(motor,0);
	}
	
	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))	return;
	if(!_GetMotorState(motor,STA_I_LOOP))	return;
	//电流闭环控制
	target_current = exGetMotorControlWord(motor,CLP_TARGET_VAL);
	actual_current = exGetMotorControlWord(motor,CLP_ACTUAL_VAL);
	err = target_current - actual_current;
	exSetMotorControlWord(motor,CLP_ERROR,err);
	pid_out = iPosPIDCal(&(motor->pids[PID_I]),err)>>10;
	_SetMotorPWM(motor,pid_out);

	//电流限幅检测，可用于速度环电流环下的过载检测
	if (ABS(exGetMotorControlWord(motor,CLP_CONTINUOUS_VAL)) > exGetMotorControlWord(motor,CLP_LIMIT))
	{
		_SetMotorState(motor,STA_CURRENT_REACH,TRUE);	
	}

	//电流时间模式控制
	if (_GetMotorState(motor,STA_TIME_CONTROL))
	{
		if (target_current != 0)		//目标电流不为零
		{
			if(gMotorCtrlWord[motor->motor_index][MOTOR_TM1]++ > exGetMotorControlWord(motor,CLP_MOVE_TIME))
			{
				Servo_Motor_SetTargetCurrent(motor,0);
				_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
			}
		}
	}
	else
	{
		exSetMotorControlWord(motor,MOTOR_TM1,0);
	}

	return;
}
static void _Open_Loop_Control(stServoMotorDef* motor)
{
	int16_t  target_pwm = 0;
	int16_t  actual_pwm = 0;
	uint16_t	step = 50;		//根据加速度设置，目前暂定10

	if(_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR))
	{
		exSetMotorControlWord(motor,OLP_TARGET_PWM,0);
		return;
	}
	if(motor->ctrl_mode != MODE_OPEN_LOOP)  return;

	target_pwm = exGetMotorControlWord(motor,OLP_TARGET_PWM);
	actual_pwm = exGetMotorControlWord(motor,OLP_ACTUAL_PWM);
	step = exGetMotorControlWord(motor,OLP_STEP); 

	if (actual_pwm < target_pwm - step)
	{
		actual_pwm += step;
	} 
	else if (actual_pwm > target_pwm + step)
	{
		actual_pwm -= step;
	}
	else
	{
		actual_pwm = target_pwm;
	}
	if (actual_pwm != exGetMotorControlWord(motor,OLP_ACTUAL_PWM))
	{
		_SetMotorPWM(motor,actual_pwm);
	}

	//开环PWM时间模式控制
	if (_GetMotorState(motor,STA_TIME_CONTROL))
	{
		if(gMotorCtrlWord[motor->motor_index][MOTOR_TM1]++ > exGetMotorControlWord(motor,OLP_MOVE_TIME))
		{
			Servo_Motor_SetOpenVelocity(motor,0);
			exSetMotorControlWord(motor,MOTOR_TM1,0);
			_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
			_SetMotorState(motor,STA_CURRENT_SHUT_CONTROL,FALSE);	
			_SetMotorState(motor,STA_TIME_REACH,TRUE);
		}
	}
	else
	{
		exSetMotorControlWord(motor,MOTOR_TM1,0);
	}
	if (_GetMotorState(motor,STA_CURRENT_SHUT_CONTROL))
	{
		if (ABS(exGetMotorControlWord(motor,CLP_CONTINUOUS_VAL)) > exGetMotorControlWord(motor,CLP_LIMIT))
		{
			Servo_Motor_SetOpenVelocity(motor,0);
			_SetMotorState(motor,STA_CURRENT_REACH,TRUE);	
			_SetMotorState(motor,STA_CURRENT_SHUT_CONTROL,FALSE);	
			_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
		}
	}

	return;
}
static void _MotorControl_callback(void *ptmr, void *parg)
{
	for (;gMotorIndex<SERVO_MOTOR_NUM;gMotorIndex++)
	{
		if ((!_GetMotorState(gServoMotors[gMotorIndex],STA_SOFT_ENABLE)) || 
			(!_GetMotorState(gServoMotors[gMotorIndex],STA_HARD_ENABLE)) //|| 
			/*_GetMotorState(gServoMotors[gMotorIndex],STA_ERROR)*/)
		{
			continue;
		}
		_Position_Loop_Control(gServoMotors[gMotorIndex]);
		_Velocity_Loop_Control(gServoMotors[gMotorIndex]);
		_Current_Loop_Control(gServoMotors[gMotorIndex]);
		_Open_Loop_Control(gServoMotors[gMotorIndex]);

		gServoMotors[gMotorIndex]->error = exGetMotorControlWord(gServoMotors[gMotorIndex],MOTOR_ERROR);
		gServoMotors[gMotorIndex]->state = exGetMotorControlWord(gServoMotors[gMotorIndex],MOTOR_STATE);
	}
	gMotorIndex = 0;
}

//以下为接口函数
void Servo_Motor_Default(stServoMotorDef* motor)
{
	memset(motor,0,sizeof(stServoMotorDef));	
}

bool Servo_Motor_Init(stServoMotorDef* motor)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_TypeDef * port;

	if(motor->motor_index < SERVO_MOTOR_NUM)
		gServoMotors[motor->motor_index] = motor;
	else
		return FALSE;		

	//没有定义PWM接口函数
	if (motor->SetPwmFun == NULL)
	{
		_SetMotorState(motor,STA_ERROR,TRUE);
		_SetMotorError(motor,ERR_NO_PWM_FUN,TRUE);
		return FALSE;
	}

	if (motor->ctrl_pins.enable_pin_port != NULL)
	{
		port = motor->ctrl_pins.enable_pin_port;

		if (port == GPIOA)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		}
		else if (port == GPIOB)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		}
		else if (port == GPIOC)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		}

		GPIO_InitStructure.GPIO_Pin = motor->ctrl_pins.enable_pin;				
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(port, &GPIO_InitStructure);
	}
	else
		motor->ctrl_pins.enable_pin_polarity = NO_ENABLE_PIN;

	if (motor->ctrl_pins.dir_pin_port != NULL)
	{		
		port = motor->ctrl_pins.dir_pin_port;
		if (port == GPIOA)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		}
		else if (port == GPIOB)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		}
		else if (port == GPIOC)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		}
		GPIO_InitStructure.GPIO_Pin = motor->ctrl_pins.dir_pin;				
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(port, &GPIO_InitStructure);

		port = motor->ctrl_pins.dir_pin2_port;
		if (port == GPIOA)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		}
		else if (port == GPIOB)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		}
		else if (port == GPIOC)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		}
		GPIO_InitStructure.GPIO_Pin = motor->ctrl_pins.dir_pin2;		
		GPIO_Init(port, &GPIO_InitStructure);
	}
	else
		motor->ctrl_pins.dir_pin_polarity = NO_DIR_PIN;
	
	/**************************************/
	/*以下数据不对用户开放，但是可根据需要进行修改*/	
	exSetMotorControlWord(motor,CLP_LIMIT,motor->motor_desp.current_max_ma);
	exSetMotorControlWord(motor,VLP_VEL_LIMIT,motor->motor_desp.velocity_max_rpm);
	exSetMotorControlWord(motor,VLP_VEL_LIMIT_1,(int32_t)motor->motor_desp.velocity_max_rpm*motor->motor_desp.encoder_resolution/15);
	exSetMotorControlWord(motor,VLP_ACC_LIMIT,motor->motor_desp.acc_max);
	exSetMotorControlWord(motor,VLP_ACC_LIMIT_1,motor->motor_desp.acc_max*motor->motor_desp.encoder_resolution/15);
	exSetMotorControlWord(motor,PLP_MAX_POS,motor->motor_desp.posotion_positive_max);
	exSetMotorControlWord(motor,PLP_MIN_POS,motor->motor_desp.posotion_negative_max);

	//定义了电流接口函数
	if (motor->GetCurrentFun != NULL)
	{
		_SetMotorState(motor,STA_HAVE_I_SENSOR,TRUE);
		exSetMotorControlWord(motor,CLP_ACTUAL_VAL,motor->GetCurrentFun());
		exSetMotorControlWord(motor,CLP_CONTINUOUS_VAL,motor->GetCurrentFun());
		if (motor->ctrl_mode&MODE_CURRENT_LOOP)
		{
			exSetMotorControlWord(motor,CLP_ENABLE,TRUE);
			_SetMotorState(motor,STA_I_LOOP,TRUE);
		}
	}
	//定义了速度接口函数和编码器
	if (motor->GetVelocityFun != NULL && motor->motor_desp.encoder_resolution != 0)
	{
		_SetMotorState(motor,STA_HAVE_V_SENSOR,TRUE);
		if (motor->ctrl_mode&MODE_VELOCITY_LOOP)
		{
			exSetMotorControlWord(motor,VLP_ENABLE,TRUE);
			_SetMotorState(motor,STA_V_LOOP,TRUE);
		}
	}
	//定义了位置接口函数
	if (motor->GetPositionFun != NULL)
	{
		_SetMotorState(motor,STA_HAVE_P_SENSOR,TRUE);
		if (motor->ctrl_mode&MODE_POSITION_LOOP)
		{
			exSetMotorControlWord(motor,PLP_ENABLE,TRUE);
			_SetMotorState(motor,STA_P_LOOP,TRUE);
		}
	}
	
	_OsTimerConfig();

	return TRUE;
}

void Servo_Motor_Enable(stServoMotorDef* motor, bool val)
{
	_SetMotorState(motor, STA_SOFT_ENABLE,val);
	_SetMotorState(motor, STA_HARD_ENABLE,val);
	_SetMotorState(motor, STA_ERROR,FALSE);
	gMotorCtrlWord[motor->motor_index][MOTOR_ERROR] = 0;

	if (motor->ctrl_pins.enable_pin_polarity == NO_ENABLE_PIN)
		return;

	else if (motor->ctrl_pins.enable_pin_polarity == LOW_ENABLE)
	{
		if (val == TRUE)
			GPIO_ResetBits(motor->ctrl_pins.enable_pin_port, motor->ctrl_pins.enable_pin);
		else
			GPIO_SetBits(motor->ctrl_pins.enable_pin_port, motor->ctrl_pins.enable_pin);
	}
	else if (motor->ctrl_pins.enable_pin_polarity == HIGH_ENABLE)
	{
		if (val == TRUE)
			GPIO_SetBits(motor->ctrl_pins.enable_pin_port, motor->ctrl_pins.enable_pin);
		else
			GPIO_ResetBits(motor->ctrl_pins.enable_pin_port, motor->ctrl_pins.enable_pin);
	}
}

void Servo_Motor_SetOpenVelocity(stServoMotorDef* motor, int16_t velocity_rpm)
{
	if(motor->ctrl_mode != MODE_OPEN_LOOP)  return;

	Limit(velocity_rpm, exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,OLP_TARGET_PWM,velocity_rpm*PWM_DUTY_RANGE_DEF/exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,OLP_STEP,50);
	_SetMotorState(motor,STA_CURRENT_SHUT_CONTROL,FALSE);
	_SetMotorState(motor,STA_CURRENT_REACH,FALSE);
	_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
	_SetMotorState(motor,STA_TIME_REACH,FALSE);
}

void Servo_Motor_SetOpenVelocityTime(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t time_ms)
{
	if(motor->ctrl_mode != MODE_OPEN_LOOP)  return;
	
	Limit(velocity_rpm, exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,OLP_TARGET_PWM,velocity_rpm*PWM_DUTY_RANGE_DEF/exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,OLP_MOVE_TIME,time_ms);
	exSetMotorControlWord(motor,OLP_STEP,50);
	_SetMotorState(motor,STA_TIME_CONTROL,TRUE);
}

void Servo_Motor_SetOpenVelocityCurrent(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t current_ma)
{
	if(!_GetMotorState(motor,STA_HAVE_I_SENSOR) || motor->ctrl_mode != MODE_OPEN_LOOP)  return;

	Limit(velocity_rpm, exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	Limit(current_ma,motor->motor_desp.current_max_ma);
	exSetMotorControlWord(motor,OLP_TARGET_PWM,velocity_rpm*PWM_DUTY_RANGE_DEF/exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,CLP_LIMIT,current_ma);
	exSetMotorControlWord(motor,OLP_STEP,50);
	_SetMotorState(motor,STA_CURRENT_SHUT_CONTROL,TRUE);
}
void Servo_Motor_SetOpenVelocityCurrentTime(stServoMotorDef* motor, int16_t velocity_rpm, uint16_t current_ma, uint16_t time_ms)
{
	Servo_Motor_SetOpenVelocityCurrent(motor,velocity_rpm,current_ma);
	Servo_Motor_SetOpenVelocityTime(motor,velocity_rpm,time_ms);
}
void Servo_Motor_SetTargetCurrent(stServoMotorDef* motor, int16_t current_ma)
{
	if(!_GetMotorState(motor,STA_HAVE_I_SENSOR) || !_GetMotorState(motor,STA_I_LOOP))	
		return; 

	Limit(current_ma, exGetMotorControlWord(motor,CLP_LIMIT));
	exSetMotorControlWord(motor,CLP_TARGET_VAL,current_ma);
	_SetMotorState(motor,STA_TIME_CONTROL,FALSE);
}

void Servo_Motor_SetTargetCurrentTime(stServoMotorDef* motor, int16_t current_ma, uint16_t time_ms)
{
	if(!_GetMotorState(motor,STA_HAVE_I_SENSOR) || !_GetMotorState(motor,STA_I_LOOP))	
		return; 

	Limit(current_ma, exGetMotorControlWord(motor,CLP_LIMIT));
	exSetMotorControlWord(motor,CLP_TARGET_VAL,current_ma);
	exSetMotorControlWord(motor,CLP_MOVE_TIME,time_ms);
	_SetMotorState(motor,STA_TIME_CONTROL,TRUE);
}
void Servo_Motor_SetTargetVelocity(stServoMotorDef* motor, int16_t velocity_rpm)
{
	if(!_GetMotorState(motor,STA_HAVE_V_SENSOR) || !_GetMotorState(motor,STA_V_LOOP))	
		return; 

	Limit(velocity_rpm, exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,VLP_TARGET_VAL,velocity_rpm);
	exSetMotorControlWord(motor,VLP_TARGET_VAL_1,velocity_rpm*motor->motor_desp.encoder_resolution/15);
	_SetMotorState(motor,STA_SPEED_REACH,FALSE);
}
void Servo_Motor_QuickStop(stServoMotorDef* motor)
{
	exSetMotorControlWord(motor,VLP_TARGET_VAL,0);
	exSetMotorControlWord(motor,VLP_TARGET_VAL_1,0);
	exSetMotorControlWord(motor,VLP_PROFILED_VAL,0);
	exSetMotorControlWord(motor,VLP_PROFILED_VAL_1,0);
	exSetMotorControlWord(motor,OLP_ACTUAL_PWM,0);
	exSetMotorControlWord(motor,OLP_TARGET_PWM,0);
}
void Servo_Motor_SetTargetPosition(stServoMotorDef* motor, int16_t vel_limit, int32_t position)
{
	if(!_GetMotorState(motor,STA_HAVE_P_SENSOR) || !_GetMotorState(motor,STA_P_LOOP))	
		return; 

	Range(position, exGetMotorControlWord(motor,PLP_MIN_POS), exGetMotorControlWord(motor,PLP_MAX_POS));
	Limit(vel_limit,exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,PLP_MAX_VEL,vel_limit*motor->motor_desp.encoder_resolution/15);
	exSetMotorControlWord(motor,PLP_TARGET_VAL,position);
	_SetMotorState(motor,STA_POSITION_REACH,FALSE);
}

void Servo_Motor_SetRelativePosition(stServoMotorDef* motor, int16_t vel_limit, int32_t position)
{
	if(!_GetMotorState(motor,STA_HAVE_P_SENSOR) || !_GetMotorState(motor,STA_P_LOOP))	
		return; 

	position = exGetMotorControlWord(motor,PLP_ACTUAL_VAL) + position;
	Range(position, exGetMotorControlWord(motor,PLP_MIN_POS), exGetMotorControlWord(motor,PLP_MAX_POS));
	Limit(vel_limit,exGetMotorControlWord(motor,VLP_VEL_LIMIT));
	exSetMotorControlWord(motor,PLP_MAX_VEL,vel_limit*motor->motor_desp.encoder_resolution/15);
	exSetMotorControlWord(motor,PLP_TARGET_VAL,position);
	_SetMotorState(motor,STA_POSITION_REACH,FALSE);
}

void Servo_Motor_ClearState(stServoMotorDef* motor, uint8_t bit)
{
	_SetMotorState(motor,bit,FALSE);
}
void exSetMotorControlWord(stServoMotorDef* motor, enMotorCrtlWord addr, int32_t val)
{
	gMotorCtrlWord[motor->motor_index][addr] = val;
}
int32_t exGetMotorControlWord(stServoMotorDef* motor, enMotorCrtlWord addr)
{
	return gMotorCtrlWord[motor->motor_index][addr];
}

#endif
