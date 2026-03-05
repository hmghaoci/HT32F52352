#ifndef __HX711_H
#define __HX711_H

#include "ht32.h"
#include "SysTick.h"

// --- 引脚定义 ---
#define HX711_SCK_GPIO_ID    HT_GPIOB
#define HX711_SCK_PIN        GPIO_PIN_4
#define HX711_DOUT_GPIO_ID   HT_GPIOA
#define HX711_DOUT_PIN       GPIO_PIN_2

// --- 操作宏 ---
#define HX711_SCK_H()       GPIO_WriteOutBits(HX711_SCK_GPIO_ID, HX711_SCK_PIN, SET)
#define HX711_SCK_L()       GPIO_WriteOutBits(HX711_SCK_GPIO_ID, HX711_SCK_PIN, RESET)
#define HX711_DOUT_READ()   GPIO_ReadInBit(HX711_DOUT_GPIO_ID, HX711_DOUT_PIN)

// --- 外部变量 ---
extern u32 Weight_Maopi;
extern s32 Weight_Shiwu;
extern u8  Data_Ready_Flag; // 数据就绪标志位

// --- 函数声明 ---
void HX711_Init(void);
u8   HX711_IsReady(void);   // 非阻塞检查
u32  HX711_Read_Raw(void);  // 快速读取
void Get_Maopi(void);
void HX711_Process(void);   // 建议在主循环调用的处理函数

#endif
