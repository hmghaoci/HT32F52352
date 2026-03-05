#include "Task.h"
#include "SysTick.h"
#include "USART1.h"
#include "oled.h"
#include "DHT11.h"
#include "HC05.h"
#include <stdio.h>

// 静态局部变量，仅限本文件使用
static u8  s_temp = 0, s_humi = 0;
static u32 s_last_dht_time = 0;
static u32 s_last_oled_time = 0;

/**
  * @brief 初始化所有外设
  */
void Task_Init(void)
{
    SysTick_Configuration();
    USART1_Configuration();
    HC05_Configuration();
    OLED_Configuration();
    DHT11_Init();

    OLED_Clear(0);
    OLED_ShowString(0, 0, (u8*)"System Running", 16);
    printf("Task Engine Started...\r\n");
}

/**
  * @brief DHT11 采集任务 (每 2000ms)
  */
static void Task_DHT11_Update(void)
{
    if (tick_ct - s_last_dht_time >= 2000)
    {
        s_last_dht_time = tick_ct;
        if (DHT11_Read_Data(&s_temp, &s_humi) == 0)
        {
            printf("[Log] T:%d H:%d\r\n", s_temp, s_humi);
            HC05_SendData(s_temp, s_humi); // 发送给蓝牙
        }
    }
}

/**
  * @brief OLED 刷新任务 (每 500ms)
  */
static void Task_OLED_Refresh(void)
{
    if (tick_ct - s_last_oled_time >= 500)
    {
        s_last_oled_time = tick_ct;
        OLED_ShowNum(48, 2, s_temp, 2, 16);
        OLED_ShowNum(48, 4, s_humi, 2, 16);
    }
}

/**
  * @brief 蓝牙指令处理任务 (实时轮询)
  */
static void Task_Bluetooth_Handler(void)
{
    if (USART_GetFlagStatus(HT_USART0, USART_FLAG_RXDR) == SET)
    {
        u8 cmd = USART_ReceiveData(HT_USART0);
        if (cmd == '1') {
            GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_14, RESET); // 开灯
            HC05_SendString("ACK: LED ON\r\n");
        }
        else if (cmd == '0') {
            GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_14, SET);   // 关灯
            HC05_SendString("ACK: LED OFF\r\n");
        }
    }
}

/**
  * @brief 总任务调度器
  */
void Task_Process(void)
{
    Task_DHT11_Update();     // 处理温湿度
    Task_OLED_Refresh();     // 处理显示
    Task_Bluetooth_Handler(); // 处理蓝牙
    // ... 你可以在这里无限增加新任务
}
