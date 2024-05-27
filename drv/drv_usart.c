#include "drv_usart.h"
#include "stdio.h"

USART_TypeDef* gUsartDef[5] = {USART1,USART2,USART3,UART4,UART5};
OS_EVENT* gUsartGetDataSem[5];
uint8_t err = 0;

/**********************************************/
vu8 FrameBuf[5][FRAME_BUF_MAX];                   /*帧缓存数组*/
vu8 FrameBufWptr[5] = {0, 0, 0, 0, 0};          /*缓冲区写指针*/
vu8 FrameBufRptr[5] = {0, 0, 0, 0, 0};          /*缓冲区读指针*/

static void USART_FrameBufWrite(enUartFrameBuf bufx);

#ifdef  USE_USART1
void USART1_Config(int baudrate, char ISR)
{
    /*定义GPIO初始化结构体 GPIO_InitStructure*/
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    /*定义NVIC初始化结构体 NVIC_InitStructure*/
    NVIC_InitTypeDef NVIC_InitStructure;
    /*选择NVIC优先级分组4( 0~15抢占优先级)*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,   ENABLE); 
    
    //******************************************************************************
    //串口1所使用管脚输出输入定义
    //******************************************************************************
    //定义UART1 TX (PA.09)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;         //IO口的第九脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化串口1输出IO口   
    // 定义 USART1 Rx (PA.10)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           //IO口的第十脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);               //初始化串口1输入IO口
    
    USART_InitStructure.USART_BaudRate = baudrate;                                     //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;                            //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //使用接收和发送功能
    USART_Init(USART1, &USART_InitStructure);                                       //初始化串口1
    
    if(ISR)
    {
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                                  //使能串口1接收中断 发送
        /*使能串口1接收中断，抢占优先级 3 响应优先级 0*/
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
    USART_Cmd(USART1, ENABLE);  
	gUsartGetDataSem[BUF_USART1] = OSSemCreate(0);
}

/////*中断处理函数------------------------------------------*/
void USART1_IRQHandler(void)
{
	OSIntEnter();
	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART1);  /*进行空读操作，目的是清除ORE位*/
	}
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
	{
		USART_FrameBufWrite(BUF_USART1);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	OSIntExit();
}
#endif

#ifdef  USE_USART2
void USART2_Config(int baudrate, char ISR)
{
    /*定义GPIO初始化结构体 GPIO_InitStructure*/
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    /*定义NVIC初始化结构体 NVIC_InitStructure*/
    NVIC_InitTypeDef NVIC_InitStructure;
    /*选择NVIC优先级分组4( 0~15抢占优先级)*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);/**/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,   ENABLE); 
    
//    GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);  /*重映射 PD5 PD6*/
    //******************************************************************************
    //串口2 所使用管脚输出输入定义
    //******************************************************************************
    //定义UART2 TX (PA.2)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;         //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化串口1输出IO口  
    // 定义 USART2 Rx (PA.3)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;           //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);               //初始化串口2输入IO口   
    
    USART_InitStructure.USART_BaudRate = baudrate;                                      //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No;                            //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                             //使用接收
    USART_Init(USART2, &USART_InitStructure);                                       //初始化串口2
     
   if(ISR)
   {
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);                                  //使能串口2接收中断
        /*使能串口2接收中断，抢占优先级   响应优先级 0*/
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
   }
   USART_Cmd(USART2, ENABLE);  
   gUsartGetDataSem[BUF_USART2] = OSSemCreate(0);
}  

void USART2_IRQHandler(void)
{
	OSIntEnter();
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART2);  /*进行空读操作，目的是清除ORE位*/
	}
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
	{
		USART_FrameBufWrite(BUF_USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	OSIntExit();
}
#endif

#ifdef  USE_USART3
void USART3_Config(int baudrate, char ISR)
{
    /*定义GPIO初始化结构体 GPIO_InitStructure*/
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    /*定义NVIC初始化结构体 NVIC_InitStructure*/
    NVIC_InitTypeDef NVIC_InitStructure;
    /*选择NVIC优先级分组4( 0~15抢占优先级)*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,   ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
    //******************************************************************************
    //串口3所使用管脚输出输入定义
    //******************************************************************************
    //定义UART3 TX (PB.10)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;         //IO口的第九脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);            //初始化串口3输出IO口    
    // 定义 UART3 Rx (PB.11)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           //IO口的第十脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);               //初始化串口3输入IO口
    
    USART_InitStructure.USART_BaudRate = baudrate;                                     //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;                            //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //使用接收和发送功能
    USART_Init(USART3, &USART_InitStructure);                                       //初始化串口3

	if(ISR)
	{ 
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                                  //使能串口3接收中断
		/*使能串口3接收中断，抢占优先级  响应优先级 0*/
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	USART_Cmd(USART3, ENABLE);  
	gUsartGetDataSem[BUF_USART3] = OSSemCreate(0);
}

/*中断处理函数------------------------------------------*/
void USART3_IRQHandler(void)
{
	OSIntEnter();
	if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART3);  /*进行空读操作，目的是清除ORE位*/
	}
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)
	{
		USART_FrameBufWrite(BUF_USART3);
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	OSIntExit();
}
#endif

