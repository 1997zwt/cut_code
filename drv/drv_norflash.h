#ifndef _DRV_NORFLASH_H_
#define _DRV_NORFLASH_H_

#define USE_SPI1
#define SPI1_NSS_PIN  "PA15"
#define SPI1_SCLK_PIN "PB3"
#define SPI1_MISO_PIN "PB4"
#define SPI1_MOSI_PIN "PB5"

#include "stm32f10x.h"
#include "st_spi.h"

void Memary_ReadPageData(uint32_t page,uint8_t *data);
void Memary_WritePageData(uint32_t page, uint8_t *data);
void Memary_EraseData(uint32_t sector);
void Memary_EraseWait(uint32_t sector);
int norflash_init(void);

struct drv_norflash
{
	const char  *name;
	const char  *mosi_pin_name;
  const char  *miso_pin_name;
	const char  *sclk_pin_name;
	const char  *nss_pin_name;
	SPI_TypeDef *Instance;
};

#if defined(USE_SPI1)
#ifndef NORFLASH_CONFIG
#define NORFLASH_CONFIG                                \
    {                                                  \
        .name = "norflash",                            \
        .mosi_pin_name = SPI1_MOSI_PIN,                \
        .miso_pin_name = SPI1_MISO_PIN,                \
				.sclk_pin_name = SPI1_SCLK_PIN,		             \
				.nss_pin_name =  SPI1_NSS_PIN,                 \
				.Instance = SPI1,                              \
		}
#endif 
#endif



#define FLASH_WEL     0x06
#define FLASH_READ    0x03
#define FLASH_WRITE   0x02
#define FLASH_ID      0x90
#define NORFLASH_BUSY 0x05
#define SECTOR_ERASE  0x20

#endif
