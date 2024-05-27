#include "lidischarge.h"
#include "drv_adc1.h"
#include "stdlib.h"
#include "stdio.h"

struct li_battery Libat1={0};
struct li_battery Libat2={0};
struct li_battery Libat3={0};
struct Li_support Lisup={0};
int16_t maxvol;
int16_t minvol=0;
u8 flg=0;
u8 downflg=0;
u8 upflg=0;
u8 keepflg=0;
u8 Ifsys=0;
//u16 lasttime;
//u16 curtime;
void Realtime_Qtity(void)
{
	int16_t CurVoltage;//mv	
//	static int qtime = 0;
//	static int qkptime=0;

	/* 获取电池放电当前电压 */
	CurVoltage = Get_vol();
	if(Lisup.LastVol==0)	//模型参数赋值 这里参数为举例
	{
		Libat1.jitterVol=11;//mv
		Libat1.StartVol = CurVoltage;
		Libat1.SlopeVol = 32.258;  //20秒降1个11
		Libat1.SlopeQ = 32;		
		Libat1.EndVol=17000;
		Libat1.EndQtity = 0;						
		
		Libat1.DownRang = 11;
		
		// Libat2.jitterVol=11;//mv
		// Libat2.SlopeVol = 16.474;  //40秒降1个11
		// Libat2.SlopeQ = 400;		
		// Libat2.EndVol=15000;
		// Libat2.EndQtity = 10;		

		// Libat3.jitterVol=11;//mv
		// Libat3.SlopeVol = 265.3;  //2.5秒降1个11
		// Libat3.SlopeQ =1000;		
		// Libat3.EndVol=5000;
		// Libat3.EndQtity = 0;		
		
		Lisup.LastVol = CurVoltage;		
		Lisup.LastQtity = 100.0;
		Lisup.keepvol = CurVoltage;
		maxvol = Libat1.DownRang+3;
			
	}else{
		Lisup.time++;
		/* 电压滤杂波 */
		if(CurVoltage-Lisup.LastVol>0 && CurVoltage-Lisup.LastVol<=Libat1.jitterVol)//回流
		{
			Lisup.LastVol = CurVoltage;
			return;
		}
		if(CurVoltage-Lisup.LastVol<0&&Lisup.LastVol-CurVoltage<=Libat1.jitterVol)
		{
			Lisup.LastVol = CurVoltage;
			return;
		}
		if(downflg==0&&Lisup.keepvol-CurVoltage>maxvol)
		{
			if(downflg==0)//两秒认为负载启动完毕
			{
				Lisup.downtime++;
			}
			if(Lisup.downtime>=20)
			{
				Lisup.downtime = 0;
				downflg = 1;				//为1代表下降到最底部，并以此电流开始电压的一直下降 downflg清0？
				Lisup.disvol = Lisup.keepvol - CurVoltage;				
				upflg=0;
			}
			if(Ifsys==0&&Lisup.downtime==10)
			{
				Lisup.downtime=0;
				Lisup.keepvol = CurVoltage;
				Ifsys=1;
			}
		}
		if(downflg==1&&Lisup.keepvol - CurVoltage<=11)
		{
			if(upflg==0)
			{
				Lisup.uptime++;
			}
			if(Lisup.uptime>=10)
			{
				Lisup.uptime=0;
				upflg=1;
				downflg=0;
				Lisup.disvol = 0;
			}
		}
		

		Lisup.CurQtity = Get_Qtity(CurVoltage);
				
		if(Lisup.CurQtity>Lisup.LastQtity)
		{	
			Lisup.CurQtity = Lisup.LastQtity;				
		}else{
			if(Lisup.LastQtity==100)
			{
				Lisup.LastQtity = Lisup.CurQtity;
			}
			if(Lisup.LastQtity-Lisup.CurQtity>=5)
			{
				Lisup.CurQtity = Lisup.LastQtity;
			}
		}
		Lisup.LastQtity = Lisup.CurQtity;
		Lisup.LastVol = CurVoltage;
	}
	
}

int16_t Get_vol(void)
{
	return ADC1_GetValue_mv(BATTARY_VOLTAGE_CHANNEL)*MV_TO_BATTARY;
}


