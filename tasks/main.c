#include "tasks/user_tasks.h"

OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE];

int main(void)
{
    SysTick_init();
    OSInit();

    /*创建初始任务 用于创建各任务*/
    OSTaskCreate(Task_Start, NULL, (OS_STK *)&startup_task_stk[STARTUP_TASK_STK_SIZE - 1], STARTUP_TASK_PRIO);
    OSStart(); 

    return  0;
}



