#include "lidischarge.h"
#include "drv_adc1.h"
#include "stdlib.h"

struct Li_support Lisup={0};
uint32_t avercval=0;
uint32_t consumC=0;
uint16_t runflg=0;
void Realtime_Qtity(void)
{
  int16_t CurVoltage;//mv
  uint16_t temp1=0;
  static uint16_t tim[2]={0};
  float idis=0;
	if(runflg<3000)
	{
		runflg+=100;
		return;
	}
  /* ?????????箕 */
  CurVoltage = Get_vol();
	Lisup.RealVol = CurVoltage;
  if(Lisup.LastVol==0)
  {
      Lisup.jitterVol=11;//mv
      Lisup.StartVol = CurVoltage;
      Lisup.EndVol=16940;
      Lisup.Endcolun = 700;
      Lisup.LastVol = CurVoltage;
      Lisup.LastQtity = 100.0;
      Lisup.SlopeQ = 0.469683;
      Lisup.bat=9000;
      Lisup.consumsec=0.04;
      Lisup.coulomb =((Lisup.StartVol-Lisup.EndVol)/Lisup.SlopeQ)+Lisup.Endcolun;//C
      Lisup.tatolcolun=Lisup.coulomb;//?貫C
  }else{
      tim[0]+=20;
      if(tim[0]>=100)
      {
          tim[0]=0;
          Lisup.tatolcolun = Lisup.tatolcolun-Lisup.consumsec;
          Lisup.keepVol = (uint16_t)(((Lisup.tatolcolun - Lisup.Endcolun)*Lisup.SlopeQ)+Lisup.EndVol);
      }
      if(Lisup.keepVol-CurVoltage>=Lisup.jitterVol)
      {
          if(Lisup.keepVol-CurVoltage<=(Lisup.jitterVol*(33)))
          {
              idis = (float)((Lisup.keepVol-CurVoltage)+(59))/460;
          }else{
              idis = (float)((Lisup.keepVol-CurVoltage)/Lisup.jitterVol+(-30))/(3.2);
          }
          Lisup.curentx = (uint32_t)(idis*1000);
          Lisup.consumcolun = ((uint32_t)(20*Lisup.curentx))/1000000.0;
          Lisup.tatolcolun = Lisup.tatolcolun-Lisup.consumcolun;
          Lisup.CurQtity=Lisup.tatolcolun*100/(Lisup.bat-Lisup.Endcolun);
      }else if(CurVoltage-Lisup.keepVol>=Lisup.jitterVol)
      {
          if(CurVoltage-Lisup.keepVol<=(Lisup.jitterVol*(33)))
          {
              idis = (float)((CurVoltage-Lisup.keepVol)+(59))/(460);
          }else{
              idis = (float)((CurVoltage-Lisup.keepVol)/Lisup.jitterVol+(-30))/(3.2);
          }
          Lisup.curentx = (uint32_t)(idis*1000);
          Lisup.consumcolun = ((uint32_t)(20*Lisup.curentx))/1000000.0;
          Lisup.tatolcolun = Lisup.tatolcolun+Lisup.consumcolun;
          Lisup.CurQtity=Lisup.tatolcolun*100/(Lisup.bat-Lisup.Endcolun);
      }
			if(Lisup.tatolcolun<0)
			{
				Lisup.CurQtity=0;
			}
      if(Lisup.CurQtity>100)
      {
          Lisup.CurQtity=100;
      }
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
  }
}

int16_t Get_vol(void) //MV
{
  return ADC1_GetValue_mv(BATTARY_VOLTAGE_CHANNEL)*MV_TO_BATTARY;
}
