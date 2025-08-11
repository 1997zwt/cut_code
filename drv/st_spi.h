#ifndef _ST_SPI_H_
#define _ST_SPI_H_


#include "stm32f10x.h"



struct drv_spi
{
	const char  *name;
  SPI_TypeDef *Instance;
	uint8_t     spi_init_flg;
};


#ifndef SPI1_CONFIG
#define SPI1_CONFIG                                    \
    {                                                  \
        .name = "spi1",                                \
        .Instance = SPI1,                              \
				.spi_init_flg=0,							   							 \
		}
#endif 



#ifndef SPI2_CONFIG
#define SPI2_CONFIG                                   \
    {                                                 \
        .name = "spi2",                               \
        .Instance = SPI2,                             \
				.spi_init_flg=0,							                \
		}
#endif 


int spi_init(const char *name);
uint8_t SPI_Read_Byte(SPI_TypeDef *spi);
uint8_t SPI_Write_Byte(SPI_TypeDef *spi,uint8_t ByteSend);

#endif
