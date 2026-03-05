#include "GPIO.h"

//-----------------------------------------------------------------------------
void GPIO_Configuration(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock;//定义时钟配置结构体
  CKCUClock.Bit.PC    = 1;//开启PC
  CKCU_PeripClockConfig(CKCUClock, ENABLE);//使能时钟

  GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_14, GPIO_DIR_OUT);//配置PC14为输出模式
  GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_15, GPIO_DIR_OUT);//配置PC15为输出模式
  
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_14,SET); //将LED1熄灭	
	GPIO_WriteOutBits(HT_GPIOC, GPIO_PIN_15,SET); //将LED2熄灭	
}

//-----------------------------------------------------------------------------


