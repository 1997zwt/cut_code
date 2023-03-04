#ifndef  _LI_DISCHARGE_
#define  _LI_DISCHARGE_


#include "stm32f10x.h"

struct li_battery{
	int16_t jitterVol; //第一阶段抖动电压，一般也是第二阶段的抖动电压
	
	int16_t StartVol;//第一阶段开始电压
	float SlopeVol;//第一阶段电压斜率
	int16_t SlopeQ;  //第一阶段容量斜率	
	int16_t EndVol;  //第一阶段结束电压 mv
	float EndQtity;  //第一阶段结束容量 0~100
		
	int16_t time;
	
	int16_t DownRang;  //正常电压下降范围
};

enum stage{
	up,
	down,
	keep
};
struct Li_support{
	int16_t LastVol;
	float keepvol;
	int16_t Cptime;
	unsigned int time;
	unsigned int keeptime;
	float LastQtity;
	float CurQtity;
	int16_t Ctime;
	int16_t lastctime;
	int16_t downtime;
	int16_t uptime;
	int16_t systime;
	u16 disvol;
};

int16_t Get_vol(void);
float Get_keepvol(float voltage);
float Get_Compensate(int16_t vol);
float Get_Quantity(int16_t voltage);
void Realtime_Qtity(void);
float Get_Qtity(int16_t vol);
extern struct Li_support Lisup;
u16 Get_keepdowm(int16_t vol);
u16 Get_time11(u16 x);

#endif

