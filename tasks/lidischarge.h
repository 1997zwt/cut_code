#ifndef APPLICATIONS_DISCHARGE_H_
#define APPLICATIONS_DISCHARGE_H_
#include "stm32f10x.h"

struct Li_support{
      int16_t LastVol;
      int16_t jitterVol;
      int16_t StartVol;
      int16_t EndVol;
      float EndQtity;
      float LastQtity;
      unsigned int time;
      uint32_t curentx;
      float tatolcolun;
      float SlopeQ;
      float consumcolun;
      double coulomb;
      uint16_t bat;
      float consumsec;
      uint16_t Endcolun;
      uint16_t keepVol;
      uint8_t CurQtity;
      uint8_t lastQtity;
			int16_t RealVol;
};

extern struct Li_support Lisup;
extern uint16_t runflg;

#ifndef ABS
#define ABS(x) ((x)<0 ? -1*(x) : (x))
#endif

uint32_t Get_current(uint16_t vol);
int16_t Get_vol(void);
void Realtime_Qtity(void);
#endif
