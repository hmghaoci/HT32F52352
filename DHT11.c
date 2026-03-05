#include "DHT11.h"

// 配置引脚为输出模式
void DHT11_Mode_Out(void)
{
    GPIO_DirectionConfig(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_DIR_OUT);
}

// 配置引脚为输入模式
void DHT11_Mode_In(void)
{
    GPIO_DirectionConfig(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_DIR_IN);
    GPIO_InputConfig(DHT11_GPIO_PORT, DHT11_GPIO_PIN, ENABLE);
}

// 初始化
void DHT11_Init(void)
{
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    CKCUClock.Bit.PA = 1; // 开启 GPIOA 时钟
    CKCUClock.Bit.AFIO = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
    
    AFIO_GPxConfig(DHT11_GPIO_ID, DHT11_AFIO_PIN, AFIO_FUN_GPIO);
    
    DHT11_Mode_Out();
    GPIO_WriteOutBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN, SET); // 初始化高电平
}

// 读取一个位
static u8 DHT11_Read_Bit(void)
{
    u8 retry = 0;
    while(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == SET && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    retry = 0;
    while(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == RESET && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    delay_us(40); // 等待 40us
    if(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == SET) return 1;
    else return 0;
}

// 读取一个字节
static u8 DHT11_Read_Byte(void)
{
    u8 i, dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

// 读取温湿度
int DHT11_Read_Data(u8 *temp, u8 *humi)
{
    u8 buf[5];
    u8 i;
    u8 retry = 0;

    // 1. 主机发送启动信号
    DHT11_Mode_Out();
    GPIO_WriteOutBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN, RESET);
    delay_ms(18); // 至少 18ms
    GPIO_WriteOutBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN, SET);
    delay_us(30); // 20-40us

    // 2. 切换输入，等待响应
    DHT11_Mode_In();
    if(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == RESET)
    {
        while(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == RESET && retry < 100) 
        {
            retry++; delay_us(1);
        }
        retry = 0;
        while(GPIO_ReadInBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == SET && retry < 100) 
        {
            retry++; delay_us(1);
        }
        // 3. 读取 40 位数据
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }
        // 4. 校验
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];
            *temp = buf[2];
            return 0; // 成功
        }
    }
    return -1; // 失败
}
