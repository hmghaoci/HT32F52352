
#include "HT32.h"


const u16 temptab[]={  //0 to 100? step 1
3157,3118,3079,3038,2998,2956,2914,2871,2827,2783,
2739,2694,2648,2603,2557,2511,2464,2418,2371,2325,
2278,2232,2186,2140,2094,2048,2003,1958,1913,1869,
1825,1782,1739,1697,1655,1614,1574,1534,1495,1457,
1419,1382,1346,1310,1275,1241,1208,1175,1143,1112,
1082,1052,1023,995,967,940,914,888,863,839,
816,793,770,749,728,707,687,668,649,631,
613,596,579,563,547,532,517,502,488,475,
462,449,436,424,413,401,390,380,369,359,
350,340,331,322,314,305,297,289,282,274,
267,
};


void adc_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
  CKCUClock.Bit.PC         = 1;																										//??GPIOA??
  CKCUClock.Bit.AFIO       = 1;																										//??????
  CKCUClock.Bit.ADC        = 1;																										//??ADC??
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
	
	CKCU_SetADCPrescaler(CKCU_ADCPRE_DIV8);																				
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_4, AFIO_FUN_ADC);																
	ADC_RegularTrigConfig(HT_ADC, ADC_TRIG_SOFTWARE);															
	ADC_RegularGroupConfig(HT_ADC, ONE_SHOT_MODE, 1, 1);														
	ADC_RegularChannelConfig(HT_ADC, ADC_CH_8, 0);																
	
	ADC_Cmd(HT_ADC,ENABLE);
}



u16 getadc(u8 ch)
{
	int sum=0;
	int i;
	ADC_RegularChannelConfig(HT_ADC, ch, 0);	
    for(i=0;i<8;i++)	
	{
		ADC_SoftwareStartConvCmd(HT_ADC,ENABLE);					
		while(ADC_GetFlagStatus(HT_ADC,ADC_FLAG_SINGLE_EOC) != SET);									
		sum += ADC_GetConversionData(HT_ADC,0);
	}		
	return sum>>3;
}


u16 getntctemp(void)
{
	u8 i;
	u16 temp;
	temp=getadc(8);
	if(temp<267)
	{
		return 99;	
	}
	else if(temp>3157)
	{
		return 0;
	}
	else
	{
		for(i=0;i<100;i++)
		{
			if(temptab[i]<temp)	break;
		}
		return i;
	}
}


u16 getlight()
{
	int temp;
	temp=getadc(8);
	return (u16)((4096-temp)*100/4096);
}


