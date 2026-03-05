#include "At24c02.h"

// --- 硬件引脚定义 (PB7=SCL, PB8=SDA) ---
#define I2C_PORT       HT_GPIOB
#define I2C_SCL_PIN    GPIO_PIN_7
#define I2C_SDA_PIN    GPIO_PIN_8

#define SCL_H()        GPIO_WriteOutBits(I2C_PORT, I2C_SCL_PIN, SET)
#define SCL_L()        GPIO_WriteOutBits(I2C_PORT, I2C_SCL_PIN, RESET)
#define SDA_H()        GPIO_WriteOutBits(I2C_PORT, I2C_SDA_PIN, SET)
#define SDA_L()        GPIO_WriteOutBits(I2C_PORT, I2C_SDA_PIN, RESET)
#define SDA_READ()     GPIO_ReadInBit(I2C_PORT, I2C_SDA_PIN)

// 极其稳健的引脚方向切换机制
#define SDA_OUT()      GPIO_DirectionConfig(I2C_PORT, I2C_SDA_PIN, GPIO_DIR_OUT)
#define SDA_IN()       GPIO_DirectionConfig(I2C_PORT, I2C_SDA_PIN, GPIO_DIR_IN)

void I2C_EEPROM_Init(void) {
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    CKCUClock.Bit.PB   = 1;
    CKCUClock.Bit.AFIO = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
    
    AFIO_GPxConfig(GPIO_PB, AFIO_PIN_7, AFIO_FUN_GPIO);
    AFIO_GPxConfig(GPIO_PB, AFIO_PIN_8, AFIO_FUN_GPIO);
    
    GPIO_DirectionConfig(I2C_PORT, I2C_SCL_PIN, GPIO_DIR_OUT);
    GPIO_DirectionConfig(I2C_PORT, I2C_SDA_PIN, GPIO_DIR_OUT);
    
    GPIO_OpenDrainConfig(I2C_PORT, I2C_SCL_PIN, DISABLE);
    GPIO_OpenDrainConfig(I2C_PORT, I2C_SDA_PIN, DISABLE);
    GPIO_PullResistorConfig(I2C_PORT, I2C_SCL_PIN, GPIO_PR_DISABLE);
    GPIO_PullResistorConfig(I2C_PORT, I2C_SDA_PIN, GPIO_PR_DISABLE);

    GPIO_InputConfig(I2C_PORT, I2C_SDA_PIN, ENABLE);
    GPIO_InputConfig(I2C_PORT, I2C_SCL_PIN, ENABLE);

    SCL_H();
    SDA_H();
}

static void I2C_Start(void) {
    SDA_OUT(); SDA_H(); SCL_H(); delay_us(4);
    SDA_L(); delay_us(4); SCL_L();
}

static void I2C_Stop(void) {
    SDA_OUT(); SCL_L(); SDA_L(); delay_us(4);
    SCL_H(); delay_us(4); SDA_H(); delay_us(4);
}

static uint8_t I2C_WaitAck(void) {
    uint8_t timeout = 0;
    SDA_OUT(); SDA_H(); delay_us(2);
    SDA_IN();  delay_us(2); SCL_H(); delay_us(2);
    while (SDA_READ() == SET) {
        timeout++;
        if (timeout > 250) { I2C_Stop(); return 1; }
    }
    SCL_L();
    return 0;
}

static void I2C_Ack(void) {
    SDA_OUT(); SCL_L(); SDA_L(); delay_us(2); SCL_H(); delay_us(2); SCL_L();
}

static void I2C_NAck(void) {
    SDA_OUT(); SCL_L(); SDA_H(); delay_us(2); SCL_H(); delay_us(2); SCL_L();
}

static uint8_t I2C_SendByte(uint8_t byte) {
    uint8_t i;
    SDA_OUT(); SCL_L();
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) SDA_H(); else SDA_L();
        byte <<= 1; delay_us(2); SCL_H(); delay_us(2); SCL_L(); delay_us(2);
    }
    return I2C_WaitAck();
}

static uint8_t I2C_ReadByte(uint8_t ack) {
    uint8_t i, receive = 0;
    SDA_IN(); 
    for (i = 0; i < 8; i++) {
        SCL_L(); delay_us(2); SCL_H(); receive <<= 1;
        if (SDA_READ() == SET) receive++; 
        delay_us(2);
    }
    if (!ack) I2C_NAck(); else I2C_Ack();
    return receive;
}

// ---------------- 外部调用接口 ----------------

// 写入一个字节 (注：执行此函数系统将微小阻塞 5ms 物理烧写时间)
void AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data) {
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_SendByte(WordAddress);
    I2C_SendByte(Data);
    I2C_Stop();
    
    // 强制延时 5ms 供芯片内部电荷泵烧写 Flash，不可省略！
    delay_ms(5); 
}

// 读取一个字节 (耗时 < 1ms，极快)
uint8_t AT24C02_ReadByte(uint8_t WordAddress) {
    uint8_t temp = 0;
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_SendByte(WordAddress);
    
    I2C_Start(); 
    I2C_SendByte(EEPROM_ADDR_READ);
    temp = I2C_ReadByte(0); 
    I2C_Stop();
    return temp;
}
