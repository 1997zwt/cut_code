#include "user_tasks.h"

OS_EVENT *ModbusSem; /*Modbus指令到达信号量*/
void Task_Start(void *p_arg)
{	
	InitAllDevice();
	OSTaskCreate(Task_Protocol, NULL, (OS_STK *)&protol_task_stk[PROTO_TASK_STK_SIZE - 1], PROTO_TASK_PRIO);
	OSTaskCreate(Task_Tool, NULL, (OS_STK *)&tool_task_stk[TOOL_TASK_STK_SIZE - 1], TOOL_TASK_PRIO);
#ifdef _SMCL_MOTOR_H
	OSTaskCreate(Task_SmclProtocol, NULL, (OS_STK *)&smcl_protol_task_stk[TOOL_TASK_STK_SIZE - 1], SMCL_TASK_PRIO);
#endif

	while(1)
	{
		IWDG_Feed();
		OSTimeDly(100);


//		if(APPRUN_FLAG_DATA != STMFLASH_ReadHalfWord(IAP_FLAG_ADDR + 10))
//		{
//			FLASH_Unlock();
//			DRV_FLASH_Write(IAP_FLAG_ADDR + 10, &init_data_a, 1);
//			FLASH_Lock();
//		}
	}
}

