#include "cut_tool.h"
#include "grip.h"
#include "lidischarge.h"
#include "stdio.h"



#define SetPowerOn()		   POWON_GPIO_PORT->BSRR = POWON_GPIO_PIN;  //开机
#define SetPowerOff()		    POWON_GPIO_PORT->BRR = POWON_GPIO_PIN;   //关机

#define IsWorkSensorOn()		(!(SENSOR1_GPIO_PORT->IDR & (SENSOR1_GPIO_PIN)))
#define IsHomeSensorOn()		(!(SENSOR2_GPIO_PORT->IDR & (SENSOR2_GPIO_PIN)))
#define isPowerButtonDown()		(!(POWIN_GPIO_PORT->IDR & POWIN_GPIO_PIN))
u32 hometim = 0;
OS_STK tool_task_stk[TOOL_TASK_STK_SIZE];
ToolStateDef tool_state = TOOL_STATE_MANUL;
static stSteeringMotorDef cut_motor;
ToolMotorStateDef  cut_motor_state, grip_motor_state;
ModuleStateDef gGripModuleState = MODULE_STATE_UNINIT, gCutModuleState = MODULE_STATE_UNINIT;
uint8_t gCameraState = 0;
uint8_t gSensorState = 0;
int32_t gWorkTimes = 0;
uint8_t cut_start_position = 100;
uint16_t gCutPauseTm=0;//切刀从工作到暂停的时间限制
uint32_t gZeroVoltageTim = 0;
OS_TMR *gSoftT1ms = NULL;
uint8_t BatteryLevel;
uint32_t heartbeat_timer;
struct home_detect ht={0};

ToolFaultCode faultcode=FAULT_CODE_NONE;

extern stGripDef gGrip;

OS_EVENT* USART_MUTEX;  //串口互斥量 
uint8_t m_err;

#ifdef	USE_PROTOCOL_V3
 
uint8_t GetBattaryQuantity(void)
{

	static uint16_t runflg=0;
	
	if(runflg<5000)
	{
		runflg+=20;
		BatteryLevel=0;
		PowerLEDFlash(1);
	}else{
		BatteryLevel = (uint8_t)Lisup.CurQtity;
		if (BatteryLevel >= 10) // 电量小于10，1分钟后启动低电压关机功能
		{
			gZeroVoltageTim = 0;
		}
		if (BatteryLevel > 50)
		{
			PowerLEDFlash(1);
		}
		else
		{
			PowerLEDFlash(BatteryLevel * 20);
		}
	}

	return BatteryLevel;
}

void CutTool_SendState(void)    //状态反馈
{
	uint8_t buf[20] = {0xAA,0x55,0x0a,TOOL_TYPE_CUT,0x03,MOTOR_CMD_REPORT,0x00};

	if (OSMutexAccept(USART_MUTEX,&m_err))
	{
		//buf[6] = GetBattaryQuantity();
		buf[6] = BatteryLevel;
		buf[7] = tool_state;
		buf[8] = faultcode;
		buf[9] = gSensorState + (gCameraState << 6);
		buf[10] = cut_motor_state.motion_state + (cut_motor_state.work_state<<2) + (gCutModuleState<<4);
		
		grip_motor_state = Grip_GetMotorState();
		
		buf[11] = grip_motor_state.motion_state+ (grip_motor_state.work_state<<2) + (gGripModuleState<<4);

		buf[12] = buf[2] + buf[3] + buf[4] + buf[5] + buf[6] + buf[7] + buf[8] + buf[9] + buf[10]+ buf[11];
		UART_SendBuf(TOOL_CONTROL_PORT,buf,13);
		OSMutexPost(USART_MUTEX);	
	}
}

#endif
uint32_t CombineU32_LF(uint8_t* data)
{
	return (data[0] + ((uint32_t)data[1]<<8) + ((uint32_t)data[2]<<16) + ((uint32_t)data[3]<<24));
}
uint32_t CombineU32_HF(uint8_t* data)
{
	return (data[3] + ((uint32_t)data[2]<<8) + ((uint32_t)data[1]<<16) + ((uint32_t)data[0]<<24));
}
//经测试延时1us时实际为1.174us
//误差为17.4%
void  delay_us(uint32_t us)
{
	us = us<<3;
	while(us--);
}		
//经测试延时1ms时误差几乎为0
void  delay_ms(uint16_t ms)
{
	while(ms--)
		delay_us(1000);
}		

#ifdef	USE_PROTOCOL_V2
extern uint16_t V3ToV2(void);
uint8_t GetBattaryQuantity(void)
{
	uint8_t bq = 0;
	uint32_t battery_mv = 0;

	battery_mv = ADC1_GetValue_mv(BATTARY_VOLTAGE_CHANNEL)*MV_TO_BATTARY;
	if(battery_mv >= 20000)  bq=100;
	else if(battery_mv <= 17000) bq = 0;
	else
	{
		bq = (battery_mv-17000)/30;
	}

	return bq;
}

uint8_t grip_motor_status(void)
{
	if(Grip_GetModuleState() == MODULE_STATE_MAX)
	{
		return 0x03;
	}
	else if(Grip_GetModuleState() == MODULE_STATE_HOME)
	{
		return 0x04;
	}
	else if(gGripMotorState.work_state == 0)
	{
		return 0x00;
	}
	else if(gGripMotorState.work_state == 1)
	{
		return 0x01;
	}
	else if(gGripMotorState.work_state == 2)
	{
		return 0x02;
	}
}            