#ifdef  USE_UART4
void UART4_Config(int baudrate, char ISR)
{
    /*定义GPIO初始化结构体 GPIO_InitStructure*/
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    /*定义NVIC初始化结构体 NVIC_InitStructure*/
    NVIC_InitTypeDef NVIC_InitStructure;
////    /*选择NVIC优先级分组4( 0~15抢占优先级)*/
////    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,   ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); 
    //******************************************************************************
    //串口4所使用管脚输出输入定义
    //******************************************************************************
    //定义UART4 TX (PC.10)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;         //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //初始化串口4输出IO口
    
    // 定义 USART4 Rx (PC.11)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);               //初始化串口4输入IO口   
    
    USART_InitStructure.USART_BaudRate = baudrate;                                     //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;                            //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //使用接收和发送功能
    USART_Init(UART4, &USART_InitStructure);                                       //初始化串口4
    
   if(ISR)
   {
        USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);                                  //使能串口4接收中断
        /*使能串口4接收中断，抢占优先级  响应优先级 0*/
        NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
   USART_Cmd(UART4, ENABLE);  
   gUsartGetDataSem[BUF_UART4] = OSSemCreate(0);
}

void UART4_IRQHandler(void)
{
	OSIntEnter();
	if(USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(UART4);  /*进行空读操作，目的是清除ORE位*/
	}
	if(USART_GetFlagStatus(UART4, USART_FLAG_RXNE) != RESET)
	{
		USART_FrameBufWrite(BUF_UART4);
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
	}
	OSIntExit();
}

#endif

#ifdef  USE_UART5
void UART5_Config(int baudrate, char ISR)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
//	USART_ClockInitTypeDef USART_ClockInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);       

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE); 
    
    /* C12 USART5_Tx */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* D2 USART5_Rx  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入-RX
    GPIO_Init(GPIOD, &GPIO_InitStructure);
   
    USART_InitStructure.USART_BaudRate = baudrate; //目前只修改波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =  USART_Mode_Rx | USART_Mode_Tx; 

	USART_Init(UART5, &USART_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
	if (ISR)
	{    
		USART_ITConfig(UART5, USART_IT_RXNE, ENABLE); 
		 /* Enable UART5_IRQn  Channel */
		NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

    /* ENABLE the USARTx */
	USART_Cmd(UART5, ENABLE);
	gUsartGetDataSem[BUF_UART5] = OSSemCreate(0);
}

void UART5_IRQHandler(void)
{
	OSIntEnter();
	if(USART_GetFlagStatus(UART5, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(UART5);  /*进行空读操作，目的是清除ORE位*/
	}
	if(USART_GetFlagStatus(UART5, USART_FLAG_RXNE) != RESET)
	{
		USART_FrameBufWrite(BUF_UART5);/*将接收到的数据写入缓存*/
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);
	}
	OSIntExit();
}
#endif

void USART_Config(USART_TypeDef* USARTx, uint32_t BaudRate)
{
	if(USARTx == USART1)
	{
#ifdef  USE_USART1
		USART1_Config(BaudRate , TRUE);
#endif
	}
	else if(USARTx == USART2)
	{
#ifdef  USE_USART2
		USART2_Config(BaudRate , TRUE);
#endif
	}
	else if(USARTx == USART3)
	{
#ifdef  USE_USART3
		USART3_Config(BaudRate , TRUE);
#endif
	}
	else if(USARTx == UART4)
	{
#ifdef  USE_UART4
		UART4_Config(BaudRate , TRUE);
#endif
	}
	else if(USARTx == UART5)
	{
#ifdef  USE_UART5
		UART5_Config(BaudRate , TRUE);
#endif
	}
}



void USART_FrameBufWrite(enUartFrameBuf bufx)
{  
  if(FrameBufWptr[bufx] == (FrameBufRptr[bufx] - 1))     /*防止套圈*/
   {
     return;
   }
   
   FrameBuf[bufx][FrameBufWptr[bufx]] = USART_ReceiveData(gUsartDef[bufx]);
   FrameBufWptr[bufx]++;
   FrameBufWptr[bufx] %= FRAME_BUF_MAX;
   OSSemPost(gUsartGetDataSem[bufx]);//通知任务数据到达
}

uint8_t USART_FrameBufRead(enUartFrameBuf bufx, uint8_t *data)
{
    if(FrameBufRptr[bufx] == FrameBufWptr[bufx])
    {
      return 0;
    }
    *data = FrameBuf[bufx][FrameBufRptr[bufx]];
    FrameBufRptr[bufx]++;
    FrameBufRptr[bufx] %= FRAME_BUF_MAX;
    return bufx + 1;
}

void UART_Sendbyte(USART_TypeDef* USARTx, uint8_t data)
{
    USART_SendData(USARTx, (uint8_t)data);
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}

void UART_SendBuf(USART_TypeDef* USARTx, uint8_t *buf, uint16_t buflen)
{
    uint16_t i;
    USART_ClearFlag(USARTx, USART_FLAG_TC);
    for(i = 0; i < buflen; i++)
    {
      UART_Sendbyte(USARTx, buf[i]);
    }
}

int fputc(int ch ,FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{}
	
		return (ch);
		
}

