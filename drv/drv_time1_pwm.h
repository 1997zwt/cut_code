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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIME1_PWM_H
#define __TIME1_PWM_H

/* Includes ------------------------------------------------------------------*/
#include "drivers_cfg.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
//#define ENABLE_TIM1_PWM
//#define ENABLE_TIM1_FullRemap
//#define ENABLE_TIM1_CH1
//#define ENABLE_TIM1_CH2
//#define ENABLE_TIM1_CH3
//#define ENABLE_TIM1_CH4

/* Private macro -------------------------------------------------------------*/
//统一用0~10000代表占空比0~100%
#define PWM_DUTY_RANGE_DEF      10000

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void PR_Time1PWMInit(uint32_t Frequency);
void PR_Time1CH1PWMSet(uint16_t DutyRadio);
void PR_Time1CH2PWMSet(uint16_t DutyRadio);
void PR_Time1CH3PWMSet(uint16_t DutyRadio);
void PR_Time1CH4PWMSet(uint16_t DutyRadio);

#endif /*__TIME1_PWM_H */
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