void CutTool_SendState(void)
{
	uint8_t buf[12] = {0xAA,0x55,0x07,0,0x4A,0x84};
//	uint16_t state=0;

//	state = V3ToV2();
	buf[6] = GetBattaryQuantity();
	buf[7] = 0x00;
	buf[8] = (tool_state<<6) + (cut_start_position << 5) + (cut_motor_state.work_state << 3) +  grip_motor_status();
	buf[9] = buf[2] + buf[3] + buf[4] + buf[5] + buf[6] + buf[7] + buf[8];
	UART_SendBuf(TOOL_CONTROL_PORT,buf,10);
}
#endif

void GPIO_Config(void)
{
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
/*****************************单目相机开关*************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = CAMERA_GPIO_PIN;	

		/*设置引脚模式为推挽输出*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(CAMERA_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/
    
/******************************SENSOR1传感器****************************///工作极限
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = SENSOR1_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(SENSOR1_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/
		GPIO_InitStructure.GPIO_Pin = WIFIBUTTON_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(GPIOB, &GPIO_InitStructure);	
/******************************SENSOR2传感器****************************///复位极限 
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = SENSOR2_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(SENSOR2_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************POWER_IN电源按键检测****************************/   
		/*选择要控制的GPIO引脚*/
//		GPIO_InitStructure.GPIO_Pin = POWIN_GPIO_PIN;	

//		/*设置引脚模式为浮空输入*/
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    
//		/*调用库函数，初始化GPIO*/
//		GPIO_Init(POWIN_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/


/******************************POWER_ON电源引脚输出****************************/   
//        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
        
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = POWON_GPIO_PIN | WIFI_RELOAD_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(POWON_GPIO_PORT, &GPIO_InitStructure);
        
/****************************************************************/


/******************************BT_LED电源LED输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = BT_LED_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(BT_LED_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/

/******************************状态指示灯输出****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = STATELED_GPIO_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(STATELED_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/

/*****************************WIFI复位****************************/   
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = WIFI_RELOAD_PIN;	

		/*设置引脚模式为浮空输入*/
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        
		/*设置引脚速率为50MHz */   
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
		/*调用库函数，初始化GPIO*/
		GPIO_Init(STATELED_GPIO_PORT, &GPIO_InitStructure);	

/****************************************************************/
	GPIO_SetBits(WIFI_RELOAD_GPIO_PORT,WIFI_RELOAD_PIN);	
	GPIO_SetBits(STATELED_GPIO_PORT,STATELED_GPIO_PIN);
	GPIO_SetBits(CAMERA_GPIO_PORT,CAMERA_GPIO_PIN);
	DisablePowerShut();
	gCameraState = 1;
}

int16_t CutMotor_GetCurrent(void)
{
    int32_t	motor_ma = 0;
	motor_ma = (ADC1_GetValue_mv(MOTOR_CURRENT_CHANNEL) * MV_TO_MOTOR_MA);  //毫安
	return motor_ma;
}

void StateLEDFlash(uint16_t period)
{
	static uint16_t state_tim = 0;
	uint8_t bitstate = 0;

	if(period == 0)
	{
		GPIO_SetBits(STATELED_GPIO_PORT,STATELED_GPIO_PIN);
		return;
	}
	state_tim +=20;
	if (state_tim > period)
	{
		state_tim = 0;
		bitstate = GPIO_ReadOutputDataBit(STATELED_GPIO_PORT,STATELED_GPIO_PIN)^1;
		GPIO_WriteBit(STATELED_GPIO_PORT,STATELED_GPIO_PIN,(BitAction)bitstate);
	}
}

void PowerLEDFlash(uint16_t period)
{
	static uint16_t power_tim = 0;
	uint8_t bitstate = 0;

	if(period <= 1)
	{
		//bitstate = period^1;
		GPIO_WriteBit(BT_LED_GPIO_PORT,BT_LED_GPIO_PIN,(BitAction)period);
		return;
	}
	power_tim +=20;
	if (power_tim > period)
	{
		power_tim = 0;
		bitstate = GPIO_ReadOutputDataBit(BT_LED_GPIO_PORT,BT_LED_GPIO_PIN)^1;
		GPIO_WriteBit(BT_LED_GPIO_PORT,BT_LED_GPIO_PIN,(BitAction)bitstate);
	}
}

void PowerButtonScan(void)
{
	static uint8_t power_state = 0;
	static uint16_t gBt_power_tm=0;
	uint16_t bq;

//	if (isPowerButtonDown())
//	{
//		gBt_power_tm +=20 ;
//		if (!power_state)//如果没电
//		{
//			if (gBt_power_tm >= 15000)//15秒
//			{
//				PowerLEDFlash(100);//按钮灯快闪
//				CutTool_Control(0,TOOL_CMD_FACTORY_RESET,0);
//			}
//			else if (gBt_power_tm >= 10000)
//			{
//				PowerLEDFlash(500);  
//				GPIO_ResetBits(GPIOB,WIFI_RELOAD_PIN);
//			}
//			else if (gBt_power_tm >= 500)
//			{
//				PowerLEDFlash(1);
//				SetPowerOn();
//			}
//		}
//		else
//		{			
//			if (gBt_power_tm >= 15000)
//			{
//			}
//			else if (gBt_power_tm >= 10000)
//			{
//				gWorkTimes = 0;
//				WriteUserSettings(VER_MODIFY_TIMES,0);
//				WriteUserSettings(WORK_TIMES,gWorkTimes);
//				PowerLEDFlash(1);
//			}
//			else if (gBt_power_tm >= 5000)
//			{
//				PowerLEDFlash(200);
//			}
//			else if (gBt_power_tm >= 2000)
//			{
//				PowerLEDFlash(100);
//			}
//			else
//			{
//				PowerLEDFlash(0);
//			}
//		}
//	}
//	else
//	{	
//		if (!power_state)
//		{
//			power_state = 1;
//			GPIO_SetBits(GPIOB,WIFI_RELOAD_PIN);
//		}	
//		else
//		{
//			if (gBt_power_tm >= 10000)
//			{
//				SaveUserSettings();
//			}
//			else if (gBt_power_tm >= 5000)
//			{
//			}
//			else if (gBt_power_tm >= 2000) //2S关机
//			{
//				if (power_state)
//				{
//					power_state = 0;
//					WriteUserSettings(VER_MODIFY_TIMES,ReadUserSettings(VER_MODIFY_TIMES)-1);
//					SaveUserSettings();
//					#ifdef USE_BOOTLOADER
//					iapUserSettings[IAP_WORK_TIMES]=gWorkTimes;
//					IAP_FUN_WriteFlag(IAP_WRITE_POWER,0,POWER_OFF_DATA); 
//					#endif

//					delay_ms(10);
//					SetPowerOff();
//				}
//			}
//			else
//			{
//				bq = GetBattaryQuantity();
//				if (bq > 50)
//				{
//					PowerLEDFlash(0);
//				}
//				else if(bq == 0)
//				{
//					if(runflg>=3000)
//					{
//						GPIO_WriteBit(GPIOB,BT_LED_GPIO_PIN,0);
//					}
//				}
//				else
//				{
//					PowerLEDFlash(bq*20);
//				}
//			}
//		}
//		if(gBt_power_tm != 0)
//		{
//			gBt_power_tm = 0;
//		}
//	}
}

void CutMotor_SetPWM(uint16_t DutyRadio)
{
	PR_Time3CH2PWMSet(DutyRadio);
}

void InitAllMotors(void)
{
	cut_motor.index = 0;
	cut_motor.type = STEERING_MOTOR_TYPE_SPEED;
	cut_motor.min = 0;
	cut_motor.max = 10000;
	cut_motor.offset = STEERING_MOTOR_SPEED_RANGE/3 - 340;//实际范围10000~20000
	cut_motor.origin = 5000;
	cut_motor.current = 0;
	cut_motor.SetPwmFun = CutMotor_SetPWM;
	Steering_Motor_Init(&cut_motor);
	Steering_Motor_Move(&cut_motor,cut_motor.origin);
			
	Grip_Init(0);
}

void Creat_softTimer(void)
{
    uint8_t err;

	if (gSoftT1ms == NULL)
	{
		gSoftT1ms = OSTmrCreate(100, 1, OS_TMR_OPT_PERIODIC, SoftTim1ms_callback, NULL, NULL, &err);
		OSTmrStart(gSoftT1ms, &err);
	}
}

void InitAllDevice(void)
{
	uint16_t succrunFLAG;
	USART_MUTEX = OSMutexCreate(MUTEX_TASK_PRIO,&m_err);
	GPIO_Config();
	framflash_init();
	#ifdef USE_BOOTLOADER
	IAP_ReadALLFlag(); //读取所有IAP相关标志位
	succrunFLAG = (uint16_t)IAP_ReadSingleFlag(FINISH_RUNNING_ADDR);
	if(succrunFLAG != SUCC_RUNNING_DATA)
	{
		IAP_FUN_WriteFlag(IAP_WRITE_RUNNING,SUCC_RUNNING_DATA,0); 
	}
	#endif
	CanBus_Init(0,CAN_BURD_1M);
	ADC1_Config();
	delay_ms(100);
	norflash_init();
	ADC1_SetOffset(MOTOR_CURRENT_CHANNEL);
	USART_Config(TOOL_CONTROL_PORT,115200);
	PR_Time3PWMInit(TIME3_PWM_FREQUENCY);
	IWDG_Init(1000);		//看门狗1S超时
	InitUserSettings(1);
	InitAllMotors();
	Log_init();
	WriteUserSettings(VER_SOFT,SOFT_VERSION);
	WriteUserSettings(VER_SOFT_DATE,SOFT_DATE);
	gWorkTimes = ReadUserSettings(WORK_TIMES);
	if(IsHomeSensorOn())
	{
		gCutModuleState = MODULE_STATE_HOME;
		cut_motor_state.work_state = WORK_STATE_HOME;
	}
	Creat_softTimer();
}
void SendDeviceInfo(void)
{
	uint8_t SendBuf[20] = {0xAA,0x55,14,TOOL_TYPE_CUT,0};
	uint32_t device_id = 0;
	uint8_t i=0;
	OSMutexPend(USART_MUTEX,100,&m_err);
	if (OS_ERR_NONE == m_err)
	{
	SendBuf[5] = DEVICE_CMD_INFO;
	SendBuf[6] = (uint8_t)SOFT_VERSION;
	SendBuf[7] = (uint8_t)(SOFT_VERSION>>8);
	SendBuf[8] = (uint8_t)SOFT_DATE;
	SendBuf[9] = (uint8_t)(SOFT_DATE>>8);
	SendBuf[10] = (uint8_t)(SOFT_DATE>>16);
	SendBuf[11] = (uint8_t)(SOFT_DATE>>24);
	device_id = ReadUserSettings(VER_DEVICE_ID);
	SendBuf[12] = (uint8_t)device_id;
	SendBuf[13] = (uint8_t)(device_id>>8);
	SendBuf[14] = (uint8_t)(device_id>>16);
	SendBuf[15] = (uint8_t)(device_id>>24);

	for (i = 2; i < 16; i++)
	{
		SendBuf[16] += SendBuf[i];
	}
	
	UART_SendBuf(TOOL_CONTROL_PORT,SendBuf,17);
	OSMutexPost(USART_MUTEX);
	}
	
}
void CutTool_CommandReply(uint8_t motor, uint8_t cmd, uint8_t state)  //命令反馈
{
	uint8_t buf[10] = {0xAA,0x55,0x05,TOOL_TYPE_CUT,0x00,0x00,0x00};

	OSMutexPend(USART_MUTEX,100,&m_err);
	if (OS_ERR_NONE == m_err)
	{
		buf[4] = motor;
		buf[5] = cmd;
		buf[6] = state;
		buf[7] = buf[2] + buf[3] + buf[4] + buf[5] + buf[6];
		UART_SendBuf(TOOL_CONTROL_PORT,buf,8);
		OSMutexPost(USART_MUTEX);	
	}	
}

uint8_t CutTool_GetState(void)
{
	return tool_state;
}
//配置WiFi模块，打开网络功能
void CoifigWifiNet(void)
{
	uint8_t cmd[20] = {0};	
	
	SetPowerOn();
	delay_ms(10000);
	strcpy((char*)cmd,"+++");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,3);	
	delay_ms(1000);
	strcpy((char*)cmd,"a");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,1);	
	delay_ms(3000);
	strcpy((char*)cmd,"AT+FEPHY=on\r");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,12);	
	delay_ms(3000);
	strcpy((char*)cmd,"AT+FVER=z\r");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,10);	
	delay_ms(3000);
	strcpy((char*)cmd,"AT+FUARTTE=fast\r");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,16);	
	delay_ms(3000);
	strcpy((char*)cmd,"AT+ENTM\r");
	UART_SendBuf(TOOL_CONTROL_PORT,cmd,8);
	delay_ms(3000);
}

