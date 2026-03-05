#include "hx711.h"

u32 Weight_Maopi = 0;
s32 Weight_Shiwu = 0;
u8  Data_Ready_Flag = 0;

#define GapValue 370.62 // 你之前算出的校准系数

void HX711_Init(void) {
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    CKCUClock.Bit.PA = 1; 
    CKCUClock.Bit.PB = 1; 
    CKCUClock.Bit.AFIO = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    AFIO_GPxConfig(GPIO_PB, AFIO_PIN_4, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(HX711_SCK_GPIO_ID, HX711_SCK_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(GPIO_PA, AFIO_PIN_2, AFIO_MODE_DEFAULT);
    GPIO_DirectionConfig(HX711_DOUT_GPIO_ID, HX711_DOUT_PIN, GPIO_DIR_IN);
    GPIO_InputConfig(HX711_DOUT_GPIO_ID, HX711_DOUT_PIN, ENABLE);
    GPIO_PullResistorConfig(HX711_DOUT_GPIO_ID, HX711_DOUT_PIN, GPIO_PR_UP);
    
    HX711_SCK_L(); 
}

// 【非阻塞】检查数据是否转换完成
u8 HX711_IsReady(void) {
    // DOUT 为低代表转换完成
    return (HX711_DOUT_READ() == 0);
}

// 【快速读取】不再包含 while 等待循环
u32 HX711_Read_Raw(void) {
    u32 count = 0;
    u8 i;

    // 进入此函数前，DOUT 必须已经为低
    for(i = 0; i < 24; i++) {
        HX711_SCK_H();
        delay_us(1); // 这里的 1us 阻塞可以忽略不计（总计约 50us）
        count = count << 1;
        HX711_SCK_L();
        if(HX711_DOUT_READ()) count++;
        delay_us(1);
    }

    HX711_SCK_H();
    delay_us(1);
    HX711_SCK_L();
    delay_us(1);

    return count ^ 0x800000;
}

// 封装一个逻辑处理函数，放在 main 的 while(1) 中
void HX711_Process(void) {
    if (HX711_IsReady()) {
        u32 raw_data = HX711_Read_Raw();
        Weight_Shiwu = (s32)raw_data - (s32)Weight_Maopi;
        Weight_Shiwu = (s32)((float)Weight_Shiwu / GapValue);
        
        if (Weight_Shiwu < 0) Weight_Shiwu = 0;
        
        Data_Ready_Flag = 1; // 置标志位，通知主程序有新数据了
    }
}

void Get_Maopi(void) {
    // 去皮通常在初始化执行，可以保留阻塞读取以确保拿到初值
    u32 timeout = 0;
    while(HX711_DOUT_READ() && timeout < 1000000) timeout++;
    Weight_Maopi = HX711_Read_Raw();
}
