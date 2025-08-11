#include "log.h"

stClodkDef gColck = {0, 1, 1, 12, 0, 0};
stFaultLogDef gFault; //故障信息8byte--
stNomalInfoLogDef gNomalInfo;
/*
可以存储最近一个月的日志，每天建立一个索引页，如果当天日志为空，则返回空
最后一次关机的索引信息固定存储在第32页，开机后读取32页，如果日期与今天一致，则继续写日志
如果日期与今天不一致，则将该信息存储到对应的页码，并建立一个新的索引。
*/
stNewLogDef gLogTable; //存储当天的日志信息 32byte
stLogDataDef gLogDataPage[16]; //存储最新的日志数据，一页存16条日志  256byte

uint8_t gPowerInfo[4] = {0};  //暂存开机信息，格式：标志1、时间1、电压2
stQueryDef gQueryInfo; //查询信息 16byte
uint8_t gPageDataBuf[300] = {0};//读取缓冲区 300byte
uint32_t gSysRunSecond = 0;    //开机后系统运行时间

uint8_t gWriteEnable = 1;
extern OS_EVENT *USART_MUTEX;
extern uint8_t m_err;
void RTC_ReadTime(void);
void _ClearThisPage(void)
{
    uint8_t* buf;
    uint16_t i = 0;

    buf = (uint8_t*)&gLogDataPage[0].type;
    for (i = 0; i < 256; i++)
    {
        buf[i] = 0;
    }    
}
void SaveLog(uint8_t type)
{
    GPIO_SetBits(GPIOB, STATELED_GPIO_PIN);
    if (gPowerInfo[0] != 0) //如果没有校时，把开机信息保存下来
    {
        Log_RecordPowerInfo(2, gPowerInfo[2] + (gPowerInfo[3] << 8), 0);
    }

    RTC_ReadTime();
    gLogDataPage[gLogTable.log_index].type = LOG_TYPE_POWER;
    gLogDataPage[gLogTable.log_index].tm.year = gColck.year;
    gLogDataPage[gLogTable.log_index].tm.month = gColck.month;
    gLogDataPage[gLogTable.log_index].tm.day = gColck.day;
    gLogDataPage[gLogTable.log_index].tm.hour = gColck.hour;
    gLogDataPage[gLogTable.log_index].tm.minute = gColck.minute;
    gLogDataPage[gLogTable.log_index].tm.second = gColck.second;
    gLogDataPage[gLogTable.log_index].log_info[0] = type;
    gLogDataPage[gLogTable.log_index].log_info[1] = 0;
    gLogDataPage[gLogTable.log_index].log_info[2] = 0;
    gLogDataPage[gLogTable.log_index].log_info[3] = 0;
    gLogDataPage[gLogTable.log_index].log_info[4] = Lisup.RealVol;
    gLogDataPage[gLogTable.log_index].log_info[5] = Lisup.RealVol >> 8;
    gLogDataPage[gLogTable.log_index].log_info[6] = 0;
    gLogDataPage[gLogTable.log_index].log_info[7] = 0;
    Fram_WriteData(PAGE_THIS_LOG, (uint8_t *)&gLogDataPage[0].type,256); //保存当前日志到临时地址
    gLogTable.log_index++;
    gLogTable.counts++;
    gLogTable.pwr_on_cnts++;
    Fram_WriteData(PAGE_NEW_LOG_INFO, (uint8_t *)&gLogTable.this_page.date,32); //保存索引表
    GPIO_ResetBits(GPIOB, STATELED_GPIO_PIN); //测试
}
/**
  * @brief  This function handles the PVD Output interrupt request.
  * @param  None
  * @retval None
  */
void PVD_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line16) != RESET)
    {
        GPIO_SetBits(GPIOB, STATELED_GPIO_PIN);
        if(PWR_GetFlagStatus(PWR_FLAG_PVDO))
				{
					if(gLogTable.run_tm_this>=3)
					{
						SaveLog(3);
					}
				}
        /* Clear the Key Button EXTI line pending bit */
        EXTI_ClearITPendingBit(EXTI_Line16);
    }
}