CommandReturn CutMotorControl(uint8_t cmd, int8_t data)
{
	CommandReturn ret = RT_OK;

	switch(cmd)
	{
	case MOTOR_CMD_STOP:
		Steering_Motor_Move(&cut_motor,cut_motor.origin);
		cut_motor_state.work_state = WORK_STATE_IDLE;
		gCutModuleState = MODULE_STATE_NOMAL;
		ht.aver_curent = 0;
		ht.betime=0;
		ht.motiontim=0;
		break;
	case MOTOR_CMD_CCW:
		if (cut_motor_state.work_state == WORK_STATE_IDLE)		
		{
			Steering_Motor_Move(&cut_motor,8000);
			gCutModuleState = MODULE_STATE_NOMAL;
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_CW:
		if (cut_motor_state.work_state == WORK_STATE_IDLE)		
		{
			Steering_Motor_Move(&cut_motor,2000);
			gCutModuleState = MODULE_STATE_NOMAL;
		}
		else
			ret = RT_MOTOR_NOT_MANUL;
		break;
	case MOTOR_CMD_WORK:
		if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			cut_motor_state.work_state = WORK_STATE_WORK;
			if(gCutModuleState != MODULE_STATE_MAX)
			{
				Steering_Motor_Move(&cut_motor,(5000- (ReadUserSettings(CUT_CUT_SPEED_1)>>1)));
				DisablePowerShut();
				ht.aver_curent = 0;
				ht.betime=0;
				ht.motiontim=0;
			}
			else
				ret = RT_MOTOR_IS_LIMIT;
		}
		else
			ret = RT_MOTOR_NOT_STOP;
		break;
	case MOTOR_CMD_HOME:
		if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			cut_motor_state.work_state = WORK_STATE_HOME;
			if(gCutModuleState != MODULE_STATE_HOME)
			{
				Steering_Motor_Move(&cut_motor,(5000 + (ReadUserSettings(CUT_HOME_SPEED)>>1)));
				ht.aver_curent = 0;
				ht.betime=0;
				ht.motiontim=0;
			}
			else
				ret = RT_MOTOR_IS_HOME;
		}
		else
			ret = RT_MOTOR_NOT_STOP;
		break;
	}

	return ret;
}
void cuttool_FailRecoder(uint8_t modulecode,uint8_t info_0, int16_t info_1)
{
	gFault.module = modulecode;//B1
	gFault.tool_state = tool_state; //B3
	gFault.info_0 = info_0;
	gFault.info_1 = info_1;
}
void CutTool_Control(uint8_t motor, uint8_t cmd, uint8_t* data)
{
	int16_t speed;
	CommandReturn rt = RT_OK;

	if (!motor)
	{
		switch(cmd)
		{
			case TOOL_CMD_STOP:
				Grip_Control(MOTOR_CMD_STOP,0);
				CutMotorControl(MOTOR_CMD_STOP,0);
				gFault.fault = FAULT_CODE_NONE;
				tool_state = TOOL_STATE_STOPPING;//改变工具状态为停止
				break;
			case TOOL_CMD_READY:
				if (tool_state == TOOL_STATE_HOMED)
				{
					Grip_Control(MOTOR_CMD_WORK,0);//
					tool_state = TOOL_STATE_PREPARING;
					gCutPauseTm = 0;
				}else if(gCutModuleState == MODULE_STATE_HOME){
					if(gGrip.state == MODULE_STATE_N_LIM||gGrip.state==MODULE_STATE_HOME)
					{
						Grip_Control(MOTOR_CMD_WORK,0);//
						tool_state = TOOL_STATE_PREPARING;
						gCutPauseTm = 0;
					}else if(gGrip.state== MODULE_STATE_MAX)
					{
						tool_state = TOOL_STATE_READY;
					}else{
						rt = RT_TOOL_NOT_HOME;
					}					
				}
				else
					rt = RT_TOOL_NOT_HOME;
				break;
			case TOOL_CMD_HOME:
				if (tool_state != TOOL_STATE_WORK&&tool_state!= TOOL_STATE_PREPARING)
				{
					Grip_Control(MOTOR_CMD_HOME,0);
					CutMotorControl(MOTOR_CMD_HOME,0);
					hometim=0;
					gFault.fault = FAULT_CODE_NONE;
					tool_state = TOOL_STATE_HOMING;
				}
				else
					rt = RT_TOOL_WORKING;
				break;
			case TOOL_CMD_WORK:
				if(BatteryLevel > ReadUserSettings(POWER_LOW_LIMIT))
				{
					if (*data == 0) //工作准备
					{
						if(tool_state == TOOL_STATE_READY)
						{
							CutMotorControl(MOTOR_CMD_WORK,0);
							tool_state = TOOL_STATE_WORK;
							DisablePowerShut();
						}
						else
							rt = RT_TOOL_NOT_READY;
					}
					else if (*data == 1) //一键工作
					{
						if(tool_state == TOOL_STATE_PAUSE)
						{
							Steering_Motor_Move(&cut_motor,(5000- (ReadUserSettings(CUT_CUT_SPEED_1)>>1)));
							tool_state = TOOL_STATE_WORK;
							DisablePowerShut();
						}
						else
							rt = RT_TOOL_NOT_PAUSE;
					}
				}
				else
					rt = RT_TOOL_LOW_BAT;
				break;
			case TOOL_CMD_UNLOCK:
					if (tool_state == TOOL_STATE_MANUL)
					{
						hometim=0;
						rt = Grip_Control(MOTOR_CMD_UNLOCK,0);
						if(rt==RT_OK)
						{
							tool_state = TOOL_STATE_UNLOCKING;
						}
					}
          else
					rt = RT_TOOL_NOT_MANUL;
				break;
			case TOOL_CMD_RESET://复位电机状态
				if (tool_state == TOOL_STATE_MANUL)
				{
					Grip_Control(MOTOR_CMD_SET,0);
				}
				else
					rt = RT_TOOL_NOT_MANUL;
			break;	
			case TOOL_CMD_FACTORY_RESET://恢复出厂设置
				if (tool_state == TOOL_STATE_MANUL)
				{
					iapUserSettings[IAP_WORK_TIMES] = ReadUserSettings(WORK_TIMES);
					InitUserSettings(0);
				}
				else
					rt = RT_TOOL_NOT_MANUL;
			break;
//			case TOOL_CMD_SHUT:
//				WriteUserSettings(VER_MODIFY_TIMES,ReadUserSettings(VER_MODIFY_TIMES)-1);
//				SaveUserSettings();
//				delay_ms(10);
//				SetPowerOff();
//			break;
			default: return;
		}
	}
	else if(motor == 0xFF)	//日志
	{
		if (cmd == 1)//日志查询
		{
			if (tool_state == TOOL_STATE_MANUL)
			{
				Log_QueryDate(data[0], data[1], data[2]);
				return;
			}
			else
			{
				rt = RT_TOOL_NOT_MANUL;
			}
		}
		else if (cmd == 2)//校时
		{
			RTC_SetTime(data);
		}
		else if (cmd == 3) //查询系统信息
		{
			Log_GetSystemInfo();
			return;
		}
		else if (cmd == 4) //查询系统时间
		{
			Log_GetSystemClock();
			return;
		}
		else if (cmd == 5) //按页查询日志
		{
			Log_QueryPage(data[0] + (data[1] << 8), data[2] + (data[3] << 8));
			return;
		}
		else if (cmd == 16) //调试
		{
			Log_Debug(data[0]);
			return;
		}
	}
	else
	{
		if (TSB(motor,CAMERA_POWER))
		{
			if (cmd == 0)
			{				
				GPIO_ResetBits(CAMERA_GPIO_PORT, CAMERA_GPIO_PIN);
				gCameraState = 0;
			}
			else
			{
				GPIO_SetBits(CAMERA_GPIO_PORT, CAMERA_GPIO_PIN);			
				gCameraState = 1;
			}
		}
		else
		{
			speed = (int8_t)*data;
			if (tool_state == TOOL_STATE_WORK)
			{
				if (cmd != MOTOR_CMD_STOP)
				{
				rt = RT_TOOL_WORKING;
				CutTool_CommandReply(motor,cmd,rt);
				return;
				}
			}
	//		tool_state = TOOL_STATE_MANUL;

			if (TSB(motor,MOTOR_CUT))
			{
				rt = CutMotorControl(cmd,speed);
				if(rt == RT_OK)
				{
					tool_state = TOOL_STATE_MANUL;
				}
			}
			if (TSB(motor,MOTOR_GRIP))
			{
				rt = Grip_Control(cmd,speed);
				if(rt == RT_OK)
				{
					tool_state = TOOL_STATE_MANUL;
				}
			}			
		}
	}
	CutTool_CommandReply(motor,cmd,rt);
}

