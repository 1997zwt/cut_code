#include "drv_adc1.h"

#if (defined ADC_CHANNEL_NUM) && (ADC_CHANNEL_NUM > 0)

//10个通道的采样时间大约是30us
#define  ADC_DATA_NUM						100				//每个通道采用的数据个数

//定义ADC的数据缓冲区
uint16_t  ADC_Value[ADC_DATA_NUM][ADC_CHANNEL_NUM] = {0}; 
uint16_t  ADC_offset[ADC_CHANNEL_NUM] = {0};

void ADC1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	//下面初始化ADC时钟（最大14MHz）
	/* ADCCLK = PCLK2/8 = 9MHz*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	/* Enable ADC1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

#ifdef ADC1_IN0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, ADC1_IN0+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, ADC1_IN1+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_2, ADC1_IN2+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3, ADC1_IN3+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_4, ADC1_IN4+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_5, ADC1_IN5+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_6, ADC1_IN6+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_7, ADC1_IN7+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8, ADC1_IN8+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_9, ADC1_IN9+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_10, ADC1_IN10+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_11, ADC1_IN11+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12, ADC1_IN12+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_13, ADC1_IN13+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN14
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_14, ADC1_IN14+1, ADC_SampleTime_13Cycles5);	
#endif
#ifdef ADC1_IN15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_15, ADC1_IN15+1, ADC_SampleTime_13Cycles5);	
#endif

	//初始化ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					//独立模式，双ADC不工作
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;							//扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;					//连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//禁止触发模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNEL_NUM;	//规则组长度
	ADC_Init(ADC1, &ADC_InitStructure);	

	//DMA模式使能
	ADC_DMACmd(ADC1, ENABLE);
	//DMA初始化
	DMA_DeInit(DMA1_Channel1);																		//ADC1映射到DMA通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;		//DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value;			//DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向：外设->内存
	DMA_InitStructure.DMA_BufferSize = ADC_DATA_NUM*ADC_CHANNEL_NUM;			//DMA通道的数据缓存的大小 

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		 //外设地址不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				 //内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;		//外设数据宽度为16位 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;				//内存数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;									 //工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;								//当前DMA通道拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                  //禁止DMA通道内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);											//初始化DMA通道 	

	//使能ADC1,CR2_ADON置1
	ADC_Cmd(ADC1, ENABLE);
	//ADC校准
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	DMA_Cmd(DMA1_Channel1, ENABLE);			//启动DMA通道
	//初始化顺序不能变，一定在初始化完成后再启动转换，否则通道数据会出错
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

//平均值滤波
//获取AD转换的结果
uint16_t ADC1_GetValue(uint8_t  channel)
{
	uint16_t line = 0;
	uint32_t	sum = 0;
	uint16_t minval=0xffff,maxval=0;

	if(channel > ADC_CHANNEL_NUM-1)  return 0;

	for (line=0;line < ADC_DATA_NUM;line++)
	{
		if (ADC_Value[line][channel] < minval)
		{
			minval = ADC_Value[line][channel];
		}
		if (ADC_Value[line][channel] > maxval)
		{
			maxval = ADC_Value[line][channel];
		} 
		sum += ADC_Value[line][channel];
	}

	return  (uint16_t)((sum-minval-maxval)/(ADC_DATA_NUM-2));
}
int16_t ADC1_GetValue_mv(uint8_t  channel)
{
	if(channel > ADC_CHANNEL_NUM-1)  return 0;

	return ADC1_GetValue(channel)*REF_VOLTAGE/ADC1_PRECISION - ADC_offset[channel];
}
int16_t ADC1_SetOffset(uint8_t channel)
{
	ADC_offset[channel] = ADC1_GetValue(channel)*REF_VOLTAGE/ADC1_PRECISION;
	return ADC_offset[channel];
}
#endif