void RTC_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // 开启PWR的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE); // 开启BKP的时钟

    PWR_BackupAccessCmd(ENABLE); // 使用PWR开启对备份寄存器的访问
    BKP_DeInit();

    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) //通过写入备份寄存器的标志位，判断RTC是否是第一次配置
    {
#ifdef RTC_CLOCK_SOURCE_LSE //使用外部晶振
        /* 使能 LSE */
        RCC_LSEConfig(RCC_LSE_ON);
        /* 等待 LSE 准备好 */
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
        }

        /* 选择 LSE 作为 RTC 时钟源 */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        /* 使能 RTC 时钟 */
        RCC_RTCCLKCmd(ENABLE);
        /* 等待 RTC 寄存器 同步
    * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
    */
        RTC_WaitForSynchro();
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 使能 RTC 秒中断 */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 设置 RTC 分频: 使 RTC 周期为1s  */
        /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
        RTC_SetPrescaler(32767);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();

#endif
#ifdef RTC_CLOCK_SOURCE_LSI //使用内部LSI时钟                            
        /* 使能 LSI */
        RCC_LSICmd(ENABLE);
        /* 等待 LSI 准备好 */
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
        }
        /* 选择 LSI 作为 RTC 时钟源 */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        /* 使能 RTC 时钟 */
        RCC_RTCCLKCmd(ENABLE);
        /* 等待 RTC 寄存器 同步
    * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
    */
        RTC_WaitForSynchro();
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 使能 RTC 秒中断 */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 设置 RTC 分频: 使 RTC 周期为1s ,LSI约为40KHz */
        /* RTC period = RTCCLK/RTC_PR = (40 KHz)/(40000-1+1) = 1HZ */
        RTC_SetPrescaler(40000 - 1);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
#endif
#ifdef RTC_CLOCK_SOURCE_HSE //使用内部HSE时钟
        /* 使能 LSI */
        RCC_LSICmd(ENABLE);
        /* 等待 LSI 准备好 */
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
        }
        /* 选择 LSI 作为 RTC 时钟源 */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
        /* 使能 RTC 时钟 */
        RCC_RTCCLKCmd(ENABLE);
        /* 等待 RTC 寄存器 同步
    * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
    */
        RTC_WaitForSynchro();
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 使能 RTC 秒中断 */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
        /* 设置 RTC 分频: 使 RTC 周期为1s ,HSE时钟128分配后约为62.5KHz */
        RTC_SetPrescaler(62500 - 1);
        /* 确保上一次 RTC 的操作完成 */
        RTC_WaitForLastTask();
#endif
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); //如果使用备用电源，程序重启后不会再次配置RTC
    }
    else //RTC不是第一次配置
    {
        RTC_WaitForSynchro();  //等待同步
        RTC_WaitForLastTask(); //等待上一次操作完成
    }
}

