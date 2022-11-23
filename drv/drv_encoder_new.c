/**
  ******************************************************************************
  * @file    drv_encoder.c
  * @author  lyq
  * @version V1.0.0
  * @date    2020*02-27
  * @brief   The file calculate the pulse and speed of the motor
					2020.5.27，采用了新的编码器速度采样方法，采样频率约为1K
  ******************************************************************************
  * @copy
  */
#include "drv_encoder.h"

//定时器7用于进行编码器脉冲计时
//TIM7使用系统时钟
#define		TIM7_PSC_INIT			0
#define		TIM7_FCLK				(SystemCoreClock)

//改变TIM7_OVERFLOW_MAX可以改变该程序可以获取的电机最低旋转速度(cnt/s)
//最低转速大约是：1000/TIM7_OVERFLOW_MAX  (cnt/s)
#define		TIM7_OVERFLOW_MAX		5			//最低200Hz即编码器50Hz

//编码器脉冲预设值，计数值达到该值时触发溢出中断
#define		TIM3_ARR_INIT			1000
#define		 BUF_MAX						10

int32_t	g_tim7_overflow = 0;							//定时器7的溢出次数
int32_t	g_tim3_overflow = 0;							//定时器7的溢出次数
int64_t	g_EncoderAbsCnts = 0;
int64_t	g_EncoderAbsCnts_previous = 0;		//前一次脉冲数
int32_t		g_EncoderRelCnts = 0;
int8_t		g_RotDirection = ENC_DIR_FORWARD;

static int32_t	g_SpeedBuf[BUF_MAX] = {0};
uint8_t	 g_BufIndex = 0;
int32_t	 g_curr_speeed = 0;

void Encoder_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//编码器连接定时器3   
	//定时器3用于测脉冲数(M法)定时器7用于测时间(T法)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7 | RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

	//使用PA口的PA6和PA7
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//定时器3全局中断,溢出中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//定时器7全局中断,溢出中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);

	//初始化编码器定时器的时基单元
	TIM_TimeBaseStructure.TIM_Prescaler = 0x00;  // 
	TIM_TimeBaseStructure.TIM_Period = TIM3_ARR_INIT-1;  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	//编码器输入滤波设置
	TIM_ICStructInit(&TIM_ICInitStructure);			
	TIM_ICInitStructure.TIM_ICFilter = 6;				//设置滤波器
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	//设置计数模式和脉冲极性
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	//初始化定时器7的时基单元
	//TIM7的时钟频率为72M
	TIM_TimeBaseStructure.TIM_Prescaler = TIM7_PSC_INIT;  // 
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
	TIM_InternalClockConfig(TIM7);

	TIM_ClearFlag(TIM7, TIM_FLAG_Update);
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM7, ENABLE); 
	TIM_Cmd(TIM3, ENABLE);
}
//中位值平均滤波
int32_t  MedAverage(int32_t* data , uint8_t len)
{
	uint8_t i =0;
	int32_t sum = 0;
	int32_t max = 0,min = (s32)2147483647;
	uint8_t zeros = 0;

	for (i=0;i<len;i++)
	{
		if(data[i] == 0)
		{
			zeros++;		
			continue;
		}
		if (data[i] > max)
		{
			max = data[i];
		}
		if(data[i] < min)
		{
			min = data[i];
		}
		sum += data[i];
	}
	if(zeros >= len-2)
		return 0;
	else
		return (sum - min - max)/(len - zeros -2);
}
/*********************************/
//对外接口函数
//不考虑电机转向脉冲累加
uint32_t Encoder_GetAbsoluteCnts(void)
{
	return g_EncoderAbsCnts;
}
//考虑电机转向的脉冲数，正向增加，反向减小
int32_t Encoder_GetRelativeCnts(void)
{
	return g_EncoderRelCnts;
}
//设置当前脉冲数
void Encoder_SetCurrentCnts(int32_t cnts)
{
	g_EncoderRelCnts = cnts;
}
uint32_t test_cnt=0;
//用脉冲数每秒描述的转速，带方向
int32_t Encoder_GetSpeed(void)
{
	g_curr_speeed = (MedAverage(g_SpeedBuf,BUF_MAX)+ g_curr_speeed)>>1;	//相当于a=0.5的一阶低通滤波

	return (g_curr_speeed*g_RotDirection);
}
/*********************************/ 
void TIM3_IRQHandler(void)
{  
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	if (g_tim3_overflow++ == (uint32_t)0x0FFFFFFF)
	{
		g_tim3_overflow = 0;
	}
}
//0.91毫秒左右
void TIM7_IRQHandler(void)
{  
	uint32_t d_pulse = 0;

	TIM_ClearFlag(TIM7, TIM_FLAG_Update);
	g_EncoderAbsCnts = (uint64_t)g_tim3_overflow*1000 + TIM3->CNT;
	d_pulse = ABS(g_EncoderAbsCnts - g_EncoderAbsCnts_previous);
	g_EncoderAbsCnts_previous = g_EncoderAbsCnts;
	if (d_pulse > 0)
	{
		g_SpeedBuf[g_BufIndex++] = (((uint64_t)d_pulse*TIM7_FCLK)>>16)/(g_tim7_overflow+1);
		g_tim7_overflow = 0;
		if (((TIM3->CR1)&0x0010) == 0)
		{
			g_RotDirection = ENC_DIR_BACKWARD;
			g_EncoderRelCnts -= d_pulse;
		}
		else
		{
			g_RotDirection = ENC_DIR_FORWARD;
			g_EncoderRelCnts += d_pulse;
		}
	}
	else
	{
		if (g_tim7_overflow++ >= TIM7_OVERFLOW_MAX)
		{
			for(g_BufIndex=0;g_BufIndex<BUF_MAX;g_BufIndex++)
			{
				g_SpeedBuf[g_BufIndex] = 0;
			}
		}	
	}
	if (g_BufIndex == BUF_MAX)
	{
		g_BufIndex = 0;
	}
}

