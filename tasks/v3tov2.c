//实现第二版协议到第三版协议的转换

#include "user_tasks.h"

#ifdef USE_PROTOCOL_V2
#ifdef _CUT_TOOL_H
extern ToolStateDef tool_state;
extern ToolMotorStateDef  cut_motor_state,gGripMotorState;

uint16_t V3ToV2(void)
{
	uint16_t state=0;

	if (tool_state == TOOL_STATE_WORK)
	{
		state |= 0x40;
	}
	else if (tool_state == TOOL_STATE_FAILURE)
	{
		state |= 0xC0;
	}
	else //if (tool_state == TOOL_STATE_SUCCEED)
	{
		state |= 0x80;
	}
	if (cut_motor_state.work_state == WORK_STATE_HOME )
	{
		if (cut_motor_state.motion_state == MOTION_STATE_CCW)
		{
			state |= 0x10;
		}
		else if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= 0x20;
		}
	}
	if (cut_motor_state.work_state == WORK_STATE_WORK )
	{
		if (cut_motor_state.motion_state == MOTION_STATE_CW)
		{
			state |= 0x08;
		}
	}
	if (gGripMotorState.work_state == WORK_STATE_WORK)
	{
		if (gGripMotorState.motion_state == MOTION_STATE_CW)
		{
			state |= 0x01;
		}
		else if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
		{
			state |= 0x03;
		}
	}
	if (gGripMotorState.work_state == WORK_STATE_HOME)
	{
		if (gGripMotorState.motion_state == MOTION_STATE_CCW)
		{
			state |= 0x02;
		}
		else if (gGripMotorState.motion_state == MOTION_STATE_STOPPED)
		{
			state |= 0x04;
		}
	}

	return state;
}

#endif

#ifdef _STRIP_TOOL_H
extern ToolStateDef tool_state;
extern ToolMotorStateDef cutter_motor_state, grip_l_motor_state, grip_r_motor_state,strip_motor_state, slider_motor_state;
uint16_t V3ToV2(void)
{
	uint16_t state=0;

	if (tool_state == TOOL_STATE_SUCCEED)
	{
		state |= (1<<9);
	}

	if (grip_l_motor_state.work_state == WORK_STATE_WORK)
	{
		if (grip_l_motor_state.motion_state == MOTION_STATE_CW)
		{
			state |= (0x02<<6);
		}
		else if (grip_l_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x05<<6);
		}
	}
	else if (grip_l_motor_state.work_state == WORK_STATE_HOME)
	{
		if (grip_l_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<6);
		}
		else 		
		{
			state |= (0x03<<6);
		}
	}
	else if (grip_l_motor_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x04<<6);
	}

	if (grip_r_motor_state.work_state == WORK_STATE_WORK)
	{
		if (grip_r_motor_state.motion_state == MOTION_STATE_CW)
		{
			state |= 0x02;
		}
		else if (grip_r_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= 0x05;
		}
	}
	else if (grip_r_motor_state.work_state == WORK_STATE_HOME)
	{
		if (grip_r_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= 0x01;
		}
		else
		{
			state |= 0x03;
		}
	}
	else if (grip_r_motor_state.work_state == WORK_STATE_IDLE)
	{
		state |= 0x04;
	}

	if (cutter_motor_state.work_state == WORK_STATE_WORK)
	{ 
		if (cutter_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x05<<3);
		}
		else
		{
			state |= (0x02<<3);
		}
	}
	else if (cutter_motor_state.work_state == WORK_STATE_HOME)
	{
		if (cutter_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<3);
		}
		else
		{
			state |= (0x03<<3);
		}
	}
	else if (cutter_motor_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x04<<3);
	}

	if (slider_motor_state.work_state == WORK_STATE_WORK)
	{
		if (slider_motor_state.motion_state == MOTION_STATE_CW)
		{
			state |= (0x02<<13);
		}
		else if (IsSliderEndPos() && slider_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x05<<13);
		}
	}
	else if (slider_motor_state.work_state == WORK_STATE_HOME)
	{
		if (slider_motor_state.motion_state == MOTION_STATE_CCW)
		{
			state |= (0x03<<13);
		}
		else if ( IsSliderStartPos() && slider_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<13);
		}
	}
	else if (slider_motor_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x04<<13);
	}

	if (strip_motor_state.work_state == WORK_STATE_WORK)
	{
		if (strip_motor_state.motion_state == MOTION_STATE_CW)
		{
			state |= (0x02<<10);
		}
	}
	else if (strip_motor_state.work_state == WORK_STATE_HOME)
	{
		if (strip_motor_state.motion_state == MOTION_STATE_CCW)
		{
			state |= (0x03<<10);
		}
		else if (/*IsStripMotorHome() && */strip_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<10);
		}
	}
	else if (strip_motor_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x04<<10);
	}

	return state;
}
#endif

#ifdef _WIRING_TOOL_H
extern ToolStateDef tool_state;
extern ToolMotorStateDef gripL_state, gripR_state,screw_state,clamp_state,sleeve_state;
uint16_t V3ToV2(void)
{
	uint16_t state=0;

	if (tool_state == TOOL_STATE_SUCCEED)
	{
		state |= (3<<9);
	}
	else if (tool_state == TOOL_STATE_FAILURE)
	{
		state |= (2<<9);
	}
	else if (tool_state == TOOL_STATE_WORK)
	{
		state |= (1<<9);
		state |= (1<<5);
	}

	if (gripL_state.work_state == WORK_STATE_WORK)
	{
		if (gripL_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x03<<11);
		}
	}
	else if (gripL_state.work_state == WORK_STATE_HOME)
	{
		if (gripL_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<11);
		}
	}
	else if (gripL_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x02<<11);
	}

	if (gripR_state.work_state == WORK_STATE_WORK)
	{
		if (gripR_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x03<<3);
		}
	}
	else if (gripR_state.work_state == WORK_STATE_HOME)
	{
		if (gripR_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<3);
		}
	}
	else if (gripR_state.work_state == WORK_STATE_IDLE)
	{
		state |= (0x02<<3);
	}
	//套筒状态
	if (sleeve_state.work_state == WORK_STATE_WORK)
	{
		if (sleeve_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x03<<14);
		}
	}
	else if (sleeve_state.work_state == WORK_STATE_HOME)
	{
		if (sleeve_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<14);
		}
	}
	//线夹解锁
	if (clamp_state.work_state == WORK_STATE_HOME)
	{
		state |= (0x01<<13);
	}
	//拧断复位
	if (screw_state.work_state == WORK_STATE_HOME)
	{
		if (screw_state.motion_state == MOTION_STATE_STOPPED)
		{
			state |= (0x01<<8);
		}
	}
	else if (screw_state.work_state == WORK_STATE_WORK)
	{
		if (screw_state.motion_state == MOTION_STATE_CW)
		{
			state |= (1<<9);
		}
	}

	return state;
}
#endif
#endif