void RTC_ReadTime(void)
{
    time_t time_cnt;
    struct tm time_date;

    time_cnt = RTC_GetCounter() + 8 * 60 * 60; // 读取当前的秒计数器并调整时区

    time_date = *localtime(&time_cnt); // 转换为日期时间格式

    gColck.year = time_date.tm_year - 100;
    gColck.month = time_date.tm_mon + 1;
    gColck.day = time_date.tm_mday;
    gColck.hour = time_date.tm_hour;
    gColck.minute = time_date.tm_min;
    gColck.second = time_date.tm_sec;
}
//校时
void RTC_SetTime(uint8_t *clock)
{
    time_t time_cnt;
    struct tm time_date;
    uint32_t date = 0;
    uint16_t new_block;

    RTC_ITConfig(RTC_IT_SEC, DISABLE);
    gColck.year = clock[0];
    gColck.month = clock[1];
    time_date.tm_year = clock[0] + 100;
    time_date.tm_mon = clock[1] - 1; // 月份减去 1
    gColck.day = time_date.tm_mday = clock[2];
    gColck.hour = time_date.tm_hour = clock[3];
    gColck.minute = time_date.tm_min = clock[4];
    gColck.second = time_date.tm_sec = clock[5];

    time_cnt = mktime(&time_date) - 8 * 60 * 60; // 转换为秒计数器格式并调整时区

    RTC_SetCounter(time_cnt); // 写入RTC的CNT中
    RTC_WaitForLastTask();    // 等待上一次操作完成
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    //自动生成一个工具ID
    if (ReadUserSettings(VER_DEVICE_ID) == 0)
    {
        date = TOOL_TYPE * 10000000 + gColck.year * 100000 + gColck.month * 1000 +
               ((gColck.day * gColck.day + gColck.hour * gColck.hour + gColck.minute * gColck.minute + gColck.second * gColck.second) % 1000);
        WriteUserSettings(VER_DEVICE_ID, date);
        date = 0;
    }

    date = (gColck.year << 16) + (gColck.month << 8) + gColck.day;
    /*如果日期为当天，则在当前的表上继续记录即可；
    如果日期为新的一天，则将前一天的日志起始页存储在第0页，并新建一个日志页
    新建的日志页必须是一个扇区的第一页*/
    if (date != gLogTable.this_page.date) //新日期
    {
        /*如果是第一次开机，没有前一天的日志信息，无需存储起始页信息*/
        if (gLogTable.pwr_on_cnts != 0) //不是第一次开机
        {
            Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);                        //存储当前日志页
            Fram_WriteData(PAGE_LOG_TABLE + (gLogDataPage[0].tm.day - 1) * 32, (uint8_t *)&gLogTable.this_page.date, 32); //保存索引表到当天日期对应的地址

            //新建索引表
            gLogTable.this_page.first_page = gLogTable.this_page.last_page + 1; //新建一页
            if (gLogTable.this_page.first_page % PAGE_PER_BLOCK != 0)           //如果不是扇区第一页
            {
                //确保起始页是扇区的第一页
                new_block = (gLogTable.this_page.first_page / PAGE_PER_BLOCK) + 1;
                if (new_block >= MAX_BLOCK_INDEX)
                {
                    new_block = BLOCK_BEGIN_PAGE_ID;
                }
                Memary_EraseWait(new_block); //先擦除当前扇区
                gLogTable.this_page.first_page = new_block * PAGE_PER_BLOCK;
            }

            gLogTable.this_page.last_page = gLogTable.this_page.first_page;
        }
        gLogTable.this_page.date = date;
        gLogTable.counts = 0;
        gLogTable.run_tm_day = gLogTable.run_tm_this;
        gLogTable.log_index = 0;
    }
    if (gPowerInfo[0] != 0) //确保只记录一次
    {
        Log_RecordPowerInfo(gPowerInfo[0], gPowerInfo[2] + (gPowerInfo[3] << 8), 1);
        gPowerInfo[0] = 0;
    }
    if (gFault.fault == ERR_NOT_SET_TIME)
    {
        gFault.fault = 0;
    }
}
//RTC秒中断
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        /* Clear the RTC Second interrupt */
        RTC_ClearITPendingBit(RTC_IT_SEC);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        gLogTable.run_tm_this++;
        gLogTable.run_tm_day++;
        gLogTable.run_tm_total++;
        // gColck.second++;
        // if (gColck.second >= 59)
        // {
        //     gColck.second = 0;
        //     RTC_ReadTime(); //每分钟刷新一次时钟
        // }
    }
}
/*开机后首先读取当天的日志索引表，如果最新的一页日志是满的就新开一页，否则读取当天的日志继续记录
如果新开的一页恰好是新扇区的第一页，则先擦除新扇区，为后续保存做准备
*/
void Log_init(void)
{
    uint16_t vol;
    uint16_t new_block;

    //初始化时钟
    RTC_Init();
    //读取最新的日志索引
    Fram_ReadData(PAGE_NEW_LOG_INFO, (uint8_t *)&gLogTable.this_page.date, 32); //保存索引表
    //从暂存区读取最新的日志
    Fram_ReadData(PAGE_THIS_LOG, (uint8_t *)&gLogDataPage[0].type, 256);

    if (gLogTable.pwr_on_cnts == 0) //第一次开机
    {
        _ClearThisPage();
        Memary_EraseWait(BLOCK_BEGIN_PAGE_ID); //先擦除第10扇区，防止存储失败
        gLogTable.this_page.date = 0;
        gLogTable.this_page.first_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK; //从第10个扇区开始计日志
        gLogTable.this_page.last_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK;  //从第10个扇区开始计日志
        gLogTable.counts = 0;
        gLogTable.log_index = 0;
        gLogTable.pwr_on_cnts = 0;
        gLogTable.run_tm_day = 0;
        gLogTable.pwr_on_cnts = 0;
        gLogTable.run_tm_total = 0;
    }
    else
    {
        if (gLogTable.log_index >= 15) //如果最后一页是满的
        {
            //先把这一页日志存起来，再新建一页
            Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);
            gLogTable.log_index = 0; //重开一页
            gLogTable.this_page.last_page++;
           // _ClearThisPage();
            if (gLogTable.this_page.last_page % PAGE_PER_BLOCK == 0) //如果恰好1个扇区结束
            {
                new_block = gLogTable.this_page.last_page / PAGE_PER_BLOCK;
                if (new_block >= MAX_BLOCK_INDEX)
                {
                    new_block = BLOCK_BEGIN_PAGE_ID;
                }
                Memary_EraseWait(new_block); //先擦除当前扇区
            }
        }
    }

    gLogTable.run_tm_this = 0;
    //获取程序复位原因
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) //上电复位
    {
        gPowerInfo[0] = 2;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) //RST管脚复位
    {
        gPowerInfo[0] = 5;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET) //软件复位
    {
        gPowerInfo[0] = 6;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) //独立看门狗复位
    {
        gPowerInfo[0] = 7;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET) //窗口看门狗复位
    {
        gPowerInfo[0] = 8;
    }
    RCC_ClearFlag();
    gPowerInfo[1] = gLogTable.run_tm_this;
    //记录开机信息，由于此时还没有电池电压，只能通过ADC读取
    vol = ADC1_GetValue_mv(BATTARY_VOLTAGE_CHANNEL) * MV_TO_BATTARY;
    gPowerInfo[2] = vol;
    gPowerInfo[3] = vol >> 8;
}

