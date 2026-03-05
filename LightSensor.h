#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include "ht32.h"

void TEMT6000_Init(void);
uint16_t TEMT6000_GetRawValue(void);
uint8_t TEMT6000_GetPercentage(void);

#endif
