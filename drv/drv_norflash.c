#include "drv_norflash.h"
#include <stdlib.h>
#include <string.h>
#include "ucos_ii.h"

#define __STM32_PORT(port)  GPIO##port##_BASE
#define GET_PIN(PORTx,PIN) (long)((16 * ( ((long)__STM32_PORT(PORTx) - (long)GPIOA_BASE)/(0x0400UL) )) + PIN)

static struct drv_norflash config[] =
{
    NORFLASH_CONFIG,

};

static long gpio_clk_enable(GPIO_TypeDef *gpiox)
{
    /* gpio ports clock enable */
    switch ((uint32_t)gpiox)
    {
    case (uint32_t)GPIOA:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); 
        break;
    case (uint32_t)GPIOB:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
        break;
    case (uint32_t)GPIOC:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
        break;
    case (uint32_t)GPIOD:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
        break;
    case (uint32_t)GPIOE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
        break;
    default:
        return -1;
    }

    return 0;
}


static struct drv_norflash *get_drv_spi(const char *name)
{
    uint8_t spilen=0;
    int i=0;
    register struct drv_norflash *spi;
    spilen = sizeof(config) / sizeof(config[0]);
    for(i=0;i<spilen;i++)
    {
        if(strcmp(config[i].name,name) == 0)
        {
            spi = &config[i];
            return spi;
        }
    }
    return NULL;
}

static int up_char(char * c)
{
    if ((*c >= 'a') && (*c <= 'z'))
    {
        *c = *c - 32;
    }
    return 0;
}

static int my_ffs(unsigned int x) {  
    if (x == 0) {  
        return 0; 
    }  
    int position = 1; 
    while ((x & 1) == 0) {
        x >>= 1; 
        position++;
    }  
    return position; 
}

static uint16_t get_pin_num(GPIO_TypeDef *pin_port, uint32_t pin_num)
{
    return (uint16_t)((16 * (((long)pin_port - (long)GPIOA_BASE)/(0x0400UL))) + (my_ffs(pin_num) - 1));
}

static void get_pin_by_name(const char* pin_name, GPIO_TypeDef **port, uint16_t *pin)
{
    int len =strlen(pin_name);
    if(pin_name==NULL||len<3)
    {
        return;
    }
    int pin_num = atoi((char*) &pin_name[2]);
    char port_name = pin_name[1];
    up_char(&port_name); 
    up_char(&port_name);
    *port = ((GPIO_TypeDef *) ((uint32_t) GPIOA
            + (uint32_t) (port_name - 'A') * ((uint32_t) GPIOB - (uint32_t) GPIOA)));
    *pin = (GPIO_Pin_0 << pin_num);
}

