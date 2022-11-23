#include "drv_dog.h"

void IWDG_Init(uint16_t time_ms)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 
    IWDG_SetPrescaler(4);		//64分频，最短时间为1.6ms
    IWDG_SetReload((time_ms*10)>>4);     
    IWDG_ReloadCounter();  
    IWDG_Enable();     
}

void IWDG_Feed(void)
{
    IWDG_ReloadCounter();    /*reload*/
}
