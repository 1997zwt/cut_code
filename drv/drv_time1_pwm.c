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
#include "drv_time1_pwm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#ifdef USE_TIME1_PWM

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Time1Peroid;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void PR_Time1PWMInit(uint32_t Frequency)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  //使能定时器1时钟
    #ifdef ENABLE_TIM1_FullRemap
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE); //Timer1完全重映射
    /* Initialize TIM1_PWM ports - PE9,PE11,PE13,PE14*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    
    #ifdef ENABLE_TIM1_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    
    #else  //#ifdef ENABLE_TIM1_FullRemap

    /* Initialize TIM1_PWM ports - PA8,PA9,PA10,PA11*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    #ifdef ENABLE_TIM1_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM1_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #endif  //#ifdef ENABLE_TIM1_FullRemap
    
	//初始化TIM1	
	if (Frequency < 2000)
	{
		Time1Peroid = 1000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 72;
	}
	else
	{
		Time1Peroid = 72000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
	}
    TIM_TimeBaseInitStruct.TIM_Period = Time1Peroid - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
    
    //初始化TIM1 PWM模式
    #ifdef ENABLE_TIM1_CH1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器
	TIM_SetCompare1(TIM1, 0);
    #endif
    
    #ifdef ENABLE_TIM1_CH2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR2上的预装载寄存器
	TIM_SetCompare2(TIM1, 0);
    #endif
    
    #ifdef ENABLE_TIM1_CH3
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR3上的预装载寄存器
	TIM_SetCompare3(TIM1, 0);
    #endif
    
    #ifdef ENABLE_TIM1_CH4
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR4上的预装载寄存器
	TIM_SetCompare4(TIM1, 0);
    #endif
    
 	TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);  //使能TIM1

}

#ifdef ENABLE_TIM1_CH1
void PR_Time1CH1PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare1(TIM1, DutyRadio * Time1Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM1_CH2
void PR_Time1CH2PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare2(TIM1, DutyRadio * Time1Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM1_CH3
void PR_Time1CH3PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare3(TIM1, DutyRadio * Time1Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM1_CH4
void PR_Time1CH4PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare4(TIM1, DutyRadio * Time1Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#endif	//#ifdef USE_TIME1_PWM
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