static void spi_gpio_configure(struct drv_norflash *config)
{
    uint16_t mosi_pinnum=0;
    int mosi_index = 0, index = 0;
    int spi_is_remap = 0,spi_num=0;
	GPIO_TypeDef *sck_port=0;
    GPIO_TypeDef *miso_port=0;
	GPIO_TypeDef *mosi_port=0;
    GPIO_TypeDef *cs_port=0;
    uint16_t sck_pin;
    uint16_t miso_pin;
	uint16_t mosi_pin;
    uint16_t cs_pin;
    get_pin_by_name(config->mosi_pin_name, &mosi_port, &mosi_pin);
    get_pin_by_name(config->miso_pin_name, &miso_port, &miso_pin);
    get_pin_by_name(config->nss_pin_name, &cs_port, &cs_pin);
    get_pin_by_name(config->sclk_pin_name, &sck_port, &sck_pin);
    struct gpio_spi_remap {
        /* index get by GET_PIN */
        uint16_t pin_index;
        /* -1: not spi, 1: spi1, 2: spi2 ... */
        int8_t normal_spi;
        /* -1: not spi, 1: spi1, 2: spi2 ... */
        int8_t remap_spi;
    };
    static const struct gpio_spi_remap spi_remaps[] =
    {
        /* spi1 configure */
        { .pin_index = GET_PIN(A, 5),  .normal_spi =  1, .remap_spi = -1 },
        { .pin_index = GET_PIN(A, 6),  .normal_spi =  1, .remap_spi = -1 },
        { .pin_index = GET_PIN(A, 7),  .normal_spi =  1, .remap_spi = -1 },
        { .pin_index = GET_PIN(B, 3),  .normal_spi = -1, .remap_spi =  1 },
        { .pin_index = GET_PIN(B, 4),  .normal_spi = -1, .remap_spi =  1 },
        { .pin_index = GET_PIN(B, 5),  .normal_spi = -1, .remap_spi =  1 },

        /* spi2 configure */
        { .pin_index = GET_PIN(B, 13),  .normal_spi =  2, .remap_spi = -1 },
        { .pin_index = GET_PIN(B, 14),  .normal_spi =  2, .remap_spi = -1 },
        { .pin_index = GET_PIN(B, 15),  .normal_spi =  2, .remap_spi = -1 },
    };
    mosi_pinnum = get_pin_num(mosi_port, mosi_pin);
    for (index = 0; index < sizeof(spi_remaps) / sizeof(struct gpio_spi_remap); index++)
	{
       if (spi_remaps[index].pin_index == mosi_pinnum)
       {
           mosi_index = index;
       }
	}
    if(config->Instance==SPI1)
		{
			spi_num = 1;
		}else if(config->Instance==SPI2)
		{
			spi_num = 2;
		}
    spi_is_remap = spi_remaps[mosi_index].remap_spi == spi_num ? 1 : 0;//确定是否重映射
    {
        //
        GPIO_InitTypeDef GPIO_InitStruct={0};
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        if(sck_port!=0)
        {
            gpio_clk_enable(sck_port);
            GPIO_InitStruct.GPIO_Pin = sck_pin;
            GPIO_Init(sck_port, &GPIO_InitStruct);
        }
        if(miso_port!=0)
        {
            gpio_clk_enable(miso_port);
            GPIO_InitStruct.GPIO_Pin = miso_pin;
            GPIO_Init(miso_port, &GPIO_InitStruct);
        }
        if(mosi_port!=0)
        {
            gpio_clk_enable(mosi_port);
            GPIO_InitStruct.GPIO_Pin = mosi_pin;
            GPIO_Init(mosi_port, &GPIO_InitStruct);
        }
        if(cs_port!=0)
        {
            gpio_clk_enable(cs_port);
            GPIO_InitStruct.GPIO_Pin = cs_pin;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_Init(cs_port, &GPIO_InitStruct);
        }
        if (spi_is_remap)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
            GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE); 
            switch (spi_num)
            {
            case 1:
                GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
                break;
            case 2:
                break;
            default:
                break;
            }
       }
    }
    
}



///////////////////////////////////////////////////////////////////////////////////////
struct drv_norflash *spiflash;
GPIO_TypeDef *cs_port=0;
uint16_t cs_pin;
#define CS_LOW()  GPIO_ResetBits(cs_port,cs_pin);
#define CS_HIGH() GPIO_SetBits(cs_port,cs_pin);
void Wirte_wel(void);

int norflash_init(void)
{
		spiflash = get_drv_spi("norflash");
		if(spiflash==NULL)
		{
			return -1;
		}
		spi_gpio_configure(spiflash);
    #if defined(USE_SPI1)    
    spi_init("spi1");
    #elif defined(USE_SPI2)
    spi_init("spi2");
    #else
        #error no define spi
    #endif
    get_pin_by_name(spiflash->nss_pin_name, &cs_port, &cs_pin);
    CS_HIGH();
    Wirte_wel();
    return 0;
}

void Wirte_wel(void)
{
    unsigned char ret;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,FLASH_WEL);
    CS_HIGH();
}

uint8_t flashifbusy(void)
{
    unsigned char ret;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,NORFLASH_BUSY);
    ret = SPI_Read_Byte(spiflash->Instance);
    CS_HIGH();
    ret = ret & 0x01;
    return ret;
}