//
float Get_RealVol(int16_t vol)
{
	float Truvol=0;
	static int Savedisdown=0;
	int lastdis;
	
	lastdis = Savedisdown;
	Savedisdown = Lisup.keepvol-vol;
	/* keepvol更新 */
	if(downflg==1&&upflg==0)
	{
		Lisup.Ctime++;
		if(Lisup.LastVol-vol>22)
		{
			Lisup.disvol = Lisup.keepvol -vol;
		}
		if(Lisup.Ctime>=Get_time11(Lisup.disvol))
		{
			Lisup.keepvol -= 11;
			Lisup.Ctime = 0;	
			Lisup.disvol = Lisup.keepvol -vol;
			if(Lisup.keepvol -vol<=44)
			{
				Lisup.disvol=11;
			}else if(Lisup.keepvol -vol>44&&Lisup.keepvol -vol<=88)
			{
				Lisup.disvol=22;
			}
		}
		
		
	}
	
	if((Lisup.LastVol - vol > maxvol)||(Savedisdown>lastdis))//下降中
	{
		
		Lisup.keeptime=0;
		Lisup.systime=0;
		Truvol = Lisup.keepvol;
	}
//	else if(Lisup.keepvol-vol>11&&upflg==1)//上升，
//	{
//		if(vol-Lisup.LastVol>11)
//		{
//			
//		}
//	}
	else if(Lisup.keepvol-vol<=Libat1.DownRang)//回到KEEPVOL
	{
		Lisup.keeptime++;
		if(Lisup.keeptime>=40)
		{
		Truvol = vol;
		Lisup.keepvol = vol;
		Lisup.keeptime=0;
		Lisup.systime=0;
		}
	}
	if(Truvol==0)
	{
		Truvol = Lisup.keepvol;
	}
	return Truvol;
}

float Get_Qtity(int16_t vol)
{
	float TempQuantity;
	float voltrue;
	
	if(Lisup.time>=600&&downflg==0)//每分减少量
	{			
		/* 分阶段减少 */
		if(Lisup.keepvol>=Libat1.EndVol)
		{
			Lisup.keepvol = Lisup.keepvol - Libat1.SlopeVol;//每减少量
		}
		// else if(Lisup.keepvol>Libat2.EndVol&&Lisup.keepvol<Libat1.EndVol)
		// {
			// Lisup.keepvol = Lisup.keepvol - Libat2.SlopeVol;
		// }else if(Lisup.keepvol>Libat3.EndVol&&Lisup.keepvol<Libat2.EndVol)
		// {
			// Lisup.keepvol = Lisup.keepvol - Libat3.SlopeVol;
		// }
		
		Lisup.time = 0;		
	}else if(downflg==1)
	{
		Lisup.time--;
	}
	voltrue = Get_RealVol(vol);
	if(voltrue>=Libat1.EndVol)
	{
		TempQuantity = (voltrue-Libat1.EndVol)/(float)Libat1.SlopeQ+Libat1.EndQtity;
	}
	// else if(voltrue>Libat2.EndVol&&voltrue<Libat1.EndVol)
	// {
		// TempQuantity = (voltrue-Libat2.EndVol)/(float)Libat2.SlopeQ+Libat2.EndQtity;
		// if(TempQuantity>Libat1.EndQtity)
		// {
			// TempQuantity = Libat1.EndQtity;
		// }
	// }
	// else if(voltrue>Libat3.EndVol&&voltrue<Libat2.EndVol)
	// {
		// TempQuantity = (voltrue-Libat3.EndVol)/(float)Libat3.SlopeQ+Libat3.EndQtity;
		// if(TempQuantity>Libat2.EndQtity)
		// {
			// TempQuantity = Libat2.EndQtity;
		// }
	// }
	
	
	if(TempQuantity>=100){
		TempQuantity = 100;
	}else if(TempQuantity<=1)
	{
		TempQuantity =1;
	}
	return TempQuantity;
}


u16 Get_time11(u16 x)
{
	float y;
	y = 20.545-0.045*x;
	if(y<=3)
	{
		y=3;
	}	
	return (u16)(y*10);
}
