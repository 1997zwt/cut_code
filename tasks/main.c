#include "tasks/user_tasks.h"

OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE];

void PVD_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    /* Configure the PVD Level to 2.9V */
    PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
    /* Enable the PVD Output */
    PWR_PVDCmd(ENABLE);

    /* Configure EXTI Line to generate an interrupt on falling edge */
    EXTI_ClearITPendingBit(EXTI_Line16);
    EXTI_InitStructure.EXTI_Line = EXTI_Line16;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//rising是上升沿检测
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* NVIC configuration */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{
		#ifdef USE_PVD
		PVD_Init();
	  #endif	
    SysTick_init();
    OSInit();

    /*创建初始任务 用于创建各任务*/
    OSTaskCreate(Task_Start, NULL, (OS_STK *)&startup_task_stk[STARTUP_TASK_STK_SIZE - 1], STARTUP_TASK_PRIO);
    OSStart(); 

    return  0;
}



