#ifndef __DHT11_H
#define __DHT11_H

#include "ht32.h"
#include "SysTick.h"

// 定义 DHT11 连接的引脚：PA0
#define DHT11_GPIO_PORT    HT_GPIOA
#define DHT11_GPIO_PIN     GPIO_PIN_0
#define DHT11_GPIO_ID      GPIO_PA
#define DHT11_AFIO_PIN     AFIO_PIN_0

// 函数声明
void DHT11_Init(void);
int DHT11_Read_Data(u8 *temp, u8 *humi); // 返回 0 表示成功，-1 表示失败
void DHT11_Task(void);                  // 放入 main 循环的非阻塞任务

#endif
