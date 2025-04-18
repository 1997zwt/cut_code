#include "grip.h"

//2024.3.8
//2024.4.11修改，0x0A指令退抱紧后状态变为位置到位
//2024.4.22修改，增加强制解锁中途憋死报错

static stServoMotorDef gGripMotor;
uint8_t gReserveflg = 0;
uint8_t gGripStopFlg = 0;
stCableDef gCable = {0};
stGripDef gGrip;

int32_t Grip_GetPos(void)
{
	return (gGrip.motor_pos * 10 / GRIP_GEAR_RATIO) >> 13; //返回0.1mm单位的抱紧位置
}
stCableDef *GetCableInfo(void) //获取线缆信息
{
	return &gCable;
	/*V型槽夹角110度，线缆半径为单边行程*sin(55)=单边行程*0.82左右
	刀尖位置按照在V型槽尖角处计算
	*/
}
int32_t Grip_GetPosition(void)
{
	gGrip.motor_pos = DJ_GetMotorAngle(ID_GRIP_L_MOTOR) * gGripMotor.motor_desp.rotate_dir;
	return gGrip.motor_pos;
}
int32_t Grip_GetVelocity(void)
{
	gGrip.motor_speed = DJ_GetMotorSpeed(ID_GRIP_L_MOTOR) * gGripMotor.motor_desp.rotate_dir;
	return gGrip.motor_speed;
}
int16_t Grip_GetCurrent(void)
{
	gGrip.motor_current = DJ_GetMotorCurrent(ID_GRIP_L_MOTOR) >> 2;
	return gGrip.motor_current;
}
void Grip_SetPWM(uint16_t DutyRadio)
{
	int32_t duty = 0;

	duty = exGetMotorControlWord(&gGripMotor, OLP_ACTUAL_PWM) * gGripMotor.motor_desp.rotate_dir;
	DJ_SetMotorVoltage(ID_GRIP_L_MOTOR, duty);
	DJ_UpdateCanMotors();
}
void Grip_SetTargetCurrent(int16_t new_current)
{
	DJ_SetMotorVoltage(ID_GRIP_L_MOTOR, new_current * gGripMotor.motor_desp.rotate_dir);
	DJ_UpdateCanMotors();
}
void Grip_Init(uint8_t index)
{
	Servo_Motor_Default(&gGripMotor);
	gGripMotor.motor_index = index;
	gGripMotor.ctrl_mode = MODE_VELOCITY_LOOP;
	gGripMotor.pids[PID_V].Kp = 5000;
	gGripMotor.pids[PID_V].Ki = 10;
	gGripMotor.pids[PID_V].errlim = 10000;
	gGripMotor.pids[PID_P].Kp = 35;
	gGripMotor.SetPwmFun = Grip_SetPWM;
	gGripMotor.GetCurrentFun = Grip_GetCurrent;
	gGripMotor.GetVelocityFun = Grip_GetVelocity;
	gGripMotor.GetPositionFun = Grip_GetPosition;

	gGripMotor.motor_desp.rotate_dir = GRIP_DIR_DEF;
	gGripMotor.motor_desp.current_max_ma = 3000;
	gGripMotor.motor_desp.velocity_max_rpm = 15000;
	gGripMotor.motor_desp.acc_max = 30000;
	gGripMotor.motor_desp.encoder_resolution = 15;
	gGripMotor.motor_desp.posotion_positive_max = ((ReadUserSettings(GRIP_POSITIVE_LIMIT) * GRIP_GEAR_RATIO) << 13) / 10;
	gGripMotor.motor_desp.posotion_negative_max = ((ReadUserSettings(GRIP_NEGATIVE_LIMIT) * GRIP_GEAR_RATIO) << 13) / 10;

	gGrip.state = MODULE_STATE_UNINIT;
	if (Servo_Motor_Init(&gGripMotor))
	{
		Servo_Motor_Enable(&gGripMotor, TRUE);
		exSetMotorControlWord(&gGripMotor, CLP_OVERLOAD_TIME, 200);
	}
}

ToolMotorStateDef Grip_GetMotorState(void)
{
	return gGrip.motor_state;
}

uint8_t Grip_GetModuleState(void)
{
	if (gGrip.state == MODULE_STATE_N_LIM)
		return MODULE_STATE_HOME;
	else if (gGrip.state == MODULE_STATE_P_LIM)
		return MODULE_STATE_MAX;
	else
		return gGrip.state;
}

