#ifndef __ADC_H
#define __ADC_H
#include "ht32.h"

void adc_Init(void);
u16 getadc(u8 ch);
u16 getntctemp(void);
u16 getlight(void);

#endif
