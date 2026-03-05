
#include "HT32.h"
#include "led.h"

u8 f1s=0;

void bftm_Init(u32 nus) 
{	
  
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.BFTM0      = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  

  NVIC_EnableIRQ(BFTM0_IRQn);
                                                 
  BFTM_SetCompare(HT_BFTM0, SystemCoreClock/1000000*nus);
  BFTM_SetCounter(HT_BFTM0, 0);
  BFTM_IntConfig(HT_BFTM0, ENABLE);
  BFTM_EnaCmd(HT_BFTM0, ENABLE);

}



//bftmÖÐ¶Ïº¯Êý
void BFTM0_IRQHandler(void)
{

	f1s=1;
	BFTM_ClearFlag(HT_BFTM0);
}

