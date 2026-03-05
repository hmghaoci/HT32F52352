#ifndef _UART_H
#define _UART_H
 
#include "ht32f5xxxx_01.h"
#include <stdio.h>
 
void USART1_Configuration(void);
void Usart_Sendbyte(HT_USART_TypeDef* USARTx, u8 Data);
void Usart_SendArray(HT_USART_TypeDef* USARTx, u8 *array,u8 num);
void Usart_SendStr(HT_USART_TypeDef* USARTx, uint8_t *str);

#endif

