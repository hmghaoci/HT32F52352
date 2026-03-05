#ifndef __HC05_H
#define __HC05_H

#include "ht32.h"

void HC05_Configuration(void);
void HC05_SendString(char* str);
void HC05_SendData(u8 temp, u8 humi);

#endif
