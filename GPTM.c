#include "GPTM.h"
void GPTM0_Configuration(void)
{  
 
  TM_TimeBaseInitTypeDef TM_TimeBaseInitStruct; // 声明结构体
  TM_OutputInitTypeDef TM_OutputInitStruct;
  
  CKCU_PeripClockConfig_TypeDef CKCUClock;
  CKCUClock.Bit.GPTM0      = 1;
  CKCUClock.Bit.AFIO       = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE); // 时钟使能  
	
  AFIO_GPxConfig(GPIO_PC, AFIO_PIN_5, AFIO_MODE_4);  //PC5开启复用功能 AFIO_FUN_PWM

  GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_5, GPIO_DIR_OUT); // 输入输出方向 
  
  // 定时器时基以及计数方式初始化   Prescaler*CounterReload/48000000=0.02s=20ms 0.5ms对应0度，2.5ms对应180度
  TM_TimeBaseInitStruct.Prescaler = 4800-1; // 预分频系数                         ==>(0.5/20)*200=5
  TM_TimeBaseInitStruct.CounterReload = 200-1; // 计数周期                    ==>(2.5/20)*200=25
  TM_TimeBaseInitStruct.RepetitionCounter = 0;
  TM_TimeBaseInitStruct.CounterMode = TM_CNT_MODE_UP; // 计数模式
  TM_TimeBaseInitStruct.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;// 立即重装载  
  TM_TimeBaseInit(HT_GPTM0, &TM_TimeBaseInitStruct);  
 
  // 通道及输出模式初始化
  TM_OutputInitStruct.Channel = TM_CH_1; //选择通道
  TM_OutputInitStruct.OutputMode = TM_OM_PWM1; //PWM模式
  TM_OutputInitStruct.Control = TM_CHCTL_ENABLE; // GPTM通道使能  
	TM_OutputInitStruct.Polarity = TM_CHP_NONINVERTED; // 通道极性，@arg TM_CHP_INVERTED是低电平或下降沿  @arg TM_CHP_NONINVERTED是上升沿
  TM_OutputInitStruct.Compare = 0;
  TM_OutputInit(HT_GPTM0, &TM_OutputInitStruct);
  
  TM_Cmd(HT_GPTM0, ENABLE); //开启GPTM0
}


