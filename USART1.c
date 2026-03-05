#include "USART1.h"
 
void USART1_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;//定义串口配置结构体
	CKCU_PeripClockConfig_TypeDef CKCUClock;//定义时钟配置结构体
	
	CKCUClock.Bit.AFIO = 1;//开启AFIO复用
	CKCUClock.Bit.USART1 = 1;//开启串口1
	CKCU_PeripClockConfig(CKCUClock, ENABLE);//使能时钟

	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_4, AFIO_FUN_USART_UART);//复用PA4为串口1
	AFIO_GPxConfig(GPIO_PA, AFIO_PIN_5, AFIO_FUN_USART_UART);//复用PA5为串口1
	
	USART_InitStructure.USART_BaudRate = 115200;//波特率
	USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;//字长
	USART_InitStructure.USART_StopBits = USART_STOPBITS_1;//停止位
	USART_InitStructure.USART_Parity = USART_PARITY_NO;//奇偶校验位
	USART_InitStructure.USART_Mode = USART_MODE_NORMAL;//模式
	USART_Init(HT_USART1  , &USART_InitStructure);//初始化
	
	USART_IntConfig(HT_USART1 , USART_INT_RXDR, ENABLE);//使能USART1接收中断
	
	USART_TxCmd(HT_USART1, ENABLE);//使能USART1发送
	USART_RxCmd(HT_USART1, ENABLE);//使能USART1接
	
	NVIC_EnableIRQ(USART1_IRQn);//使能中断
}

void USART1_IRQHandler(void)
{
	u8 data;
	if( USART_GetFlagStatus(HT_USART1, USART_FLAG_RXDR))//串口1接收中断标志位置1（接收到数据）
	{
		data = USART_ReceiveData(HT_USART1);//接收数据时已经自动清除中断标志位
		if(data == '0')
		{
			GPIO_ClearOutBits(HT_GPIOC, GPIO_PIN_14);//led1点亮
			GPIO_ClearOutBits(HT_GPIOC, GPIO_PIN_15);//led2点亮
			printf("LED ON\n");
		}
		else if(data == '1')
		{
			GPIO_SetOutBits(HT_GPIOC, GPIO_PIN_14);  //led1熄灭
			GPIO_SetOutBits(HT_GPIOC, GPIO_PIN_15);  //led2熄灭
			printf("LED OFF\n");
		}
	}
}

// 发送一个字节
void Usart_Sendbyte(HT_USART_TypeDef* USARTx, u8 Data)
{
	USART_SendData(HT_USART1, Data);
	// 等待发送数据寄存器清空
	while (USART_GetFlagStatus(HT_USART1, USART_FLAG_TXDE) == RESET);		
}
 
//发送每个元素数据长度为8位的数组
void Usart_SendArray(HT_USART_TypeDef* USARTx, u8 *array,u8 num)
{
	u8 i;
	for( i = 0;i < num;i++)
	{
		Usart_Sendbyte(USARTx,*array);
		array++;
	}
}
 
// 发送字符串
void Usart_SendStr(HT_USART_TypeDef* USARTx, uint8_t *str)
{
	uint8_t i;
	for(i = 0;str[i] != '\0';i++)
	{
		Usart_Sendbyte(USARTx,str[i]);
	}
}
 
// 重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口 */
	USART_SendData(HT_USART1, (uint8_t) ch);
		
	/* 等待发送完毕 */
	while (USART_GetFlagStatus(HT_USART1, USART_FLAG_TXDE) == RESET);		
	
	return (ch);
}
 
// 重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
	/* 等待串口输入数据 */
	while (USART_GetFlagStatus(HT_USART1, USART_FLAG_RXDNE) == RESET);
	return (int)USART_ReceiveData(HT_USART1);
}

