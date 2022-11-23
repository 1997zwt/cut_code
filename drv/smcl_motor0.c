#include "smcl_motor.h"

#if (defined SMCL_MOTOR_NUM) && (SMCL_MOTOR_NUM > 0)

static stSmclMotorDef*  gSmclMotor[SMCL_MOTOR_NUM];

//以下为私有函数
static void _smcl_SendData(stSmclMotorDef* motor, uint8_t* data, uint8_t len)
{
	uint8_t check = 0;
	uint8_t i = 0;

	for(i=2;i<len;i++)
	{
		check += data[i];
	}
	check = ~check;
	data[len-1] = check;
	UART_SendBuf(SMCL_CONTROL_PORT, data, len);
	delay_ms(2);
}

//以下为接口函数
bool Smcl_Motor_Init(stSmclMotorDef* motor)
{
	uint8_t data[20] = {0xff,0xff,0,8,SMCL_CMD_WRITE,0x09,0,0,0,0,0,0};
	int16_t tmp = 0;

	if(motor->motor_index >= SMCL_MOTOR_NUM)  return FALSE;

	USART_Config(SMCL_CONTROL_PORT,SMCL_BAUDRATE);
	delay_ms(5);

	data[2] = motor->com_id;
	tmp = motor->min_angle*4095/3600;		//角度0~3600（0.1度）转换到0~0xFFF
	data[6] = (uint8_t)tmp;
	data[7] = (uint8_t)(tmp>>8);
	tmp = motor->max_angle*4095/3600;
	data[8] = (uint8_t)tmp;
	data[9] = (uint8_t)(tmp>>8);
	data[10] = motor->max_temperature;
	_smcl_SendData(motor,data,12);
	delay_ms(5);
	Smcl_Motor_SetMode(motor,motor->mode);
	gSmclMotor[motor->motor_index] = motor;

	return TRUE;
}
void Smcl_Motor_SetPwm(stSmclMotorDef*motor, int16_t pwm)
{
	uint8_t data[20] = {0xff,0xff,0,5,SMCL_CMD_WRITE,0x2C,0,0,0};

	pwm = pwm*motor->dir_def;
	Limit(pwm, motor->max_pwm);
	if (pwm < 0)
	{
		pwm = ABS(pwm);
		pwm |= (1<<10);									//bit15=1表示反转
	}
	data[2] = motor->com_id;
	data[6] = (uint8_t)pwm;
	data[7] = (uint8_t)(pwm>>8);

	_smcl_SendData(motor,data,9);
}
void Smcl_Motor_Move(stSmclMotorDef*motor, int16_t velocity_0_1_dps)
{
	uint8_t data[20] = {0xff,0xff,0,5,SMCL_CMD_WRITE,0x2E,0,0,0};

	velocity_0_1_dps = velocity_0_1_dps*motor->dir_def;
	Limit(velocity_0_1_dps, motor->max_speed);
	velocity_0_1_dps = velocity_0_1_dps*100/87;			//与协议的速度单位不同，协议为0.087度每秒
	if (velocity_0_1_dps < 0)
	{
		velocity_0_1_dps = ABS(velocity_0_1_dps);
		velocity_0_1_dps |= 0x8000;									//bit15=1表示反转
	}

	data[2] = motor->com_id;
	data[6] = (uint8_t)velocity_0_1_dps;
	data[7] = (uint8_t)(velocity_0_1_dps>>8);

	_smcl_SendData(motor,data,9);
}