void Log_RecordCmdInfo(uint8_t* cmd)//obj, uint8_t cmd, uint8_t add, uint32_t param)
{
    uint8_t i = 0;
    unLogInfoDef new_info;
    uint16_t new_block;

    if (gColck.year == 0) //未校时不存储
    {
        gFault.module = 0;
        gFault.fault = ERR_NOT_SET_TIME;//发送控制命令时没有校时，产生故障码，不会存储日志
        return;
    }
		if (!gWriteEnable)
		return;

    RTC_ReadTime();
    gLogDataPage[gLogTable.log_index].type = LOG_TYPE_COMMAND;
    gLogDataPage[gLogTable.log_index].tm.year = gColck.year;
    gLogDataPage[gLogTable.log_index].tm.month = gColck.month;
    gLogDataPage[gLogTable.log_index].tm.day = gColck.day;
    gLogDataPage[gLogTable.log_index].tm.hour = gColck.hour;
    gLogDataPage[gLogTable.log_index].tm.minute = gColck.minute;
    gLogDataPage[gLogTable.log_index].tm.second = gColck.second;

    new_info.cmd_info.tool = TOOL_TYPE;
    // new_info.cmd_info.obj = obj;
    // new_info.cmd_info.cmd = cmd;
    // new_info.cmd_info.cmd1 = add;
    // new_info.cmd_info.param = param;
    if (cmd[0] == 0xff) //参数
    {
        new_info.cmd_info.obj = 0xfe;
        new_info.cmd_info.cmd = cmd[1];
        new_info.cmd_info.cmd1 = cmd[2];
        //参数存储低字节在前，参数配置命令参数发送时是高字节在前
        new_info.cmd_info.param = (cmd[3] << 24) + (cmd[4] << 16) + (cmd[5] << 8) + cmd[6];
    }
    else if (cmd[0] == 0xc8) //读取系统信息
    {
        new_info.cmd_info.obj = 0xc8;
        new_info.cmd_info.param = 0;
    }
    else if (cmd[0] == TOOL_TYPE) //命令

    {
        new_info.cmd_info.obj = cmd[1];
        new_info.cmd_info.cmd = cmd[2];
        new_info.cmd_info.cmd1 = 0;
        if (cmd[1] == 0xff) //日志
        {
           // new_info.cmd_info.param = (cmd[6]<<16) + (cmd[7]<<8)+ cmd[8];
        }
        else
        {
            //参数存储低字节在前，控制命令参数发送时是低字节在前
           //new_info.cmd_info.param = cmd[3] + (cmd[4] << 8) + (cmd[5] << 16) + (cmd[6] << 24);
					 new_info.cmd_info.param = cmd[3];
        }
    }

    for (i = 0; i < 8; i++)
    {
        gLogDataPage[gLogTable.log_index].log_info[i] = new_info.buf[i];
    }
    if (gLogTable.log_index == 15) //最后一条
    {
        gWriteEnable = 0;
        //保存日志
        Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);
        gLogTable.this_page.last_page++; //建立新页
                                         // _ClearThisPage();
        gLogTable.log_index = 0;
        if (gLogTable.this_page.last_page == MAX_PAGE_INDEX)
        {
            gLogTable.this_page.last_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK;
        }
        if (gLogTable.this_page.last_page % PAGE_PER_BLOCK == 0) //1个扇区结束
        {
            new_block = gLogTable.this_page.last_page / PAGE_PER_BLOCK;
            if (new_block >= MAX_BLOCK_INDEX)
            {
                new_block = BLOCK_BEGIN_PAGE_ID;
            }
            Memary_EraseData(new_block); //先擦除当前扇区
        }
        gWriteEnable = 1;
    }
    else
    {
        gLogTable.log_index++;
    }
    gLogTable.counts++;
}