//0~32767
void Memary_WritePageData(uint32_t page, uint8_t *data)
{
    uint32_t addr=0,i=0;
    unsigned char ret;
    if(page>32767||spiflash==NULL)
    {
        return;
    }
		OS_CPU_SR  cpu_sr;
		OS_ENTER_CRITICAL();
    while (flashifbusy())
    {
    }
		
    Wirte_wel();
    addr = page*256;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,FLASH_WRITE);
    ret = SPI_Write_Byte(spiflash->Instance,addr>>16);
    ret = SPI_Write_Byte(spiflash->Instance,addr>>8);
    ret = SPI_Write_Byte(spiflash->Instance,addr);
    for(i=0;i<256;i++)
    {
        ret = SPI_Write_Byte(spiflash->Instance,*(data+i));
    }
    CS_HIGH();
		OS_EXIT_CRITICAL();
}

void Memary_ReadPageData(uint32_t page,uint8_t *data)
{
    unsigned char ret;
    uint32_t addr=0,i=0;//0~16777215 Byte
    if(page>32767||spiflash==NULL)
    {
        return;
    }
    addr = page*256;
    while(flashifbusy())
    {

    }
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,FLASH_READ);
    ret = SPI_Write_Byte(spiflash->Instance,addr>>16);
    ret = SPI_Write_Byte(spiflash->Instance,addr>>8);
    ret = SPI_Write_Byte(spiflash->Instance,addr);
    for(i=0;i<256;i++)
    {
        *(data+i) = SPI_Read_Byte(spiflash->Instance);
    }
    CS_HIGH();
}

uint16_t Read_ManuID(void)
{
    unsigned char ret;
    uint16_t temp=0;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,FLASH_ID);
    ret = SPI_Write_Byte(spiflash->Instance,0);
    ret = SPI_Write_Byte(spiflash->Instance,0);
    ret = SPI_Write_Byte(spiflash->Instance,0);
    ret = SPI_Read_Byte(spiflash->Instance);
    temp = (uint16_t)ret<<8;
    ret = SPI_Read_Byte(spiflash->Instance);
    temp |= ret;
    CS_HIGH();
    return temp;
}
//0~2047
void Memary_EraseData(uint32_t sector) 
{
    uint32_t addr=0,i=0;
    unsigned char ret;
    if(sector>2047||spiflash==NULL)
    {
        return;
    }
				OS_CPU_SR  cpu_sr;
		OS_ENTER_CRITICAL();
	while (flashifbusy())
    {
    }
    Wirte_wel();
    addr = sector*4096;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,SECTOR_ERASE);
    ret = SPI_Write_Byte(spiflash->Instance,(addr >> 16) & 0xFF);
    ret = SPI_Write_Byte(spiflash->Instance,(addr >> 8) & 0xFF);
    ret = SPI_Write_Byte(spiflash->Instance,addr & 0xFF);
    CS_HIGH();
		OS_EXIT_CRITICAL();
}

void Memary_EraseWait(uint32_t sector)
{
    uint32_t addr=0,i=0;
    unsigned char ret;
    if(sector>2047||spiflash==NULL)
    {
        return;
    }
				OS_CPU_SR  cpu_sr;
		OS_ENTER_CRITICAL();
	while (flashifbusy())
    {
    }
    Wirte_wel();
    addr = sector*4096;
    CS_LOW();
    ret = SPI_Write_Byte(spiflash->Instance,SECTOR_ERASE);
    ret = SPI_Write_Byte(spiflash->Instance,(addr >> 16) & 0xFF);
    ret = SPI_Write_Byte(spiflash->Instance,(addr >> 8) & 0xFF);
    ret = SPI_Write_Byte(spiflash->Instance,addr & 0xFF);
    CS_HIGH();
    while(flashifbusy())
    {

    }
		OS_EXIT_CRITICAL();
}


