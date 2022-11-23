/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */  

/* Includes ------------------------------------------------------------------*/
#include "drv_time4_pwm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#ifdef USE_TIME4_PWM

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Time4Peroid;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void PR_Time4PWMInit(uint32_t Frequency)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //使能定时器4时钟
    /* Initialize TIM5_PWM ports - PA0,PA1,PA2,PA3*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    #ifdef ENABLE_TIM4_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM4_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM4_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM4_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    //初始化TIM4
	if (Frequency < 2000)
	{
		Time4Peroid = 1000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 72;
	}
	else
	{
		Time4Peroid = 72000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
	}
	TIM_TimeBaseInitStruct.TIM_Period = Time4Peroid - 1;	
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
    
    //初始化TIM4 PWM模式
    #ifdef ENABLE_TIM4_CH1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM5在CCR1上的预装载寄存器
	TIM_SetCompare1(TIM4, 0);
    #endif
    
    #ifdef ENABLE_TIM4_CH2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM5在CCR2上的预装载寄存器
	TIM_SetCompare2(TIM4, 0);
    #endif
    
    #ifdef ENABLE_TIM4_CH3
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM5在CCR3上的预装载寄存器
	TIM_SetCompare3(TIM4, 0);
    #endif
    
    #ifdef ENABLE_TIM4_CH4
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM5在CCR4上的预装载寄存器
	TIM_SetCompare4(TIM4, 0);
    #endif
    
 	TIM_CtrlPWMOutputs(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);  //使能TIM4

}

#ifdef ENABLE_TIM4_CH1
void PR_Time4CH1PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare1(TIM4, DutyRadio * Time4Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM4_CH2
void PR_Time4CH2PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare2(TIM4, DutyRadio * Time4Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM4_CH3
void PR_Time4CH3PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare3(TIM4, DutyRadio * Time4Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM4_CH4
void PR_Time4CH4PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare4(TIM4, DutyRadio * Time4Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#endif	//#ifdef USE_TIME4_PWM
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