//记录信息
void Log_RecordNomalInfo(stNomalInfoLogDef *info)
{
    uint8_t i = 0;
    unLogInfoDef new_info;
    uint16_t new_block;

    if (gColck.year == 0) //未校时不存储
    {
        return;
    }
    if (!gWriteEnable)
        return;

    RTC_ReadTime();
    gLogDataPage[gLogTable.log_index].type = LOG_TYPE_INFO;
    gLogDataPage[gLogTable.log_index].tm.year = gColck.year;
    gLogDataPage[gLogTable.log_index].tm.month = gColck.month;
    gLogDataPage[gLogTable.log_index].tm.day = gColck.day;
    gLogDataPage[gLogTable.log_index].tm.hour = gColck.hour;
    gLogDataPage[gLogTable.log_index].tm.minute = gColck.minute;
    gLogDataPage[gLogTable.log_index].tm.second = gColck.second;

    new_info.nomal_info.info_code = info->info_code;
    new_info.nomal_info.tool_state = info->tool_state;
    new_info.nomal_info.param_0 = info->param_0;
    new_info.nomal_info.param_1 = info->param_1;
    new_info.nomal_info.param_2 = info->param_2;

    for (i = 0; i < 8; i++)
    {
        gLogDataPage[gLogTable.log_index].log_info[i] = new_info.buf[i];
    }
    if (gLogTable.log_index == 15) //最后一条
    {
        gWriteEnable = 0;
        //保存日志
        Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);
        gLogTable.this_page.last_page++; //建立新页
        //_ClearThisPage();
        gLogTable.log_index = 0;
        if (gLogTable.this_page.last_page == MAX_PAGE_INDEX)
        {
            gLogTable.this_page.last_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK;
        }
        if (gLogTable.this_page.last_page % PAGE_PER_BLOCK == 0) //1个扇区结束
        {
            new_block = gLogTable.this_page.last_page / PAGE_PER_BLOCK;
            if (new_block >= MAX_BLOCK_INDEX)
            {
                new_block = BLOCK_BEGIN_PAGE_ID;
            }
            Memary_EraseData(new_block); //先擦除当前扇区
        }
        gWriteEnable = 1;
    }
    else
    {
        gLogTable.log_index++;
    }
    gLogTable.counts++;
}
void Log_RecordFaultInfo(stFaultLogDef *fault)
{
    uint8_t i = 0;
    unLogInfoDef new_info;
    uint16_t new_block;

    if (gColck.year == 0) //未校时不存储
    {
        return;
    }
    if (!gWriteEnable)
        return;

    RTC_ReadTime();
    gLogDataPage[gLogTable.log_index].type = LOG_TYPE_FAULT;
    gLogDataPage[gLogTable.log_index].tm.year = gColck.year;
    gLogDataPage[gLogTable.log_index].tm.month = gColck.month;
    gLogDataPage[gLogTable.log_index].tm.day = gColck.day;
    gLogDataPage[gLogTable.log_index].tm.hour = gColck.hour;
    gLogDataPage[gLogTable.log_index].tm.minute = gColck.minute;
    gLogDataPage[gLogTable.log_index].tm.second = gColck.second;

    new_info.fault_info.module = fault->module;
    new_info.fault_info.fault = fault->fault;
    new_info.fault_info.tool_state = fault->tool_state;
    new_info.fault_info.info_0 = fault->info_0;
    new_info.fault_info.info_1 = fault->info_1;
    new_info.fault_info.bat_vol = fault->bat_vol;

    for (i = 0; i < 8; i++)
    {
        gLogDataPage[gLogTable.log_index].log_info[i] = new_info.buf[i];
    }
    if (gLogTable.log_index == 15) //最后一条
    {
        gWriteEnable = 0;
        //保存日志
        Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);
        gLogTable.this_page.last_page++; //建立新页
        //_ClearThisPage();
        gLogTable.log_index = 0;
        if (gLogTable.this_page.last_page == MAX_PAGE_INDEX)
        {
            gLogTable.this_page.last_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK;
        }
        if (gLogTable.this_page.last_page % PAGE_PER_BLOCK == 0) //1个扇区结束
        {
            new_block = gLogTable.this_page.last_page / PAGE_PER_BLOCK;
            if (new_block >= MAX_BLOCK_INDEX)
            {
                new_block = BLOCK_BEGIN_PAGE_ID;
            }
            Memary_EraseData(new_block); //先擦除当前扇区
        }
        gWriteEnable = 1;
    }
    else
    {
        gLogTable.log_index++;
    }
    gLogTable.counts++;
}
//最后一个参数是时间偏移
void Log_RecordPowerInfo(uint8_t flg, uint16_t voltage, uint8_t tm_offset)
{
    uint8_t i = 0;
    unLogInfoDef new_info;
    time_t time_cnt;
    struct tm time_date;
    uint16_t new_block;

    if (!gWriteEnable)
        return;

    RTC_ReadTime();
    gLogDataPage[gLogTable.log_index].type = LOG_TYPE_POWER;
    gLogDataPage[gLogTable.log_index].tm.year = gColck.year;
    gLogDataPage[gLogTable.log_index].tm.month = gColck.month;
    gLogDataPage[gLogTable.log_index].tm.day = gColck.day;
    gLogDataPage[gLogTable.log_index].tm.hour = gColck.hour;
    gLogDataPage[gLogTable.log_index].tm.minute = gColck.minute;
    gLogDataPage[gLogTable.log_index].tm.second = gColck.second;

    if (tm_offset != 0) //未校时修改开关机时间
    {
        time_cnt = RTC_GetCounter() + 8 * 60 * 60 - gLogTable.run_tm_this; // 读取当前的秒计数器,减去经过的时间
        time_date = *localtime(&time_cnt);                                 // 转换为日期时间格式
        //重写时间
        gLogDataPage[gLogTable.log_index].tm.hour = time_date.tm_hour;
        gLogDataPage[gLogTable.log_index].tm.minute = time_date.tm_min;
        gLogDataPage[gLogTable.log_index].tm.second = time_date.tm_sec;
    }

    new_info.power_info.flg = flg;
    new_info.power_info.bat_vol = voltage;

    for (i = 0; i < 8; i++)
    {
        gLogDataPage[gLogTable.log_index].log_info[i] = new_info.buf[i];
    }
    if (gLogTable.log_index == 15) //最后一条
    {
        gWriteEnable = 0;
        //保存日志
        Memary_WritePageData(gLogTable.this_page.last_page, (uint8_t *)&gLogDataPage[0].type);
        gLogTable.this_page.last_page++; //建立新页
        //_ClearThisPage();
        gLogTable.log_index = 0;
        if (gLogTable.this_page.last_page == MAX_PAGE_INDEX)
        {
            gLogTable.this_page.last_page = BLOCK_BEGIN_PAGE_ID * PAGE_PER_BLOCK;
        }
        if (gLogTable.this_page.last_page % PAGE_PER_BLOCK == 0) //1个扇区结束
        {
            new_block = gLogTable.this_page.last_page / PAGE_PER_BLOCK;
            if (new_block >= MAX_BLOCK_INDEX)
            {
                new_block = BLOCK_BEGIN_PAGE_ID;
            }
            Memary_EraseData(new_block); //先擦除当前扇区
        }
        gWriteEnable = 1;
    }
    else
    {
        gLogTable.log_index++;
    }
    gLogTable.counts++;
}

