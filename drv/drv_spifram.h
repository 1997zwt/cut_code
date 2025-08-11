#ifndef _DRV_SPIFRAM_H_
#define _DRV_SPIFRAM_H_

#define USE_SPI1
#define SPI1_NSS_PIN  "PC13"
#define SPI1_SCLK_PIN "PB3"
#define SPI1_MISO_PIN "PB4"
#define SPI1_MOSI_PIN "PB5"



#include "stm32f10x.h"
#include "st_spi.h"

int framflash_init(void);
void Fram_WriteData(uint32_t addr, uint8_t *data ,uint16_t num);
void Fram_ReadData(uint32_t addr, uint8_t *data ,uint16_t num);

struct drv_framflsh
{
	const char  *name;
	const char  *mosi_pin_name;
    const char  *miso_pin_name;
	const char  *sclk_pin_name;
	const char  *nss_pin_name;
    SPI_TypeDef *Instance;
};

#if defined(USE_SPI1)
#ifndef FRAMFLASH_CONFIG
#define FRAMFLASH_CONFIG                               \
    {                                                  \
        .name = "framflash",                           \
        .mosi_pin_name = SPI1_MOSI_PIN,                \
        .miso_pin_name = SPI1_MISO_PIN,                \
				.sclk_pin_name = SPI1_SCLK_PIN,		           \
				.nss_pin_name =  SPI1_NSS_PIN,                 \
        .Instance = SPI1,                              \
		}
#endif 
#endif


#define WREN 6
#define WRDI 4
#define RDSR 5
#define WRSR 1
#define READ 3
#define FSTRD 11
#define WRITE 2
#define SLEEP 185
#define RDID 159

#endif
