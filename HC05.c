#include "HC05.h"
#include <stdio.h>

void HC05_Configuration(void)
{
    // 1. 先把所有要用到的变量定义写在最前面
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    USART_InitTypeDef USART_InitStructure; // <--- 移到这里

    // 2. 然后再写执行逻辑
    CKCUClock.Bit.PA         = 1;
    CKCUClock.Bit.USART0     = 1; 
    CKCUClock.Bit.AFIO       = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    // 接下来使用刚才定义好的结构体，不要再重复定义
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_2, AFIO_FUN_USART_UART);
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_3, AFIO_FUN_USART_UART);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
    USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
    USART_InitStructure.USART_Parity = USART_PARITY_NO;
    USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
    USART_Init(HT_USART0, &USART_InitStructure);

    USART_TxCmd(HT_USART0, ENABLE);
    USART_RxCmd(HT_USART0, ENABLE);
}

// 蓝牙发送字符串
void HC05_SendString(char* str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(HT_USART0, USART_FLAG_TXC) == RESET);
        USART_SendData(HT_USART0, *str++);
    }
}

// 格式化发送温湿度数据
void HC05_SendData(u8 temp, u8 humi)
{
    char buf[32];
    // 格式化为：T:23 C, H:80 % \r\n
    sprintf(buf, "T:%d C, H:%d %%\r\n", temp, humi);
    HC05_SendString(buf);
}
