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
#include "stm32f10x.h"
#include "drv_time3_pwm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#ifdef USE_TIME3_PWM

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Time3Peroid;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void PR_Time3PWMInit(uint32_t Frequency)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //使能定时器3时钟
    #ifdef ENABLE_TIM3_FullRemap
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3完全重映射
    /* Initialize TIM3_PWM ports - PC6,PC7,PC8,PC9*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    #ifdef ENABLE_TIM3_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    
    #else  //#ifdef ENABLE_TIM3_FullRemap
    #ifdef ENABLE_TIM3_PartialRemap
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射
    /* Initialize TIM3_PWM ports - PB4,PB5,PB0,PB1*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    #ifdef ENABLE_TIM3_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #else  //#ifdef ENABLE_TIM3_PartialRemap
    /* Initialize TIM3_PWM ports - PA6,PA7,PB0,PB1*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    #ifdef ENABLE_TIM3_CH1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH3
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    
    #ifdef ENABLE_TIM3_CH4
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #endif  //#ifdef ENABLE_TIM3_PartialRemap
    #endif  //#ifdef ENABLE_TIM3_FullRemap
    
    //初始化TIM3
	if (Frequency < 2000)
	{
		Time3Peroid = 1000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 72;
	}
	else
	{
		Time3Peroid = 72000000/Frequency;
		TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
	}
    TIM_TimeBaseInitStruct.TIM_Period = Time3Peroid - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    
    //初始化TIM3 PWM模式
    #ifdef ENABLE_TIM3_CH1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR1上的预装载寄存器
	TIM_SetCompare1(TIM3, 0);
    #endif
    
    #ifdef ENABLE_TIM3_CH2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	TIM_SetCompare2(TIM3, 0);
    #endif
    
    #ifdef ENABLE_TIM3_CH3
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR3上的预装载寄存器
	TIM_SetCompare3(TIM3, 0);
    #endif
    
    #ifdef ENABLE_TIM3_CH4
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR4上的预装载寄存器
	TIM_SetCompare4(TIM3, 0);
    #endif
    
 	TIM_Cmd(TIM3, ENABLE);  //使能TIM3

}

#ifdef ENABLE_TIM3_CH1
void PR_Time3CH1PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare1(TIM3, DutyRadio * Time3Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM3_CH2
void PR_Time3CH2PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare2(TIM3, DutyRadio * Time3Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM3_CH3
void PR_Time3CH3PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare3(TIM3, DutyRadio * Time3Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#ifdef ENABLE_TIM3_CH4
void PR_Time3CH4PWMSet(uint16_t DutyRadio)
{
    TIM_SetCompare4(TIM3, DutyRadio * Time3Peroid / PWM_DUTY_RANGE_DEF);
}
#endif

#endif  //#ifdef ENABLE_TIM3_PWM

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
