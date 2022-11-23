/**
  ******************************************************************************
  * @file    drv_encoder.h
  * @author  lyq
  * @version V1.0.0
  * @date    2015-12-08
  * @brief   The file defines the interface of the encoder
					2020.5.19进行了修改和测试，将计时定时器修改为TIM7.目前编码器
					只能使用TIM3，接PA6和PA7，采样频率为4KHz,最低可测频率为50Hz。
  ******************************************************************************
  * @copy
  */
#ifndef  USER_ENCODER_H
#define   USER_ENCODER_H

#include "drivers_cfg.h"

#define ENC_DIR_FORWARD			1
#define ENC_DIR_BACKWARD		(-1)

//编码器初始化
extern void Encoder_Init(void);
//获取编码器的绝对脉冲个数(4倍频以后的脉冲数)
extern uint32_t Encoder_GetAbsoluteCnts(void);
//获取编码器的相对脉冲个数(4倍频以后的脉冲数)
extern int32_t Encoder_GetRelativeCnts(void);
//获取以cnt/s描述的电机转速(4倍频以后的速度)
extern int32_t Encoder_GetSpeed(void);
//设置当前脉冲数
extern void Encoder_SetCurrentCnts(int32_t cnts);
//extern void Encoder_UpdateData(void);

#endif
