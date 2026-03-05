#include "LightSensor.h"

void TEMT6000_Init(void)
{
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    
    CKCUClock.Bit.PA    = 1;
    CKCUClock.Bit.ADC0  = 1;
    CKCUClock.Bit.AFIO  = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    CKCU_SetADCnPrescaler(CKCU_ADCPRE_ADC0, CKCU_ADCPRE_DIV64);

    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_1, AFIO_FUN_ADC);

    ADC_SamplingTimeConfig(HT_ADC0, 0x80);

    /* ? 关键增加：配置触发源为软件触发 (ADC_TRIG_SOFTWARE) */
    /* 很多时候不加这一句，ADC_SoftwareStartConvCmd 无法生效 */
    ADC_RegularTrigConfig(HT_ADC0, ADC_TRIG_SOFTWARE);

    ADC_RegularGroupConfig(HT_ADC0, ONE_SHOT_MODE, 1, 0);
    ADC_RegularChannelConfig(HT_ADC0, ADC_CH_1, 0);
    
    ADC_Cmd(HT_ADC0, ENABLE);
}

/**
  * @brief  获取原始值用于调试
  */
uint16_t TEMT6000_GetRawValue(void)
{
    uint32_t timeout = 10000;
    
    ADC_SoftwareStartConvCmd(HT_ADC0, ENABLE);
    
    while (!ADC_GetFlagStatus(HT_ADC0, ADC_FLAG_SINGLE_EOC) && timeout > 0)
    {
        timeout--;
    }
    
    /* 确保读取的是 Rank 0 的数据 */
    return ADC_GetConversionData(HT_ADC0, 0); 
}



/**
  * @brief  获取光照强度百分比
  */
uint8_t TEMT6000_GetPercentage(void)
{
    uint32_t sum = 0;
    uint32_t avg = 0;
    uint32_t percent = 0;
    int i = 0;

    for(i = 0; i < 8; i++)
    {
        sum += TEMT6000_GetRawValue();
    }
    avg = sum / 8;

    /* 百分比计算: (当前值 / 4095) * 100 */
    percent = (avg * 100) / 4095;
    
    if(percent > 100) percent = 100;
    return (uint8_t)percent;
}
