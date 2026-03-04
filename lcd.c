#include "lcd.h"

// --- 内部底层宏：控制引脚电平 ---
#define SCL_H    GPIO_WriteOutBits(LCD_I2C_GPIO, LCD_I2C_SCL_PIN, SET)
#define SCL_L    GPIO_WriteOutBits(LCD_I2C_GPIO, LCD_I2C_SCL_PIN, RESET)
#define SDA_H    GPIO_WriteOutBits(LCD_I2C_GPIO, LCD_I2C_SDA_PIN, SET)
#define SDA_L    GPIO_WriteOutBits(LCD_I2C_GPIO, LCD_I2C_SDA_PIN, RESET)

// --- 模拟 I2C 协议 ---
static void I2C_Start(void) {
    SDA_H; SCL_H; delay_us(5);
    SDA_L; delay_us(5);
    SCL_L;
}

static void I2C_Stop(void) {
    SDA_L; SCL_H; delay_us(5);
    SDA_H; delay_us(5);
}

static void I2C_SendByte(uint8_t dat) {
    uint8_t i;
    for(i=0; i<8; i++) {
        if(dat & 0x80) SDA_H; else SDA_L;
        delay_us(5); SCL_H; delay_us(5); SCL_L;
        dat <<= 1;
    }
    SDA_H; SCL_H; delay_us(5); SCL_L; // 等待 ACK（简单处理）
}

// --- LCD 核心写入 ---
static void LCD_WriteI2C(uint8_t data) {
    I2C_Start();
    I2C_SendByte(LCD_ADDR);
    I2C_SendByte(data | BL_BIT); // 默认开启背光
    I2C_Stop();
}

static void LCD_PulseEnable(uint8_t data) {
    LCD_WriteI2C(data | EN_BIT);
    delay_us(5);
    LCD_WriteI2C(data & ~EN_BIT);
    delay_us(40);
}

static void LCD_RawWrite(uint8_t val, uint8_t mode) {
    uint8_t high = (val & 0xF0) | mode;
    uint8_t low = ((val << 4) & 0xF0) | mode;
    LCD_PulseEnable(high);
    LCD_PulseEnable(low);
}

// --- API 函数 ---
void LCD_Init(void) {
    // 1. 初始化 GPIO 时钟
    CKCU_PeripClockConfig_TypeDef ckcu_clk = {{0}};
    ckcu_clk.Bit.PA = 1;
    CKCU_PeripClockConfig(ckcu_clk, ENABLE);
    
    // 2. 配置引脚为推挽输出
    GPIO_DirectionConfig(LCD_I2C_GPIO, LCD_I2C_SCL_PIN | LCD_I2C_SDA_PIN, GPIO_DIR_OUT);
    
    // 3. LCD 初始化序列 (4位总线模式)
    delay_ms(50);
    LCD_PulseEnable(0x30); delay_ms(5);
    LCD_PulseEnable(0x30); delay_us(200);
    LCD_PulseEnable(0x30);
    LCD_PulseEnable(0x20); 
    
    LCD_RawWrite(0x28, 0); // 1602设置：2行显示，5x8点阵
    LCD_RawWrite(0x0C, 0); // 显示开，不显示光标
    LCD_RawWrite(0x01, 0); // 清屏
    delay_ms(5);
}

void LCD_ShowString(uint8_t row, uint8_t col, char *str) {
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_RawWrite(addr, 0);
    while(*str) {
        LCD_RawWrite(*str++, RS_BIT);
    }
}

void LCD_Clear(void) {
    LCD_RawWrite(0x01, 0);
    delay_ms(2);
}