int16_t Cutter_GetMotorCurrent(void)
{
	static int16_t cutter_currentbuf[10];
	static uint8_t cutter_current_index = 0;
	uint8_t i = 0;
	int32_t temp = 0;
	int16_t rescurrent=0;
	//电流10次平均值滤波
	cutter_currentbuf[cutter_current_index] = CutMotor_GetCurrent();
	cutter_current_index++;
	if (cutter_current_index == 10)
		cutter_current_index = 0;
	for (i = 0; i < 10; i++)
		temp += cutter_currentbuf[i];
	rescurrent = temp / 10;

	return rescurrent;
}

void Send_val(int32_t val)
{
	static int temp[5] = {0};
	static uint8_t tindex = 0;
	int i=0;
	uint8_t buf[40] = {0xCC,0x55};
	int vol;

	temp[tindex++]=val;
	if(tindex==5)
	{
			buf[2] = 0x15;
			
			for(i=0;i<5;i++)
			{
				buf[3+(i*4)] = temp[i];
				buf[4+(i*4)] = temp[i]>>8;
				buf[5+(i*4)] = temp[i]>>16;
				buf[6+(i*4)] = temp[i]>>24;
				buf[23] = buf[23] + buf[3+(i*4)] +buf[4+(i*4)]+buf[5+(i*4)]+buf[6+(i*4)];
			}
			buf[23] = buf[23] +buf[2];
			if (OSMutexAccept(USART_MUTEX,&m_err))
			{
				UART_SendBuf(TOOL_CONTROL_PORT,buf,24);
				OSMutexPost(USART_MUTEX);	
			}
			tindex = 0;
	}
}

