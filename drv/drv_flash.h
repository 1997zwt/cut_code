/**
  ******************************************************************************
  * @file    drv_flash.h 
  * @author  lyq
  * @version V1.0.0
  * @date    2016-2-17
  * @brief   本程序根据原子ALIENTEK战舰STM32开发板例程修改
  ******************************************************************************
  * @copy
  */ 
#ifndef USER_FLASH_H
#define USER_FLASH_H

#include "stm32f10x.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//用户根据自己的需要设置
#define STM32_FLASH_SIZE		128 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN	1              //使能FLASH写入(0，不是能;1，使能)

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	 2048
#endif		 
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
	
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
void DRV_FLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		
//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void DRV_FLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		
uint16_t DRV_FLASH_ReadHalfWord(uint32_t faddr);								   
#endif

















