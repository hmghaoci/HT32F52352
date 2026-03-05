#ifndef __WDT_H
#define __WDGT_H
#include "ht32.h"

void wdt_Init(u16 prer,u16 rlr);
void WDT_Feed(void);

#endif
