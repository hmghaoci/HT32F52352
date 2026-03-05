
#include "HT32.h"
#include "gptm.h"



sPulseCaptureStructure CaptureCHx;




void gptm_cap_Init(void) 
{	
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  TM_CaptureInitTypeDef  CapInit;

  CKCUClock.Bit.PC         = 1;
  CKCUClock.Bit.AFIO      = 1;
  CKCUClock.Bit.GPTM1     = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  AFIO_GPxConfig(GPIO_PC, GPIO_PIN_13, AFIO_FUN_MCTM_GPTM);
  
  CapInit.Channel = TM_CH_3;//key2  通道3
  CapInit.Polarity = TM_CHP_INVERTED;//低有效
  CapInit.Selection = TM_CHCCS_DIRECT;//直接信号输入
  CapInit.Prescaler = TM_CHPSC_OFF;//每个信号都触发
  CapInit.Filter = 0x0;//无滤波器
  TM_CaptureInit(HT_GPTM1, &CapInit);
  
  TM_IntConfig(HT_GPTM1, TM_INT_CH3CC | TM_INT_UEV, ENABLE);                                                                                      
  TM_Cmd(HT_GPTM1, ENABLE);
  NVIC_EnableIRQ(GPTM1_IRQn);
 
  

}


void CaptureProcess(sPulseCaptureStructure* cap, u16 capture_value, bool isCapBeforeUpdate)
{
  if (cap->ChannelPolarity == TM_CHP_INVERTED)//当前是下降沿触发
  {                        
    if (isCapBeforeUpdate)
    {
      cap->OverflowCounter = 1;
    }
    else
    {
      cap->OverflowCounter = 0;
    }
    cap->StartValue = capture_value;//记录下开始时计数器值
    cap->ChannelPolarity = TM_CHP_NONINVERTED;//下次改为上升沿
  }
  else  //上升沿到后，开始计算下降沿持续多长时间
  {                                      
    if (isCapBeforeUpdate)
      cap->OverflowCounter--;
	//根据溢出次数、起始计数值、和匹配计数值，计算低电平时间
    cap->CapturePulse = (cap->OverflowCounter << 16) + capture_value - cap->StartValue;
	
	//覆盖上次数据，完成标志
    if (cap->DataValid)
      cap->DataOverwrite = TRUE;
    else
      cap->DataValid = TRUE;     
	
    cap->ChannelPolarity = TM_CHP_INVERTED;
  }
}


void GPTM1_IRQHandler(void)
{
  bool update_flag = FALSE;

  //获取状态和计数值
  u32 status = HT_GPTM1->INTSR;
  u32 cnt = HT_GPTM1->CNTR;
	
  //存储和清标志位
  if ((status & TM_INT_UEV) != (HT_GPTM1->INTSR & TM_INT_UEV))
  {
    status = HT_GPTM1->INTSR;
    cnt = HT_GPTM1->CNTR;     //cnt=
  }
  HT_GPTM1->INTSR = ~status;

  //计数器中断
  if (status & TM_INT_UEV)
  {
    update_flag = TRUE;                                                
    if (CaptureCHx.OverflowCounter != 0xFFFF) CaptureCHx.OverflowCounter++;//记录溢出次数
  }
  //捕获中断
  if (status & TM_INT_CH3CC)
  {
    u32 cap_value = TM_GetCaptureCompare1(HT_GPTM1);//获取匹配到的值
    bool isCapBeforeUpdate = (update_flag && (cap_value > cnt))? TRUE : FALSE;//判断本次计数有没有溢出
    CaptureProcess(&CaptureCHx, cap_value, isCapBeforeUpdate);
    TM_ChPolarityConfig(HT_GPTM1, TM_CH_3, CaptureCHx.ChannelPolarity);//设置下次触发条件  上升沿或者下降沿
  }
}