void WifiButtonScan(void)
{
	static uint16_t gBt_power_tm = 0;

	if (isWifiButtonDown())
	{
		gBt_power_tm+=20;
		if (gBt_power_tm >= 15000) // 15秒后指示电量状态
		{
			PowerLEDFlash(1);
		}
		else if (gBt_power_tm >= 10000) // 10秒后重置完成，指示灯常亮
		{
			GPIO_SetBits(GPIOB,WIFI_RELOAD_PIN); // 重置完成
			PowerLEDFlash(100);
		}
		else if (gBt_power_tm >= 5000) // 5秒后快闪，开始重置，此时可松开按钮
		{
			PowerLEDFlash(100);
			GPIO_ResetBits(GPIOB,WIFI_RELOAD_PIN);
		}
		else // 5秒内慢闪
		{
			PowerLEDFlash(500);
		}
	}
	else
	{
		if (gBt_power_tm >= 5000) // 已重置网络
		{
			gBt_power_tm+=20;
			if (gBt_power_tm >= 15000) // 15秒后指示电量状态
			{
				gBt_power_tm = 0;
			}
			else if (gBt_power_tm >= 10000) // 10秒后重置完成，指示灯常亮
			{
				GPIO_SetBits(GPIOB,WIFI_RELOAD_PIN); // 重置完成
				//PowerLEDFlash(1);
			}
			else
			{
				//PowerLEDFlash(500);
			}
		}
		else
		{
			gBt_power_tm = 0;
		}
		GetBattaryQuantity();
	}
}


