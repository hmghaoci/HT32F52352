#ifndef __GPTM_H
#define __GPTM_H
#include "ht32.h"

typedef struct
{
  u32 OverflowCounter;
  u32 StartValue;
  u32 CapturePulse;
  TM_CHP_Enum ChannelPolarity;
  bool DataValid;
  bool DataOverwrite;
} sPulseCaptureStructure;

void gptm_cap_Init(void);

#endif
