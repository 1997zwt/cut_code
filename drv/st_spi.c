#include "st_spi.h"
#include <string.h>

static struct drv_spi spi_config[] =
{

    SPI1_CONFIG,
    SPI2_CONFIG,
};


static long spi_clk_enable(struct drv_spi *config)
{
    switch ((uint32_t)config->Instance)
    {

    case (uint32_t)SPI1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
        break;


    case (uint32_t)SPI2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
        break;

    default:
        return -1;
    }

    return 0;
}

struct drv_spi *get_drv_spi(const char *name)
{
    uint8_t spilen=0;
    int i=0;
    register struct drv_spi *spi;
    spilen = sizeof(spi_config) / sizeof(spi_config[0]);
    for(i=0;i<spilen;i++)
    {
        if(strcmp(spi_config[i].name,name) == 0)
        {
            spi = &spi_config[i];
            return spi;
        }
    }
    return NULL;
}



int spi_init(const char *name)
{
    register struct drv_spi *spi;
	spi = get_drv_spi(name);
	if(spi==NULL)
	{
		return -1;
	}
	if(spi->spi_init_flg==0)
	{
		spi_clk_enable(spi);
		SPI_InitTypeDef SPI_InitStructure={0};						
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;	
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;				
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;			
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;				
		SPI_InitStructure.SPI_CRCPolynomial = 7;				
		SPI_Init(spi->Instance, &SPI_InitStructure);
		SPI_Cmd(spi->Instance, ENABLE);
		spi->spi_init_flg=1;
	}
		
		return 1;
}

uint8_t SPI_Write_Byte(SPI_TypeDef *spi,uint8_t ByteSend)
{
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) != SET);	//等待传输寄存器为空
	SPI_I2S_SendData(spi, ByteSend);								//发送数据
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) != SET);	//等待接收寄存器为空
	return SPI_I2S_ReceiveData(spi);								//接收数据
}

uint8_t SPI_Read_Byte(SPI_TypeDef *spi)
{
  return (SPI_Write_Byte(spi,0xFF));
}