int16_t test_cuttent[500] = {0};
uint16_t test_index = 0;
int16_t this_current = 0;
uint16_t tm_cut_1 = 0;
uint16_t test_tm2=0;

void StripTool_Heartbeat_clear(void)
{
	heartbeat_timer = 0;

	if (gFault.fault == err_tool_communicate_fault)
	{
		gFault.fault = 0;
	}	
}

void SoftTim1ms_callback(void *ptmr, void *parg)
{
	Grip_Update();
	Cutter_GetMotorCurrent();
	if (ReadUserSettings(TOOL_CONTROL_WORD_HEARTBEAT) > 0)
	{
		//工作状态下心跳计数器清零，不计数
		if (tool_state == TOOL_STATE_WORK)
		{
			StripTool_Heartbeat_clear();
		}			
		if (heartbeat_timer == ReadUserSettings(TOOL_CONTROL_WORD_HEARTBEAT)*1000)
		{
			heartbeat_timer++;
			//复位
			if(cut_motor_state.motion_state!=MOTION_STATE_STOPPED)
			{
				CutTool_Control(1,MOTOR_CMD_STOP,0);
			}
			CutTool_Control(2,MOTOR_CMD_STOP,0);
			CutTool_Control(0,TOOL_CMD_HOME,0);
			gFault.fault = err_tool_communicate_fault;
			gFault.tool_state = tool_state;
			gFault.bat_vol = Lisup.RealVol;
			gFault.module = 0;
			gFault.info_1 = heartbeat_timer / 1000;
		}
		else
		{
			heartbeat_timer++;
		}
	}	
	if(IsHomeSensorOn())
	{			
		STB(gSensorState,1);
		if(cut_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			gCutModuleState = MODULE_STATE_HOME;

			
			if(tool_state == TOOL_STATE_HOMING)
			{
				if (gGripModuleState == MODULE_STATE_HOME)
				{
					tool_state = TOOL_STATE_HOMED;						
				}
			}
		}
		else if (cut_motor_state.motion_state == MOTION_STATE_CCW)
		{
			Steering_Motor_Move(&cut_motor,cut_motor.origin);
		}
	}
	else if(IsWorkSensorOn())
	{
		STB(gSensorState,0);
		if(cut_motor_state.motion_state == MOTION_STATE_STOPPED)
		{
			gCutModuleState = MODULE_STATE_MAX;
			test_index = 0;
		}
		else if (cut_motor_state.motion_state == MOTION_STATE_CW)
		{
			Steering_Motor_Move(&cut_motor,cut_motor.origin);
		}			
	}
	else
	{			
		gSensorState = 0;
		if(tool_state == TOOL_STATE_HOMING)
		{
			//this_current = Cutter_GetMotorCurrent();
			//homedetect();
			if(hometim<18000)
			{
				hometim +=1;
			}else{
				Steering_Motor_Move(&cut_motor,cut_motor.origin);
				Grip_Control(MOTOR_CMD_STOP,0);
				tool_state = TOOL_STATE_HOMEFAILURE;
				gFault.fault = FAULT_HOMESENSOR;
				cuttool_FailRecoder(SCREW_MOD,0,0);
				hometim=0;
			}
		}
	}
	
}

