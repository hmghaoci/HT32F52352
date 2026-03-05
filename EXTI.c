#include "EXTI.h"

int flag=0; 
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStruct;
  CKCU_PeripClockConfig_TypeDef CKCUClock;
  CKCUClock.Bit.EXTI    = 1;
  CKCUClock.Bit.AFIO    = 1;
  CKCUClock.Bit.PC      = 1;   
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
 
  AFIO_GPxConfig(GPIO_PC,AFIO_PIN_10,AFIO_FUN_GPIO); //复用PC10为GPIO
  AFIO_EXTISourceConfig(AFIO_EXTI_CH_10,AFIO_ESS_PC); //复用中断源选择通道10
  GPIO_InputConfig(HT_GPIOC,GPIO_PIN_10,ENABLE);    // 此函数可实现GPIO口变为输入模式，上拉电阻，默认电流。
	GPIO_PullResistorConfig(HT_GPIOC,GPIO_PIN_10,GPIO_PR_UP);//下降沿--上拉电阻

  EXTI_InitStruct.EXTI_Debounce=EXTI_DEBOUNCE_ENABLE; //开启去抖
  EXTI_InitStruct.EXTI_DebounceCnt=65535;//去抖计数
  EXTI_InitStruct.EXTI_Channel=AFIO_EXTI_CH_10;//复用中断源选择通道10
  EXTI_InitStruct.EXTI_IntType=EXTI_NEGATIVE_EDGE; //下降沿触发
  EXTI_Init(&EXTI_InitStruct);
  
  EXTI_IntConfig(AFIO_EXTI_CH_10,ENABLE);//检测中断源状态函数
  NVIC_EnableIRQ(EXTI4_15_IRQn);//使能中断
}
 
//---------- 外部中断函数-----------
void EXTI4_15_IRQHandler(void)
{
  if (EXTI_GetEdgeStatus(EXTI_CHANNEL_10, EXTI_EDGE_NEGATIVE))//通道10中断标志位置1
  {
      EXTI_ClearEdgeFlag(EXTI_CHANNEL_10);//清除中断标志位
		  if(flag==0)
			{
				GPIO_ClearOutBits(HT_GPIOC, GPIO_PIN_14);//led1点亮
		 		GPIO_ClearOutBits(HT_GPIOC, GPIO_PIN_15);//led2点亮
				flag=!flag;
			}	
			else if(flag==1)
     {
			  GPIO_SetOutBits(HT_GPIOC, GPIO_PIN_14);  //led1熄灭
		 		GPIO_SetOutBits(HT_GPIOC, GPIO_PIN_15);  //led2熄灭
			  flag=!flag;
		 }
  }
 
}
