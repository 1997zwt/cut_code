/**
  ******************************************************************************
  * @file    drv_hallpos.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-08-19
  * @brief   该驱动文件用于舵机霍尔信号做位置反馈的位置采集，可用于其他采用IO
  中断方式进行位置反馈的应用。
  ******************************************************************************
  * @copy
  */
#ifndef  _DRV_HALLPOS_H
#define   _DRV_HALLPOS_H

#include "stm32f10x.h"

void Hall_Init(void);
uint32_t GetHallPos(void);
void SetHallPos(uint32_t newpos);
#endif
