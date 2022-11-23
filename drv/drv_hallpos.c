#include "drv_hallpos.h"

static uint32_t gHallCnts = 0;

void Hall_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStructure;  
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	//PA6,PA7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	//PB0
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
    
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource6);//开启中断线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);

	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line6 | EXTI_Line7;               //中断线选择
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      //EXTI为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                //使能中断
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;        //中断源
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;        //中断源
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;        //中断源
	NVIC_Init(&NVIC_InitStructure);
}

 void EXTI0_IRQHandler(void)
{
	gHallCnts++;
	EXTI_ClearITPendingBit(EXTI_Line0);
 }
 void EXTI9_5_IRQHandler(void)
 {
	 gHallCnts++;
	 EXTI_ClearITPendingBit(EXTI_Line6);
	 EXTI_ClearITPendingBit(EXTI_Line7);
 }
 uint32_t GetHallPos(void)
 {
	 return gHallCnts;
 }
 void SetHallPos(uint32_t newpos)
 {
	 gHallCnts = newpos;
 }
 