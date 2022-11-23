#include "grip.h"

//2022.1.5

ToolMotorStateDef gGripMotorState;
static stDJMotorDef  gGripMotor;
int32_t gGripMotorPos = 0;
uint32_t gGripTim = 0;
uint8_t gReserveflg = 0;
uint16_t gReserveTime = 0;	//抱紧电机退刀时间
ModuleStateDef gGripState = MODULE_STATE_UNINIT;
uint8_t gGripStopFlg = 0;

int16_t test_current=0,test_speed=0;

void Grip_Init(uint8_t index)
{
	DJ_Motor_Default(&gGripMotor);
	gGripMotor.motor_index = index;
	gGripMotor.can_id = ID_GRIP_L_MOTOR;
	gGripMotor.type = DJ_TYPE_2610;
	gGripMotor.ctrl_mode = MODE_VELOCITY_LOOP;
	gGripMotor.pids[PID_V].Kp = 10000;
	gGripMotor.pids[PID_V].Ki = 10;
	gGripMotor.pids[PID_V].errlim = 10000;

	gGripMotor.motor_desp.rotate_dir = -1;
	gGripMotor.motor_desp.current_max_ma = 3000;
	gGripMotor.motor_desp.velocity_max_rpm = 10000;
	gGripMotor.motor_desp.acc_max = 10000;
	gGripMotor.motor_desp.posotion_positive_max = ((ReadUserSettings(GRIP_POSITIVE_LIMIT)*GRIP_GEAR_RATIO)<<13) - (ReadUserSettings(GRIP_MOVE_SPEED)<<4);
	gGripMotor.motor_desp.posotion_negative_max = ((ReadUserSettings(GRIP_NEGATIVE_LIMIT)*GRIP_GEAR_RATIO)<<13) + (ReadUserSettings(GRIP_MOVE_SPEED)<<5);

	if (DJ_Motor_Init(&gGripMotor))
	{
		DJ_Motor_Enable(&gGripMotor,TRUE);
		exSetMotorControlWord(&gGripMotor,CLP_OVERLOAD_TIME,200);
	}
}

ToolMotorStateDef Grip_GetMotorState(void)
{
	return gGripMotorState;
}

uint8_t Grip_GetModuleState(void)
{
	 if (gGripState == MODULE_STATE_N_LIM) 
		 return MODULE_STATE_HOME;
	 else  if (gGripState == MODULE_STATE_P_LIM) 
		return MODULE_STATE_MAX;
	 else
		 return gGripState;
}