void Smcl_Motor_MoveTo(stSmclMotorDef*motor, int16_t velocity_0_1_dps, int16_t angle_0_1_d)
{
	uint8_t data[20] = {0xff,0xff,0,9,SMCL_CMD_WRITE,0x2A,0,0,0,0,0,0,0};

	RLimit(velocity_0_1_dps, motor->max_speed);
	Range(angle_0_1_d,motor->min_angle,motor->max_angle);
	velocity_0_1_dps = velocity_0_1_dps*100/87;			//与协议的速度单位不同，协议为0.087度每秒	
	angle_0_1_d = angle_0_1_d*4095/3600;					//协议0~4095代表0~360度
	if (velocity_0_1_dps < 0)
	{
		velocity_0_1_dps = ABS(velocity_0_1_dps);
		velocity_0_1_dps |= 0x8000;									//bit15=1表示反转
	}
	data[2] = motor->com_id;
	data[6] = (uint8_t)angle_0_1_d;
	data[7] = (uint8_t)(angle_0_1_d>>8);
	data[10] = (uint8_t)velocity_0_1_dps;
	data[11] = (uint8_t)(velocity_0_1_dps>>8);

	_smcl_SendData(motor,data,13);
}
void Smcl_Motor_SetMode(stSmclMotorDef*motor, SmclOpModedef mode)
{	
#ifdef USE_SMCL_MOTOR
	uint8_t data[20] = {0xff,0xff,0,4,SMCL_CMD_WRITE,0x23,0,0};
#endif
#ifdef USE_SMBL_MOTOR
	uint8_t data[20] = {0xff,0xff,0,4,SMCL_CMD_WRITE,0x21,0,0};
#endif
	
	data[2] = motor->com_id;
	data[6] = (uint8_t)mode;
	_smcl_SendData(motor,data,8);
}
void Smcl_Motor_UpdateState(stSmclMotorDef*motor)
{
	uint8_t data[20] = {0xff,0xff,0,4,SMCL_CMD_READ,0x38,15,0};
	
	data[2] = motor->com_id;
	_smcl_SendData(motor,data,8);
}

OS_STK smcl_protol_task_stk[SMCL_TASK_STK_SIZE]; 
void Task_SmclProtocol(void *p_arg)
{
	uint8_t databuf[50] = {0};
	uint8_t data=0, index=0, len=0, check=0;
	uint8_t id=0;

	for(;;)
	{
		WaitUsartDataUpdate(SMCL_CONTROL_BUF); 		
		if(USART_FrameBufRead(SMCL_CONTROL_BUF, &data))
		{
			databuf[index] = data;
			if (index == 0)
			{
				if(data == 0xff)	index++;
				else index = 0;
			}
			else if (index == 1)
			{
				if(data == 0xff)	index++;
				else index = 0;
			}
			else if (index == 2)
			{
				id = data;
				check = data;
				index++;
			}
			else if (index == 3)
			{
				len = data + 4;
				check += data;
				index++;
			}
			else if (index < len - 1)
			{
				check += data;
				index++;
			}
			else if(index == len - 1)
			{
				check = ~check;
				if (check == data)
				{
					if (len == 21)			//状态反馈
					{
						for (index=0;index<SMCL_MOTOR_NUM;index++)
						{
							if (gSmclMotor[index]->com_id == id)
							{
								gSmclMotor[index]->state.state.angle = (databuf[5] + (databuf[6]<<8))*3600/4095;//转换到0.1度
								gSmclMotor[index]->state.state.velocity = (databuf[7] + (databuf[8]<<8));
								if (gSmclMotor[index]->state.state.velocity & 0x8000)//负值
								{
									gSmclMotor[index]->state.state.velocity = 0 - (gSmclMotor[index]->state.state.velocity & 0x7FFF);
								}
								gSmclMotor[index]->state.state.velocity = gSmclMotor[index]->state.state.velocity;//*87/100;//转换到0.1度/s
								gSmclMotor[index]->state.state.voltage = databuf[11];
								gSmclMotor[index]->state.state.temprature = databuf[12];
								gSmclMotor[index]->state.state.current = databuf[18] + (databuf[19]<<8);
							}
						}
					}
					else if (len == 6)		//命令反馈
					{
						for (index=0;index<SMCL_MOTOR_NUM;index++)
						{
							if (gSmclMotor[index]->com_id == id)
							{
								gSmclMotor[index]->state.state.error = databuf[4];
							}
						}
					}
				}
				index = 0;
			}	
			else
			{
				index = 0;
			}	
		}
	}
}


#endif