int16_t Log_SendData(void)
{
    uint16_t temp;

    if (gQueryInfo.date != 0)
    {
        gPageDataBuf[0] = 0xAA;
        gPageDataBuf[1] = 0x55;
        gPageDataBuf[2] = 0;
        gPageDataBuf[3] = TOOL_TYPE;
        gPageDataBuf[4] = 0xFF;
        gPageDataBuf[5] = 1;
        temp = gQueryInfo.end_page - gQueryInfo.start_page + 1; //总包数
        gPageDataBuf[6] = temp;                                 //低字节
        gPageDataBuf[7] = temp >> 8;
        temp = gQueryInfo.this_page - gQueryInfo.start_page + 1; //当前包号
        gPageDataBuf[8] = temp;                                  //低字节
        gPageDataBuf[9] = temp >> 8;
        //当天的最新一页日志还没存，不能从存储器里面读取
        if (gQueryInfo.date == gLogTable.this_page.date && gQueryInfo.this_page == gLogTable.this_page.last_page)
        {
            uint8_t *buf;
            buf = (uint8_t *)&gLogDataPage[0].type;
            for (temp = 0; temp < 256; temp++)
            {
                if (temp < gLogTable.log_index * 16)
                {
                    gPageDataBuf[temp + 10] = buf[temp];
                }
                else
                {
                    gPageDataBuf[temp + 10] = 0;
                }
            }
        }
        else
        {
            Memary_ReadPageData(gQueryInfo.this_page, gPageDataBuf + 10); //读取数据
            if (gQueryInfo.this_page == gQueryInfo.end_page)              //最后一页
            {
                for (temp = gQueryInfo.log_index * 16; temp < 256; temp++) //清楚最后一页没有日志的部分
                {
                    gPageDataBuf[temp + 10] = 0;
                }
            }
        }

        gPageDataBuf[266] = 0;
        for (temp = 2; temp < 266; temp++)
        {
            gPageDataBuf[266] += gPageDataBuf[temp];
        }

        OSMutexPend(USART_MUTEX, 100, &m_err);
        if (OS_ERR_NONE == m_err)
        {
            UART_SendBuf(TOOL_CONTROL_PORT, gPageDataBuf, 267);
            OSMutexPost(USART_MUTEX);
        }
        gQueryInfo.this_page++;
        if (gQueryInfo.this_page > gQueryInfo.end_page)
        {
            gQueryInfo.date = 0;
        }
        return gQueryInfo.end_page - gQueryInfo.this_page; //返回剩余包数
    }
    else
    {
        return 0;
    }
}

