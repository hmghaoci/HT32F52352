#include "ht32.h"


void beep_init(void)
{
	
	//使能PC端口的时钟
	CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
	CKCUClock.Bit.PA         = 1;
	CKCUClock.Bit.AFIO       = 1;
	CKCU_PeripClockConfig(CKCUClock, ENABLE);

	//配置端口功能为GPIO
	AFIO_GPxConfig(GPIO_PA, GPIO_PIN_10, AFIO_FUN_GPIO);

	//配置IO口为输出模式                                                     
	GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_10, GPIO_DIR_OUT);

	//默认输出0
	GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_10, RESET);

}
//设置LED1输出  SET:1  RESET:0
void beep(u8 a)
{
	if (a)
	GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_10, SET);
	else
	GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_10, RESET);	
}