void homedetect(void)
{
	static int32_t temp=0;
	if(ReadUserSettings(CUT_DETECT_ENABLE)==0)
	{
		return ;
	}
	ht.motiontim+=20;
	if(ht.motiontim>500)
	{
		if(ht.flg==0&&ht.motiontim > ReadUserSettings(CUT_DETECT_TIME_1)) 
		{
			ht.aver_curent = ht.aver_curent/ht.betime;
			WriteUserSettings(CUT_DETECT_CURRENT,ht.aver_curent);
			ht.flg=1;
		}else if(ht.flg==1&&ht.motiontim > ReadUserSettings(CUT_DETECT_TIME_1))
		{
			if(ABS(this_current)>(ht.aver_curent*ReadUserSettings(CUT_DETECT_MAXRATIO)/10)||ABS(this_current)<(ht.aver_curent*10/ReadUserSettings(CUT_DETECT_MAXRATIO)))
			{
				WriteUserSettings(CUT_DETECT_MAX,ABS(this_current));
				Steering_Motor_Move(&cut_motor,cut_motor.origin);
				tool_state = TOOL_STATE_MANUL; 
				gFault.fault = FAULT_HOMESENSOR;
				tm_cut_1 = 0;
				cuttool_FailRecoder(SCREW_MOD,this_current,ht.aver_curent);
				ht.flg=2;
			}
		}else if(ht.flg==0&&ht.motiontim <= ReadUserSettings(CUT_DETECT_TIME_1))
		{
			ht.betime++;
			ht.aver_curent = ht.aver_curent + this_current;
		}
	}else{
		ht.flg=0;
		ht.aver_curent=0;
		ht.betime=0;
	}
}


