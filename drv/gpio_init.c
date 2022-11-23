/**
  ******************************************************************************
  * @file    gpio_init.c
  * @author  
  * @version 
  * @date   
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
  
#include "gpio_init.h"   

 /**
  * @brief  初始化控制的IO
  * @param  无
  * @retval 无
  */
void GPIO_Config(void)
{		
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( SENSOR_GPIO_CLK | POWIN_GPIO_CLK | POWON_GPIO_CLK | BT_LED_GPIO_CLK | RELAY1_GPIO_CLK | RELAY2_GPIO_CLK | STATELED_GPIO_CLK, ENABLE);
    
/******************************SENSOR传感器****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = SENSOR_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(SENSOR_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************POWER_IN电源按键检测****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = POWIN_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(POWIN_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************POWER_ON电源引脚输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = POWON_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(POWON_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************BT_LED电源LED输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = BT_LED_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(BT_LED_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************RELAY1输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = RELAY1_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(RELAY1_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************RELAY2输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = RELAY2_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(RELAY2_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************状态指示灯输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = STATELED_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(STATELED_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/






}

/*********************************************END OF FILE**********************/
