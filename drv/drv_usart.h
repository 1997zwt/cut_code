#ifndef __USART_H__
#define __USART_H__

#include "drivers_cfg.h"

/**********************************************/
/*定义串口接收缓冲区长度*/
#define  FRAME_BUF_MAX			100

typedef enum
{
	BUF_USART1,
	BUF_USART2,
	BUF_USART3,
	BUF_UART4,
	BUF_UART5
}enUartFrameBuf;

extern OS_EVENT* gUsartGetDataSem[5];
extern uint8_t err;
#define WaitUsartDataUpdate(bufx)	OSSemPend((OS_EVENT *)gUsartGetDataSem[bufx], 0,&err);

/*
初始化串口为RS232串口
--USARTx: 串口号(USART1,USART2,USART3,UART4,UART5)
--BaudRate: 波特率
--enable_isr:是否使用接收中断
*/
void USART_Config(USART_TypeDef* USARTx, uint32_t BaudRate); 
/*
* 函数名：USART_FrameBufRead
* 描述  ：缓冲区读函数
* 输入  ：data,待存放读出数据的内存空间指针
* 输入  ：bufx, 数据缓冲区编号
* 输出  ：0：无数据  1：有数据
*/
uint8_t USART_FrameBufRead(enUartFrameBuf bufx, uint8_t *data);/*
* 函数名：UART_Sendbyte
* 描述  ：发送字节
* 输入  ：字节数据
* 输出  ：无
*/
void UART_Sendbyte(USART_TypeDef* USARTx, uint8_t data);
/*
* 函数名：UART_SendBuf
* 描述  ：发送数组
* 输入  ：指向数组首地址指针，数组长度
* 输出  ：无
*/
void UART_SendBuf(USART_TypeDef* USARTx, uint8_t *buf, uint16_t buflen);

#endif
/******************* (C) COPYRIGHT 2020 REAPDARO *****END OF FILE****/


