#ifndef __KEY_M_H
#define __KEY_M_H

#include "ht32.h"

// 矩阵键盘配置
#define MATRIX_ROWS     4    // 行数
#define MATRIX_COLS     4    // 列数

// 按键事件定义
#define KEY_EVENT_NONE      0
#define KEY_EVENT_PRESS     1
#define KEY_EVENT_RELEASE   2
#define KEY_EVENT_LONG_PRESS 3

// 按键状态定义
#define KEY_STATE_IDLE      0
#define KEY_STATE_PRESS     1
#define KEY_STATE_LONG      2
#define KEY_STATE_RELEASE   3

// 扫描参数
#define DEBOUNCE_TIME       2    // 消抖时间（扫描周期数）
#define LONG_PRESS_TIME     50   // 长按时间（50*10ms=500ms）

// 按键编码定义（4x4矩阵）
#define KEY_NONE      0
#define KEY_1         1
#define KEY_2         2
#define KEY_3         3
#define KEY_A         4
#define KEY_4         5
#define KEY_5         6
#define KEY_6         7
#define KEY_B         8
#define KEY_7         9
#define KEY_8         10
#define KEY_9         11
#define KEY_C         12
#define KEY_STAR      13
#define KEY_0         14
#define KEY_HASH      15
#define KEY_D         16

// GPIO引脚定义（使用PB端口）
#define ROW_PORT      HT_GPIOB
#define COL_PORT      HT_GPIOB

// 行线引脚（输出）
#define ROW0_PIN      GPIO_PIN_0
#define ROW1_PIN      GPIO_PIN_1
#define ROW2_PIN      GPIO_PIN_2
#define ROW3_PIN      GPIO_PIN_3

// 列线引脚（输入）
#define COL0_PIN      GPIO_PIN_4
#define COL1_PIN      GPIO_PIN_5
#define COL2_PIN      GPIO_PIN_6
#define COL3_PIN      GPIO_PIN_7

// 按键信息结构体
typedef struct {
    unsigned char row;
    unsigned char col;
    unsigned char code;
    unsigned char state;
    unsigned char press_cnt;
    unsigned char long_cnt;
    unsigned short timestamp;
} Key_Info;

// 按键事件结构体
typedef struct {
    unsigned char key_code;
    unsigned char event;
    unsigned short duration;
} Key_Event;

// 全局变量声明
extern Key_Event key_event;
extern unsigned char matrix_key_pressed;

// 函数声明
void Key_Matrix_Init(void);
void Key_Matrix_Scan(void);
Key_Event Key_Matrix_GetEvent(void);
unsigned char Key_Matrix_IsPressed(unsigned char key_code);
void Key_Matrix_ClearEvent(void);
unsigned short Key_Matrix_GetKeyValue(void);

#endif

