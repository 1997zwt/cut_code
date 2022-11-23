/**
  ******************************************************************************
  * @file    drv_adc1.h
  * @author  lyq
  * @version V1.0.0
  * @date    2020-04-24
  * @brief   driver of ADC1
					2020.5.19修改，增加了ADC转换0值校准功能，通过该功能可将电流芯片
					的电压范围从0~5V转换到-2.5~2.5V，同时可校正电流0值。
  ******************************************************************************
  * @copy
  */

#ifndef   _DRV_ADC1_H
#define  _DRV_ADC1_H

#include "drivers_cfg.h"

#define ADC1_PRECISION			4096				//adc精度12位
#define REF_VOLTAGE					3300				//参考电压3.3V

//#define  ADC_CHANNEL_NUM		2				//使用的模拟通道数量
// #define ADC1_IN0		0
// #define ADC1_IN1		1
// #define ADC1_IN2		2
// #define ADC1_IN3		3
// #define ADC1_IN4		0
// #define ADC1_IN5		1
// #define ADC1_IN6		2
// #define ADC1_IN7		3
// #define ADC1_IN8		0
// #define ADC1_IN9		1
// #define ADC1_IN10		2
// #define ADC1_IN11		3
// #define ADC1_IN12		0
// #define ADC1_IN13		1
// #define ADC1_IN14		2
// #define ADC1_IN15		3

void ADC1_Config(void);
//返回ADC转换原始值
uint16_t ADC1_GetValue(uint8_t  channel);
//返回mV为单位的转换值
int16_t ADC1_GetValue_mv(uint8_t  channel);
//进行0值校准
int16_t ADC1_SetOffset(uint8_t channel);

#endif

