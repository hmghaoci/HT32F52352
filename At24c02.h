#ifndef __AT24C02_H
#define __AT24C02_H

#include "ht32.h"
#include "ht32f5xxxx_gpio.h"
#include "SysTick.h"

// I2C 设备硬件地址
#define EEPROM_ADDR_WRITE 0xA0
#define EEPROM_ADDR_READ  0xA1

void I2C_EEPROM_Init(void);
void AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t AT24C02_ReadByte(uint8_t WordAddress);

#endif