uint16_t Grip_GetPos(void)
{
	return (gGripMotorPos*10/GRIP_GEAR_RATIO)>>14;//返回0.1mm单位的抱紧位置
}
//左夹紧电机
CommandReturn Grip_Control(uint8_t cmd, int8_t data)
{
	CommandReturn ret = RT_OK;

	switch(cmd)
	{
	case MOTOR_CMD_STOP:
		DJ_Motor_SetTargetVelocity(&gGripMotor,0);
		gGripMotorState.work_state = WORK_STATE_IDLE;
		if (gGripState == MODULE_STATE_WORKING || gGripState == MODULE_STATE_HOMING || gGripState == MODULE_STATE_ERROR)
		{
			gGripState = MODULE_STATE_NOMAL;
		}
		break;
	case MOTOR_CMD_CCW:
		if (gGripMotorState.work_state == WORK_STATE_IDLE/* && gGripState != MODULE_STATE_UNINIT*/)
		{
			DJ_Motor_SetTargetVelocity(&gGripMotor,-ABS(data)*gGripMotor.motor_desp.velocity_max_rpm/100);
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_CW:
		if (gGripMotorState.work_state == WORK_STATE_IDLE/* && gGripState != MODULE_STATE_UNINIT*/)
		{
			DJ_Motor_SetTargetVelocity(&gGripMotor,ABS(data)*gGripMotor.motor_desp.velocity_max_rpm/100);
		}
		else
			ret = RT_MOTOR_NOT_MANUL;

		break;
	case MOTOR_CMD_WORK:
		if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)		
		{
			gGripMotorState.work_state = WORK_STATE_WORK;
			if (gGripState != MODULE_STATE_MAX && gGripState != MODULE_STATE_P_LIM)
			{
				gReserveTime = ABS(data)*100;			//转换为ms
				if (gReserveTime > 0)
				{
					DJ_Motor_SetTargetVelocity(&gGripMotor,5000);
					gReserveflg = 1;//退
				}
				else
				{
					DJ_Motor_SetTargetVelocity(&gGripMotor,(int16_t)ReadUserSettings(GRIP_MOVE_SPEED));
					gReserveflg = 0;//不退
				}
				if(gGripState == MODULE_STATE_UNINIT)
				{
					DJ_Motor_SetTargetVelocity(&gGripMotor,-10000);					
					gGripMotorState.work_state = WORK_STATE_HOME;
					gGripStopFlg = 5;//上电复位后工作
				}
			}
			else
				ret = RT_MOTOR_IS_LIMIT;
		}
		else
			ret = RT_MOTOR_NOT_STOP;
		break;
	case MOTOR_CMD_HOME:
		if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)		
		{
			gGripMotorState.work_state = WORK_STATE_HOME;		
			if(gGripState != MODULE_STATE_HOME && gGripState != MODULE_STATE_N_LIM)
			{
				DJ_Motor_SetTargetVelocity(&gGripMotor,-10000);
				if (gGripState != MODULE_STATE_UNINIT)
				{
					gGripStopFlg = 6;
				}
				else
				{
					gGripStopFlg = 4;//上电复位
				}		
			}
			else
				ret = RT_MOTOR_IS_HOME;
		}
		else
			ret = RT_MOTOR_NOT_STOP;
		break;
	case 0x0A:		//抱紧之后退刀
		if (gGripState == MODULE_STATE_MAX)
		{
			DJ_Motor_SetTargetVelocity(&gGripMotor,-2000);
			gGripState = MODULE_STATE_WORKING;
			gReserveTime = ABS(data)*100;			//转换为ms
			gReserveflg = 3;
		}
		break;
	case MOTOR_CMD_UNLOCK:	
		if (gGripMotorState.work_state == WORK_STATE_IDLE/* && gGripState != MODULE_STATE_UNINIT*/)
		{
			gGripState = MODULE_STATE_UNINIT;
			gGripStopFlg = 7;
			DJ_Motor_SetTargetVelocity(&gGripMotor,-10000);
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_SET:
		if (gGripMotorState.work_state == WORK_STATE_IDLE)
		{
			if(data == 0)
				gGripState = MODULE_STATE_UNINIT;
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	}
	gGripTim = 0;
	test_speed=0;
	test_current = 0;
	return ret;
}

void Grip_Update(void)
{
	int16_t new_speed = 0;
	int16_t new_current = 0;
	static uint8_t flg = 0;
//	int16_t pos_err = 0;
//	int16_t work_speed = 0;

	new_speed = exGetMotorControlWord(&gGripMotor,VLP_ACTUAL_VAL);
	new_current = exGetMotorControlWord(&gGripMotor,CLP_CONTINUOUS_VAL);
	gGripTim += 20;

if (ABS(test_current) < ABS(new_current))
{
	test_current = new_current;
}

	if (new_speed > 500)
	{
		gGripMotorState.motion_state = MOTION_STATE_CW;
	}
	else if (new_speed < -500)
	{
		gGripMotorState.motion_state = MOTION_STATE_CCW;
	}
	else if(ABS(new_speed) < 100)
	{
		gGripMotorState.motion_state = MOTION_STATE_STOPPED;
	}
	if (gGripState != MODULE_STATE_UNINIT)
	{
		gGripMotorPos = exGetMotorControlWord(&gGripMotor,PLP_ACTUAL_VAL)*gGripMotor.motor_desp.rotate_dir;
		if (gGripMotorPos >= gGripMotor.motor_desp.posotion_positive_max)
		{
			if (gGripMotorState.motion_state == MOTION_STATE_CW)
			{
				DJ_Motor_QuickStop(&gGripMotor);
				gGripStopFlg = 1;
			}
			else if(gGripStopFlg == 1)
			{
				if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
				{
					gGripState = MODULE_STATE_P_LIM;	
					gGripStopFlg = 0;								
				}
			}
		}
		else if (gGripMotorPos <= gGripMotor.motor_desp.posotion_negative_max)
		{
			if (gGripStopFlg == 6)
			{
				DJ_Motor_QuickStop(&gGripMotor);
				gGripStopFlg = 2;
			}
			else if(gGripStopFlg == 2)
			{
				if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
				{
					gGripState = MODULE_STATE_N_LIM;	
					gGripStopFlg = 0;								
				}
			}
		}
		if (gGripMotorState.work_state == WORK_STATE_WORK)
		{
			if (gGripMotorState.motion_state == MOTION_STATE_CW)
			{
				gGripState = MODULE_STATE_WORKING;
			}
			if (gGripState == MODULE_STATE_WORKING)
			{
				if (gReserveflg == 0 && gGripTim > 500)
				{
					if (ABS(new_current) > ReadUserSettings(GRIP_WORK_CURRENT) || new_speed == 0 || gGripTim > ReadUserSettings(GRIP_MOVE_OVERTIME))
					{
						DJ_Motor_QuickStop(&gGripMotor);
						gReserveflg = 4;						
					}
				}
				else if (gReserveflg == 1)
				{
					if (ABS(new_current) > 800 || new_speed == 0 || gGripTim > 10000)
					{
						DJ_Motor_QuickStop(&gGripMotor);
						gReserveflg = 2;			
					}
				}
				else if (gReserveflg == 2)
				{
					if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
					{
						DJ_Motor_SetTargetVelocity(&gGripMotor,-1000);
						gReserveflg = 3;			
						gGripTim = 0;
					}
				}
				else if (gReserveflg == 3)
				{
					if (gGripMotorState.motion_state == MOTION_STATE_CCW)
					{
						if (gGripTim > gReserveTime)	
						{
							DJ_Motor_QuickStop(&gGripMotor);
							gReserveflg = 4;		
						}	
					}
				}
				else if (gReserveflg == 4)
				{
					if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
					{
						gGripState = MODULE_STATE_MAX;
						gGripTim = 0;
						gReserveflg = 0;
					}
				}
			}
		}
		else if (gGripMotorState.work_state == WORK_STATE_HOME)
		{
			if (gGripMotorState.motion_state == MOTION_STATE_CCW && gGripStopFlg != 0)
			{
				gGripState = MODULE_STATE_HOMING;
			}
			if (flg == 0)
			{ //用最大力量打开抱紧，在到达复位位置前减速，超时时间为3秒
				if (gGripStopFlg == 6)
				{
					if((gGripMotorPos <= gGripMotor.motor_desp.posotion_negative_max + 600000) || gGripTim > 3000)
					{
						DJ_Motor_SetTargetVelocity(&gGripMotor,-4000);
						gGripTim = 0;
					}
				}
			}				
			else if (flg == 1)
			{
				if (ABS(new_current) > ReadUserSettings(GRIP_HOME_CURRENT)  || new_speed == 0) 
				{
					DJ_Motor_QuickStop(&gGripMotor);
					DJ_SetMotorPos(ID_GRIP_L_MOTOR,0);
					if (gGripStopFlg == 4) //上电复位
					{		
						DJ_Motor_SetTargetVelocity(&gGripMotor,3000);//复位回退	
						flg = 2;	
					}
					else if (gGripStopFlg == 5) //上电先复位后工作
					{		
						flg = 3;
					}
					else if (gGripStopFlg == 6) //正常复位
					{		
						flg = 4;
					}
					gGripTim = 0;
				}						
				if (gGripTim > ReadUserSettings(GRIP_MOVE_OVERTIME) + 20000000/ABS(new_speed))
				{
					DJ_Motor_QuickStop(&gGripMotor);
					gGripMotorState.work_state = WORK_STATE_IDLE;
					gGripState = MODULE_STATE_ERROR;
				}
			}
			else if(flg == 3)//先复位后工作
			{
				if(gGripTim > 500 || gGripMotorState.motion_state == MOTION_STATE_STOPPED)
				{
					DJ_Motor_SetTargetVelocity(&gGripMotor,5000);//工作
					gGripMotorState.work_state = WORK_STATE_WORK;
					flg = 0;
				}
			}
			else if(flg == 2)//上电复位回退300ms
			{
				if (gGripTim > 300)
				{				
					DJ_Motor_QuickStop(&gGripMotor);
					flg = 4;
				}								
			}
			else if(flg == 4)//等待完全停止
			{
				if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
				{
					gGripState = MODULE_STATE_HOME;
					gGripStopFlg = 0;
					flg = 0;							
				}
			}
		}
		else
		{
			gGripStopFlg = 0;
			flg = 0;
		}
	}
	else
	{		
		if (gGripStopFlg != 0)
		{
			if (gGripStopFlg == 7)
			{//强制解锁不限制电流，确保能够打开，但是开机发送解锁命令可能导致电机憋死
				if (new_speed > 1000)//憋死后正转
				{
					gGripMotorState.work_state = WORK_STATE_HOME;
					gGripState = MODULE_STATE_HOMING;
					gGripTim = 0;
					flg = 2;
				}
				else
				{
					if(new_speed < -7000 || gGripTim > 1000)
					{
						DJ_Motor_SetTargetVelocity(&gGripMotor,-2000);
						gGripMotorState.work_state = WORK_STATE_HOME;
						gGripState = MODULE_STATE_HOMING;
						gGripStopFlg = 4;
						gGripTim = 0;
						flg = 1;
						test_speed = new_speed;
					}
				}
			}
			else
			{//复位命令或工作命令对电流进行限制，但是开机时如果处于抱紧状态可能导致复位失败
				if(ABS(new_speed) > 3000 || gGripTim > 1000 || ABS(new_current) > 1500)
				{
					DJ_Motor_SetTargetVelocity(&gGripMotor,-2000);
					gGripMotorState.work_state = WORK_STATE_HOME;
					gGripState = MODULE_STATE_HOMING;
					gGripTim = 0;
					flg = 1;
					test_speed = new_speed;
				}
			}
		}
	}
	//过流保护
	if (ABS(new_current) > gGripMotor.motor_desp.current_max_ma  && gGripTim > 1000)
	{
		DJ_Motor_QuickStop(&gGripMotor);
	}
}
