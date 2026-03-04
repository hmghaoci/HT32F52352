#ifndef __LCD_H
#define __LCD_H

#include "ht32.h"
#include "delay.h" // 引用你提供的延时头文件

// --- 引脚配置 (可根据实际连线修改) ---
#define LCD_I2C_GPIO      HT_GPIOA
#define LCD_I2C_SCL_PIN   GPIO_PIN_0
#define LCD_I2C_SDA_PIN   GPIO_PIN_1

// --- I2C 地址设置 ---
// 大多数 PCF8574 为 0x4E，如果是 PCF8574A 则为 0x7E
#define LCD_ADDR          0x4E 

// --- 接口控制位 ---
#define RS_BIT   0x01  // P0 - 数据/命令选择
#define RW_BIT   0x02  // P1 - 读/写选择
#define EN_BIT   0x04  // P2 - 使能信号
#define BL_BIT   0x08  // P3 - 背光控制

// --- 函数声明 ---
void LCD_Init(void);
void LCD_ShowString(uint8_t row, uint8_t col, char *str);
void LCD_Clear(void);

#endif