void Task_Tool(void *p_arg)
{
  	uint8_t cut_step = 0;
	uint16_t tm_cut = 0;
	
	uint16_t tm_cut_2 = 0;
	uint32_t tm_cut_tatal = 0;
	static uint8_t powofflg=0;
	int16_t last_bat = 0;

	for(;;)
	{
		StateLEDFlash(500);
		WifiButtonScan();
		gGripModuleState = Grip_GetModuleState();
		
		if ((cut_motor.current >= 9660)&&(cut_motor.current < 14600))    //正转
		{
			cut_motor_state.motion_state = MOTION_STATE_CW;		
		}
		else if((cut_motor.current > 14700)&&(cut_motor.current < 19660))   //反转
		{
			 cut_motor_state.motion_state = MOTION_STATE_CCW;
		}
		else if((cut_motor.current <= 14700)&&(cut_motor.current >= 14600))    //停止
		{
			 cut_motor_state.motion_state = MOTION_STATE_STOPPED;
		}

		if (tool_state == TOOL_STATE_WORK)
		{
			tm_cut_1 += 20;
			tm_cut_tatal+=20;
			this_current = Cutter_GetMotorCurrent();
			if (cut_step == 0)//开始断线
			{
				if(!IsHomeSensorOn())
				{
					Steering_Motor_Move(&cut_motor,7500);
					cut_step = 6;	
				}else{
				Steering_Motor_Move(&cut_motor,0);			
				cut_step = 1;	
				}
				tm_cut_1 = 0;
				tm_cut_tatal=0;
			}else if(cut_step == 6) //复位过程
			{
				//homedetect();
				if(IsHomeSensorOn())
				{
					tm_cut_1 = 0;
					cut_step=7;
				}else if(tm_cut_1>=18000)
				{
					Steering_Motor_Move(&cut_motor,cut_motor.origin);
					tm_cut_1=0;
					tool_state = TOOL_STATE_FAILURE;
					gFault.fault = FAULT_HOMESENSOR;
					cuttool_FailRecoder(SCREW_MOD,cut_step,0);
				}
			}else if(cut_step == 7)
			{
				if(tm_cut_1>=1200)
				{
					Steering_Motor_Move(&cut_motor,0);			
					cut_step = 1;	
					tm_cut_1 = 0;
				}
			}
			else if (cut_step == 1)
			{				
				if(ReadUserSettings(CUT_CUT_CURRENT_1) > 1000)//如果设置了电流切刀线皮后自动暂停
				{
					if(tm_cut_1 > 1000) 
					{
						if (tm_cut_1 > gCutPauseTm || this_current > ReadUserSettings(CUT_CUT_CURRENT_1))
						{
							Steering_Motor_Move(&cut_motor,cut_motor.origin);
							test_tm2 = tm_cut_1;
							tm_cut_1 = 0;
							cut_step =10;
						}	
						else if(IsWorkSensorOn() || (tm_cut_1 > 20000))//超时失败
						{
							Steering_Motor_Move(&cut_motor,cut_motor.origin);
							tm_cut_1 = 0;
							cut_step =11;
						}
					}
				}
				else//如果没有设置电流参数则没有切刀暂停功能
				{
					tm_cut_1 = 0;
					cut_step =2;
				}
			}
			else if (cut_step == 2)//通过传感器检测断线完成
			{
				if(IsWorkSensorOn())
				{
					Steering_Motor_Move(&cut_motor,cut_motor.origin);
					tm_cut_1 = 0;
					cut_step =3;
				}else if((tm_cut_1 > 20000))
				{
					Steering_Motor_Move(&cut_motor,cut_motor.origin);
					tm_cut_1 = 0;
					cut_step=11;
				}
			}
			else if (cut_step == 3)//切刀复位
			{
				if(tm_cut_1 > 1000)
				{
					CutMotorControl(MOTOR_CMD_HOME,0);
					tm_cut_1 = 0;
					cut_step = 4;
				}
			}
			else if (cut_step == 4)//断线成功
			{
				if (cut_motor_state.motion_state == MOTION_STATE_CCW)
				{
					//homedetect();
					if (IsHomeSensorOn() || tm_cut_1 > 20000)
					{
						Steering_Motor_Move(&cut_motor,cut_motor.origin);
						tm_cut_1 = 0;
						cut_step = 5;
					}
				}else if(cut_motor_state.motion_state== MOTION_STATE_STOPPED)
				{
					if(IsHomeSensorOn())
					{
						Steering_Motor_Move(&cut_motor,cut_motor.origin);
						tm_cut_1 = 0;
						cut_step = 5;
					}
				}
			}
			else if (cut_step == 5)//等待切刀停止
			{
				if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
				{
					tool_state = TOOL_STATE_SUCCEED;
					gWorkTimes++;
					WriteUserSettings(WORK_TIMES,gWorkTimes);
					SaveUserSettings();
					gNomalInfo.info_code=1;
					gNomalInfo.tool_state = tool_state;
					gNomalInfo.param_0=gCutPauseTm;
					gNomalInfo.param_1=tm_cut_tatal;
					gNomalInfo.param_2=gWorkTimes;
					tm_cut_tatal=0;
					Log_RecordNomalInfo(&gNomalInfo);
				}
			}
			else if (cut_step == 10)//断线暂停
			{				
				if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
				{
					tool_state = TOOL_STATE_PAUSE;
					tm_cut_1 = 0;
					cut_step =2;
				}
			}
			else if (cut_step == 11)//断线失败
			{				
				if (cut_motor_state.motion_state == MOTION_STATE_STOPPED)
				{
					tool_state = TOOL_STATE_FAILURE;
					gFault.fault = FAULT_CUTWOKE;
					cuttool_FailRecoder(SCREW_MOD,cut_step,0);
					cut_step =0;
				}
			}
			
		}
		else if (tool_state == TOOL_STATE_STOPPING)
		{
			if(grip_motor_state.motion_state == MOTION_STATE_STOPPED && cut_motor_state.motion_state == MOTION_STATE_STOPPED)
			{
				tool_state = TOOL_STATE_MANUL;
			}
		}
		else if (tool_state == TOOL_STATE_UNLOCKING)
		{
			tm_cut_2 += 20;		
			if (tm_cut_2 > 30000)//解锁30s超时失败
			{
				tool_state = TOOL_STAT_UNLOCKFAILURE;
				tm_cut_2 = 0;
			}
			else
			{
				if (gGripModuleState == MODULE_STATE_HOME)
				{
					tool_state = TOOL_STATE_UNLOCKED;
					tm_cut_2 = 0;
				}
			}
		}
		else
		{
			tm_cut_2 = 0;
			if(tool_state != TOOL_STATE_PAUSE)
			{
				cut_step = 0;
				tm_cut_1 = 0;
			}
		}
		
		if(tool_state == TOOL_STATE_PREPARING)
		{
			gCutPauseTm += 20;

			if (gGrip.state == MODULE_STATE_MAX)
			{
				tool_state = TOOL_STATE_READY;						
			}
			else if (gGrip.state == MODULE_STATE_P_LIM)
			{
				tool_state = TOOL_STATE_ERROR1;						
			}
		}
		
		if(tool_state == TOOL_STATE_HOMING)
		{
			if(gGripModuleState==MODULE_STATE_ERROR)
			{
				tool_state=TOOL_STATE_HOMEFAILURE;
			}
		}
		if(tool_state == TOOL_STATE_UNLOCKING)
		{
			if(gGripModuleState==MODULE_STATE_ERROR)
			{
				tool_state=TOOL_STAT_UNLOCKFAILURE;
			}
		}




		if(cut_motor_state.work_state == WORK_STATE_WORK)
		{
			if(cut_motor_state.motion_state == MOTION_STATE_CW)
			{
				gCutModuleState = MODULE_STATE_WORKING;
				
//				test_cuttent[test_index++] = this_current;
//				if (test_index == 500)
//				{
//						test_index = 0;
//				}
			}
		}
		else if(cut_motor_state.work_state == WORK_STATE_HOME)
		{
			if(cut_motor_state.motion_state == MOTION_STATE_CCW)
			{
				gCutModuleState = MODULE_STATE_HOMING;
			}
		}
		// else if((tool_state != TOOL_STATE_UNLOCKING)&&((tool_state != TOOL_STATE_UNLOCKED)))
		// {
		// 	tool_state = TOOL_STATE_MANUL;
		// }
		//Send_val(cut_step);
		//检测故障并存储日志
		//存储的信息是故障前的模块状态，故障时的电机电流，故障时的工具状态
		if (gFault.fault != 0)
		{
			if (faultcode == 0)
			{
				Realtime_Qtity();
				gFault.bat_vol = Lisup.RealVol;
				if (gFault.module != 0) //不是工具
				{
					gFault.tool_state = tool_state;
				}
				Log_RecordFaultInfo(&gFault);
				faultcode = gFault.fault;
			}
		}
		else
		{
			faultcode = 0;
		}
		
				//电压第一次低于15V记录
		if (Lisup.RealVol > 18000)
		{
			last_bat = Lisup.RealVol;
		}
		else if (Lisup.RealVol < 15000)
		{
			if (last_bat > 18000)
			{
				//电池电压低
				Log_RecordPowerInfo(4, Lisup.RealVol, 0);
			}
			last_bat = Lisup.RealVol;
		}
		if(tm_cut%10==0)
		{
			Log_SendData();
		}
		
		
		tm_cut += 20;
		Realtime_Qtity();
		if (tm_cut > 100)
		{
			CutTool_SendState();
			tm_cut = 0;
		}
		
		gZeroVoltageTim += 20;
		if (gZeroVoltageTim > 60000) 
		{
				if(tool_state != RT_TOOL_WORKING&&powofflg==0)
				{
					powofflg=1;
					gZeroVoltageTim=0;
					WriteUserSettings(VER_MODIFY_TIMES,ReadUserSettings(VER_MODIFY_TIMES)-1); //1.19
					SaveUserSettings(); //保存配置
					delay_ms(10);	
					#ifdef USE_BOOTLOADER
					iapUserSettings[IAP_WORK_TIMES] = gWorkTimes;
					#endif
					delay_ms(20);
					EnablePowerShut();		
			}
		}
		//心跳断联检测
		//心跳时间间隔为1000ms,连续超过10分钟（默认值）未收到心跳，记录故障，同时复位抱紧和旋转
		//配置参数为0时，关掉该功能


		OSTimeDly(20);
		IWDG_Feed();
	}
}


