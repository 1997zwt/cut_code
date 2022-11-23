#include "steering_motor.h"

#if (defined STEERING_MOTOR_NUM) && (STEERING_MOTOR_NUM > 0)

//以下为接口函数
bool Steering_Motor_Init(stSteeringMotorDef* motor)
{
	if(motor->index >= STEERING_MOTOR_NUM)
		return FALSE;		

	//没有定义PWM接口函数
	if (motor->SetPwmFun == NULL)		
		return FALSE;

	Steering_Motor_MoveTo(motor,motor->origin);

	return TRUE;
}
void Steering_Motor_MoveTo(stSteeringMotorDef*motor, int16_t angle_1_d)
{
	if(motor->type != STEERING_MOTOR_TYPE_ANGLE)  return;

	Range(angle_1_d,motor->min,motor->max);
	angle_1_d = angle_1_d + motor->offset;
	motor->current = angle_1_d;
	//以下根据舵机控制方法计算得到
	angle_1_d = (int32_t)angle_1_d*PWM_DUTY_RANGE_DEF*2/3/STEERING_MOTOR_ANGLE_RANGE;
	motor->SetPwmFun(angle_1_d);
}
void Steering_Motor_Move(stSteeringMotorDef* motor, int16_t speed_1_dps)
{
	if(motor->type != STEERING_MOTOR_TYPE_SPEED)  return;

	Range(speed_1_dps,motor->min,motor->max);
	speed_1_dps = speed_1_dps + motor->offset;
	motor->current = speed_1_dps;
	motor->SetPwmFun((int32_t)speed_1_dps*PWM_DUTY_RANGE_DEF/STEERING_MOTOR_SPEED_RANGE);
}
#endif