//左夹紧电机
CommandReturn Grip_Control(uint8_t cmd, int8_t data)
{
	CommandReturn ret = RT_OK;

	Servo_Motor_Enable(&gGripMotor, TRUE);
	switch (cmd)
	{
	case MOTOR_CMD_STOP:
		Servo_Motor_QuickStop(&gGripMotor);
		gGrip.motor_state.work_state = WORK_STATE_IDLE;
		if (gGrip.state == MODULE_STATE_WORKING || gGrip.state == MODULE_STATE_HOMING || gGrip.state == MODULE_STATE_ERROR)
		{
			gGrip.state = MODULE_STATE_NOMAL;
			if (gGrip.step == 1)
			{
				gGrip.state = MODULE_STATE_UNINIT;
			}
		}
		break;
	case MOTOR_CMD_CCW:
		if (gGrip.motor_state.work_state == WORK_STATE_IDLE /* && gGrip.state != MODULE_STATE_UNINIT*/)
		{
			Servo_Motor_SetTargetVelocity(&gGripMotor, -ABS(data) * gGripMotor.motor_desp.velocity_max_rpm / 100);
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_CW:
		if (gGrip.motor_state.work_state == WORK_STATE_IDLE /* && gGrip.state != MODULE_STATE_UNINIT*/)
		{
			Servo_Motor_SetTargetVelocity(&gGripMotor, ABS(data) * gGripMotor.motor_desp.velocity_max_rpm / 100);
		}
		else
			ret = RT_MOTOR_NOT_MANUL;

		break;
	case MOTOR_CMD_WORK:
		if (gGrip.motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			gGrip.motor_state.work_state = WORK_STATE_WORK;
			if (gGrip.state != MODULE_STATE_MAX && gGrip.state != MODULE_STATE_P_LIM)
			{
				if (data < 5 && data > 0) //太小转不动
					data = 5;
				gGrip.aim_pos = ((ABS(data) * GRIP_GEAR_RATIO) << 13) / 10; //0.1mm转换为转换为脉冲数
				if (gGrip.aim_pos > 0)
				{
					gGrip.mode = 1; //抱紧结束后退
				}
				else
				{
					gGrip.mode = 2; //抱紧结束不退
				}
				if (gGrip.state == MODULE_STATE_UNINIT)
				{
					gGrip.mode += 5; //上电复位后工作
					gGrip.step = 1;
					gGrip.motor_state.work_state = WORK_STATE_HOME;
					Servo_Motor_SetTargetVelocity(&gGripMotor, -4000);
				}
				else
				{
					gGrip.step = 11; //正常工作步骤，向最大位置运动
					Servo_Motor_SetTargetVelocity(&gGripMotor, ReadUserSettings(GRIP_MOVE_SPEED));
				}
			}
			else
				ret = RT_MOTOR_IS_LIMIT;
		}
		else
			ret = RT_MOTOR_NOT_STOP;
		break;
	case MOTOR_CMD_HOME:
		if (gGrip.motor_state.motion_state != MOTION_STATE_STOPPED || gGrip.state == MODULE_STATE_WORKING)
		{
			ret = RT_MOTOR_NOT_STOP;
		}
		else
		{
			if (gGrip.state != MODULE_STATE_HOME && gGrip.state != MODULE_STATE_N_LIM)
			{
				if (gGrip.state != MODULE_STATE_UNINIT)
				{
					gGrip.mode = 3; //正常复位,向最小位置运动
					Servo_Motor_SetTargetVelocity(&gGripMotor, -ReadUserSettings(GRIP_MOVE_SPEED));
				}
				else
				{
					gGrip.mode = 5; //上电复位
					gGrip.step = 1;
					Servo_Motor_SetTargetVelocity(&gGripMotor, -4000);
				}
			}
			else
			{
				ret = RT_MOTOR_IS_HOME;
			}
			gGrip.motor_state.work_state = WORK_STATE_HOME;
		}
		break;
	case 0x0A: //抱紧后退
		if (gGrip.state == MODULE_STATE_MAX)
		{
			gGrip.state = MODULE_STATE_WORKING;
			gGrip.aim_pos = ((ABS(data) * GRIP_GEAR_RATIO) << 13) / 10; 
			gGrip.aim_pos = gGrip.motor_pos - gGrip.aim_pos;
			gGrip.step = 13;
			gGrip.mode = 6; //指令退抱紧
			Servo_Motor_SetTargetVelocity(&gGripMotor, -2000);
		}
		break;
	case MOTOR_CMD_UNLOCK:
		if (gGrip.motor_state.work_state == WORK_STATE_IDLE)
		{
			if (gGrip.state != MODULE_STATE_UNINIT)
			{
				gGrip.motor_state.work_state = WORK_STATE_HOME;
				gGrip.mode = 11;
				Servo_Motor_SetTargetVelocity(&gGripMotor, -12000);
			}
			else
				ret = RT_MOTOR_NOT_MANUL;
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_SET:
		if (gGrip.motor_state.work_state == WORK_STATE_IDLE)
		{
			if (data == 0)
				gGrip.state = MODULE_STATE_UNINIT;
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	}
	gGrip.timer = 0;
	return ret;
}
void Cable_Measuring(void)
{
	gCable.diameter = (GRIP_MAX_DISTENCE - gGrip.pos) + 75; //9是经过测量得到的平均误差补偿
	gCable.cut_gap = ((GRIP_MAX_DISTENCE - gGrip.pos * 2) - gCable.diameter) / 2;
	if (gCable.cut_gap < 2)
	{
		gCable.cut_gap = 0;
	}
}
// int16_t test_currentg[10] = {0};
// int16_t test_speedg[10] = {0};
// uint8_t test_step[10] = {0};
// uint8_t test_mode[10] = {0};
// uint16_t test_tim[10] = {0};
// uint8_t test_index = 0;
void Grip_Update(void)
{
	static uint32_t current_timer = 0;

	gGrip.pos = Grip_GetPos();
	gGrip.timer += 20;

	if (gGrip.motor_speed > 300)
	{
		gGrip.motor_state.motion_state = MOTION_STATE_CW;
	}
	else if (gGrip.motor_speed < -300)
	{
		gGrip.motor_state.motion_state = MOTION_STATE_CCW;
	}
	else if (ABS(gGrip.motor_speed) < 100)
	{
		gGrip.motor_state.motion_state = MOTION_STATE_STOPPED;
	}

	if (gGrip.motor_state.work_state == WORK_STATE_WORK)
	{
		if (gGrip.motor_state.motion_state == MOTION_STATE_CW && gGrip.mode != 0)
		{
			gGrip.state = MODULE_STATE_WORKING;
		}

		if (gGrip.step == 11) //正常工作过程
		{
			if (gGrip.timer > 500)
			{
				if (ABS(gGrip.motor_current) >= ReadUserSettings(GRIP_WORK_CURRENT) || (ABS(gGrip.motor_current) >= 1000 && gGrip.mode == 1))
				{
					Servo_Motor_QuickStop(&gGripMotor);
					gGrip.step = 12;
					Cable_Measuring();
				}
				else if (gGrip.motor_pos >= gGripMotor.motor_desp.posotion_positive_max - 320000)
				{
					Servo_Motor_QuickStop(&gGripMotor);
					gGrip.step = 12;
					gGrip.mode = 4; //软件正限位
				}
			}
		}
		else if (gGrip.step == 12) //
		{
			if (gGrip.motor_state.motion_state == MOTION_STATE_STOPPED)
			{
				if (gGrip.mode == 1) //抱紧完成后退
				{
					if (gCable.diameter > 0)
					{
						Servo_Motor_SetTargetVelocity(&gGripMotor, -2000);
						gGrip.aim_pos = gGrip.motor_pos - gGrip.aim_pos;
						gGrip.step = 13;
						gGrip.timer = 0;
					}
					else
					{
						gGrip.state = MODULE_STATE_MAX;
						gGrip.step = 0;
						gGrip.mode = 0;
					}
				}
				else
				{
					if (gGrip.mode == 4)
					{
						gGrip.state = MODULE_STATE_P_LIM;
						gGrip.step = 0;
					}
					else if (gGrip.mode == 2) //抱紧完不后退
					{
						gGrip.state = MODULE_STATE_MAX;
						gGrip.step = 0;
					}
					else if (gGrip.mode == 6) //指令退抱紧
					{
						gGrip.state = MODULE_STATE_POS_REACH;
						gGrip.step = 0;
					}
					gGrip.mode = 0;
				}
			}
		}
		else if (gGrip.step == 13) //
		{
			if (gGrip.motor_pos <= gGrip.aim_pos + 10000)
			{
				Servo_Motor_QuickStop(&gGripMotor);
				gGrip.step = 12;
				if (gGrip.mode == 1)//抱紧完后退
				{
					gGrip.mode = 2;
				}
			}
			else if (gGrip.timer > 5000) //退不动
			{
				Servo_Motor_QuickStop(&gGripMotor);
				gGrip.state = MODULE_STATE_ERROR;
				faultcode = FAULT_GRIPBACK;
			}
		}
	}
	else if (gGrip.motor_state.work_state == WORK_STATE_HOME)
	{
		if (gGrip.motor_state.motion_state == MOTION_STATE_CCW && gGrip.mode != 0) //防止外部拧动产生状态错误
		{
			gGrip.state = MODULE_STATE_HOMING;
		}

		if (gGrip.step == 0) //正常复位
		{
			if (gGrip.mode == 3) //正常位置控制复位
			{
				if (gGrip.motor_pos <= gGripMotor.motor_desp.posotion_negative_max + 320000)
				{
					Servo_Motor_QuickStop(&gGripMotor);
					gGrip.step = 4;
				}
			}
			else if (gGrip.mode == 11) //强制解锁
			{
				if (ABS(gGrip.motor_speed) > 10000)
				{
					Servo_Motor_SetTargetVelocity(&gGripMotor, -ReadUserSettings(GRIP_MOVE_SPEED));
					gGrip.mode = 5; //上电复位
					gGrip.step = 5;
				}
			}
		}
		else if (gGrip.step == 1) //上电复位
		{
			if (ABS(gGrip.motor_current) >= ReadUserSettings(GRIP_HOME_CURRENT) && gGrip.timer > 500)
			{
				Servo_Motor_QuickStop(&gGripMotor);
				gGrip.step = 2;
			}
		}
		else if (gGrip.step == 2)
		{
			if (gGrip.motor_state.motion_state == MOTION_STATE_STOPPED)
			{
				DJ_SetMotorPos(ID_GRIP_L_MOTOR, 0);
				if (gGrip.mode == 5) //上电复位
				{
					gGrip.step = 3;
					Servo_Motor_SetTargetVelocity(&gGripMotor, 2000);
				}
				else if (gGrip.mode > 5) //复位后工作
				{
					gGrip.mode -= 5;
					gGrip.step = 11;
					gGrip.timer = 0;
					gGrip.motor_state.work_state = WORK_STATE_WORK;
					Servo_Motor_SetTargetVelocity(&gGripMotor, ReadUserSettings(GRIP_MOVE_SPEED));
				}
			}
		}
		else if (gGrip.step == 3)
		{
			if (gGrip.motor_pos >= gGripMotor.motor_desp.posotion_negative_max - 10000)
			{
				Servo_Motor_QuickStop(&gGripMotor);
				gGrip.step = 4;
			}
		}
		else if (gGrip.step == 4)
		{
			if (gGrip.motor_state.motion_state == MOTION_STATE_STOPPED)
			{
				gGrip.state = MODULE_STATE_N_LIM;
				gGrip.mode = 0;
				gGrip.step = 0;
			}
		}
		else if (gGrip.step == 5) //强制解锁
		{
			if (ABS(gGrip.motor_current) >= ReadUserSettings(GRIP_HOME_CURRENT) && gGrip.timer > 500)
			{
				Servo_Motor_QuickStop(&gGripMotor);
				if (gGrip.motor_pos <= gGripMotor.motor_desp.posotion_negative_max + 320000)
				{
					gGrip.step = 2;
				}
				else
				{
					gGrip.state = MODULE_STATE_ERROR;
					faultcode = FAULT_UNLOCK;
					gGrip.step = 0;
					gGrip.mode = 0;
				}
			}
		}
	}
	else
	{
		if (gGrip.motor_speed == 0)
		{
			gGrip.mode = 10;
			gGrip.step = 0;
			Servo_Motor_Enable(&gGripMotor,FALSE);
		}
	}
	// if (test_step[test_index] != gGrip.step)
	// {
	// 	test_index++;
	// 	test_step[test_index] = gGrip.step;
	// 	test_mode[test_index] = gGrip.mode;
	// 	test_speedg[test_index] = gGrip.motor_speed;
	// 	test_currentg[test_index] = gGrip.motor_current;
	// 	test_tim[test_index] = gGrip.timer;
	// 	if (gGrip.step == 0)
	// 	{
	// 		while (test_index < 9)
	// 		{
	// 			test_index++;
	// 			test_step[test_index] = 0;
	// 			test_mode[test_index] = 0;
	// 			test_speedg[test_index] = 0;
	// 			test_currentg[test_index] = 0;
	// 			test_currentg[test_index] = 0;
	// 		}

	// 		test_index = 0;
	// 	}
	// }
	if(ABS(gGrip.motor_current) >= 200)
	{
		if (ABS(gGrip.motor_speed) <= 20)
		{
			current_timer += 20;
			if (current_timer > 3000) //连续3秒电流速度为0 电流不为0报错
			{
				gGrip.mode = 0;
				gGrip.step = 0;
				Servo_Motor_QuickStop(&gGripMotor);
				gGrip.state = MODULE_STATE_ERROR;
				faultcode = FAULT_GRIPSTOP;
			}
		}
		else if (ABS(gGrip.motor_speed) >= 200)
		{
			current_timer = 0;
		}
	}
	else if (ABS(gGrip.motor_current) <= 20)
	{
		current_timer = 0;
	}
}