//发送查询失败信息
void Log_SendErr(uint8_t err)
{
    gPageDataBuf[0] = 0xAA;
    gPageDataBuf[1] = 0x55;
    gPageDataBuf[2] = 5;
    gPageDataBuf[3] = TOOL_TYPE;
    gPageDataBuf[4] = 0xFF;
    gPageDataBuf[5] = 5;
    gPageDataBuf[6] = err; //低字节
    gPageDataBuf[7] = gPageDataBuf[2] + gPageDataBuf[3] + gPageDataBuf[4] + gPageDataBuf[5] + gPageDataBuf[6];

    OSMutexPend(USART_MUTEX, 100, &m_err);
    if (OS_ERR_NONE == m_err)
    {
        UART_SendBuf(TOOL_CONTROL_PORT, gPageDataBuf, 8);
        OSMutexPost(USART_MUTEX);
    }
}

void Log_SendSysInfo(stNewLogDef table)
{
    uint16_t temp;
    uint8_t i = 0;
    uint8_t *buf;

    gPageDataBuf[0] = 0xAA;
    gPageDataBuf[1] = 0x55;
    gPageDataBuf[2] = 40;
    gPageDataBuf[3] = TOOL_TYPE;
    gPageDataBuf[4] = 0xFF;
    gPageDataBuf[5] = 3;

    buf = (uint8_t *)&table.this_page.date;
    for (i = 0; i < 36; i++)
    {
        gPageDataBuf[6 + i] = buf[i];
    }

    gPageDataBuf[42] = 0;
    for (temp = 2; temp < 42; temp++)
    {
        gPageDataBuf[42] += gPageDataBuf[temp];
    }

    OSMutexPend(USART_MUTEX, 100, &m_err);
    if (OS_ERR_NONE == m_err)
    {
        UART_SendBuf(TOOL_CONTROL_PORT, gPageDataBuf, 43);
        OSMutexPost(USART_MUTEX);
    }
}
//获取系统运行信息
void Log_GetSystemInfo(void)
{
	Log_SendSysInfo(gLogTable);
}
//读取某个日期的数据
void Log_QueryDate(uint8_t year, uint8_t month, uint8_t day)
{
    stNewLogDef log_table;
    uint32_t date;

    date = (year << 16) + (month << 8) + day;
    gQueryInfo.start_page = 0;
    if (date == gLogTable.this_page.date) //当天的日志
    {
        gQueryInfo.start_page = gLogTable.this_page.first_page;
        gQueryInfo.this_page = gQueryInfo.start_page;
        gQueryInfo.end_page = gLogTable.this_page.last_page;
        Log_SendSysInfo(gLogTable);
        gQueryInfo.date = date;
    }
    else
    {
        Fram_ReadData(PAGE_LOG_TABLE + (day - 1) * 32, (uint8_t *)&log_table.this_page.date, 32); //读取当天对应的索引表
        if (log_table.this_page.date == date)                                          //检查日期是否一致
        {
            gQueryInfo.start_page = log_table.this_page.first_page;
            gQueryInfo.this_page = gQueryInfo.start_page;
            gQueryInfo.end_page = log_table.this_page.last_page;
            gQueryInfo.log_index = log_table.log_index - 1; //最后一页的日志数量
            Log_SendSysInfo(log_table);
            gQueryInfo.date = date;
        }
        else
        {
            gQueryInfo.date = 0;
            Log_SendErr(1); //返回该日期没有日志
        }
    }
}
//获取系统时间
void Log_GetSystemClock(void)
{
    uint16_t temp;

    RTC_ReadTime();
    gPageDataBuf[0] = 0xAA;
    gPageDataBuf[1] = 0x55;
    gPageDataBuf[2] = 10;
    gPageDataBuf[3] = TOOL_TYPE;
    gPageDataBuf[4] = 0xFF;
    gPageDataBuf[5] = 4;
    gPageDataBuf[6] = gColck.year; 
    gPageDataBuf[7] = gColck.month;
    gPageDataBuf[8] = gColck.day;
    gPageDataBuf[9] = gColck.hour;
    gPageDataBuf[10] = gColck.minute; //低字节
    gPageDataBuf[11] = gColck.second;
    gPageDataBuf[12] = 0;
    for (temp = 2; temp < 12; temp++)
    {
        gPageDataBuf[12] += gPageDataBuf[temp];
    }

    OSMutexPend(USART_MUTEX, 100, &m_err);
    if (OS_ERR_NONE == m_err)
    {
        UART_SendBuf(TOOL_CONTROL_PORT, gPageDataBuf, 13);
        OSMutexPost(USART_MUTEX);
    }
}
//测试用，读取某页数据
void Log_QueryPage(uint16_t begin, uint16_t end)
{
    //测试，读取某页数据
    gQueryInfo.date = 1;
    gQueryInfo.start_page = begin;
    gQueryInfo.this_page = gQueryInfo.start_page;
    gQueryInfo.end_page = end;
}
//调试
void Log_Debug(uint8_t param)
{
    if (param == 1)
    {
        Memary_EraseWait(0);
    }
    else if (param == 2)
    {
        SaveLog(3);
        Log_init();
    }
    else if (param == 3) //程序升级
    {
        SaveLog(1);
        delay_ms(10);
    }
    else if (param == 4) //擦除日志
    {
        gLogTable.pwr_on_cnts = 0;
        Fram_WriteData(PAGE_NEW_LOG_INFO, (uint8_t *)&gLogTable.this_page.date, 32); //保存索引表
        Log_init();
    }
}
/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    SaveLog(10);//重启
    while (1)
    {
    }
}
