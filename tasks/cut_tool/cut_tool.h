/**
  ******************************************************************************
  * @file    cut_tool.h
  * @author  lyq
  * @version V2.0.1
  * @date    2020-04-24
  * @brief   这是需要用户自己编写的应用程序，实现用户自己的控制逻辑和功能
				   用户只需要关注自己的驱动配置和控制协议
				   2020.5.9 按照驱动版本2.01修改了断线逻辑，增加了钢芯线支持
				   2020.5.9 测试稳定，能够剪断各种线径铝线，钢芯线，具备无线空剪
				   自动保护功能，具备低速找线自动停止功能
				   2020.7.9 增加夹紧电机支持，采用第三代协议;增加断线成功后自动复位功能；
				   增加过流保护功能。
				   2020.7.13 增加第二版协议支持，通过修改user_task.h中的宏定义USE_PROTOCOL_V2
				   可更改为使用第二版协议
  ******************************************************************************
  * @copy
  */
#ifndef _CUT_TOOL_H
#define _CUT_TOOL_H

//配置断线工具的驱动
#include "driver_cfg_cut.h"
//断线工具的协议
#include "protocols/protocol.h"
//通用定义文件
#include "../user_define.h"
#include "grip.h" 
#include "lidischarge.h"

struct home_detect
{
	int32_t aver_curent;
	uint32_t betime;
	uint32_t motiontim;
	uint8_t  flg;
};


#define CAMERA_GPIO_PORT    	GPIOA			              /* GPIO端口 */
#define CAMERA_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define CAMERA_GPIO_PIN		    GPIO_Pin_2		

#define SENSOR1_GPIO_PORT    	GPIOA			              /* GPIO端口 */
#define SENSOR1_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define SENSOR1_GPIO_PIN		GPIO_Pin_6			        

#define SENSOR2_GPIO_PORT    	GPIOA			              /* GPIO端口 */
#define SENSOR2_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define SENSOR2_GPIO_PIN		GPIO_Pin_5			        


#define POWIN_GPIO_PORT    	    GPIOB			              /* GPIO端口 */
#define POWIN_GPIO_CLK 	        RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define POWIN_GPIO_PIN		    GPIO_Pin_6       

#define POWON_GPIO_PORT    	    GPIOB			              /* GPIO端口 */
#define POWON_GPIO_CLK 	        RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define POWON_GPIO_PIN		    GPIO_Pin_7       

#define BT_LED_GPIO_PORT    	GPIOB			              /* GPIO端口 */
#define BT_LED_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define BT_LED_GPIO_PIN		    GPIO_Pin_0       

#define STATELED_GPIO_PORT    	GPIOB		              /* GPIO端口 */
#define STATELED_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define STATELED_GPIO_PIN		GPIO_Pin_14

#define WIFI_RELOAD_GPIO_PORT  GPIOB
#define WIFI_RELOAD_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define WIFI_RELOAD_PIN				GPIO_Pin_13	

#define WIFIBUTTON_PIN        GPIO_Pin_1
#define isWifiButtonDown()  (GPIOB->IDR & WIFIBUTTON_PIN)

#define DisablePowerShut()						GPIOB->BSRR = POWON_GPIO_PIN;
#define EnablePowerShut()					GPIOB->BRR = POWON_GPIO_PIN;

#define TOOL_CONTROL_PORT				USART1
#define TOOL_CONTROL_BUF				BUF_USART1
#define BATTARY_VOLTAGE_CHANNEL		ADC1_IN4
//#define GRIP_MOTOR_CURRENT_CHANNEL  ADC1_IN4
#define MV_TO_BATTARY								(11)				//毫伏电压转电池电压
#define MOTOR_CURRENT_CHANNEL			ADC1_IN3
#define MOTOR_CURRENT_OFFSET				(0)						//电机电流零值校准
//采样电阻为1K时值为8.5，采样电阻为500时值为17
#define MV_TO_MOTOR_MA			 (17)	 //50AB用42  20AB用17

#define MOTOR_CUT				TOOL_MOTOR_1
#define MOTOR_GRIP			TOOL_MOTOR_2
#define CAMERA_POWER		TOOL_MOTOR_7

//断线工具的接口函数
void CutTool_Control(uint8_t motor, uint8_t cmd, uint8_t* data);
uint8_t CutTool_GetState(void);
void StateLEDFlash(uint16_t period);
void PowerButtonScan(void);
void PowerLEDFlash(uint16_t period);
void SoftTim1ms_callback(void *ptmr, void *parg);

void  delay_ms(uint16_t ms);
//配置工具的任务,不用改
#define TOOL_TASK_STK_SIZE							300
extern OS_STK tool_task_stk[TOOL_TASK_STK_SIZE]; 
void Task_Tool(void *p_arg);
extern ToolStateDef tool_state;

#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
