/**
  ******************************************************************************
  * @file    drv_dog.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-26
  * @brief   独立看门狗驱动
******************************************************************************
* @copy
*/
#ifndef DRV_DOG_H
#define DRV_DOG_H

#include "drivers_cfg.h"

//看门狗初始化
//time_ms: 以ms为单位的超时时间
void IWDG_Init(uint16_t time_ms);
void IWDG_Feed(void);
#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/
