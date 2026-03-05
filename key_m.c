#include "ht32.h"
#include "key_m.h"

// 全局变量定义
Key_Event key_event = {KEY_NONE, KEY_EVENT_NONE, 0};
unsigned char matrix_key_pressed = 0;

// 静态变量
static Key_Info key_matrix[MATRIX_ROWS][MATRIX_COLS];
static unsigned short scan_counter = 0;

// 按键映射表（使用unsigned char类型）
static const unsigned char key_map[MATRIX_ROWS][MATRIX_COLS] = {
    {KEY_1, KEY_2, KEY_3, KEY_A},
    {KEY_4, KEY_5, KEY_6, KEY_B},
    {KEY_7, KEY_8, KEY_9, KEY_C},
    {KEY_STAR, KEY_0, KEY_HASH, KEY_D}
};

// 行线引脚数组
static const unsigned short row_pins[MATRIX_ROWS] = {
    ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN
};

// 列线引脚数组
static const unsigned short col_pins[MATRIX_COLS] = {
    COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN
};

/**
  * @brief  初始化矩阵按键
  */
void Key_Matrix_Init(void)
{
    // 提前声明所有局部变量
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    unsigned char i;
    unsigned char row, col;
    
    // 使能PB端口和AFIO时钟
    CKCUClock.Bit.PB = 1;
    CKCUClock.Bit.AFIO = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
    
    // 初始化行线为推挽输出
    for (i = 0; i < MATRIX_ROWS; i++) {
        AFIO_GPxConfig(GPIO_PB, row_pins[i], AFIO_FUN_GPIO);
        GPIO_DirectionConfig(ROW_PORT, row_pins[i], GPIO_DIR_OUT);
        GPIO_WriteOutBits(ROW_PORT, row_pins[i], SET);
    }
    
    // 初始化列线为上拉输入
    for (i = 0; i < MATRIX_COLS; i++) {
        AFIO_GPxConfig(GPIO_PB, col_pins[i], AFIO_FUN_GPIO);
        GPIO_DirectionConfig(COL_PORT, col_pins[i], GPIO_DIR_IN);
        GPIO_InputConfig(COL_PORT, col_pins[i], ENABLE);
        GPIO_PullResistorConfig(COL_PORT, col_pins[i], GPIO_PR_UP);
    }
    
    // 初始化按键状态数组
    for (row = 0; row < MATRIX_ROWS; row++) {
        for (col = 0; col < MATRIX_COLS; col++) {
            key_matrix[row][col].row = row;
            key_matrix[row][col].col = col;
            key_matrix[row][col].code = key_map[row][col];
            key_matrix[row][col].state = KEY_STATE_IDLE;
            key_matrix[row][col].press_cnt = 0;
            key_matrix[row][col].long_cnt = 0;
            key_matrix[row][col].timestamp = 0;
        }
    }
    
    // 初始化事件
    key_event.key_code = KEY_NONE;
    key_event.event = KEY_EVENT_NONE;
    key_event.duration = 0;
    
    scan_counter = 0;
    matrix_key_pressed = 0;
}

/**
  * @brief  扫描矩阵按键（核心扫描函数）
  * @return 当前按下的按键值（位图表示）
  */
unsigned short Key_Matrix_GetKeyValue(void)
{
    // 提前声明所有局部变量
    unsigned short key_value = 0;
    unsigned char row;
    volatile unsigned short i;
    unsigned char col0, col1, col2, col3;
    
    // 遍历所有行
    for (row = 0; row < MATRIX_ROWS; row++) {
        // 设置当前行为低电平，其他行为高电平
        if (row == 0) {
            GPIO_WriteOutBits(ROW_PORT, ROW0_PIN, RESET);
            GPIO_WriteOutBits(ROW_PORT, ROW1_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW2_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW3_PIN, SET);
        } else if (row == 1) {
            GPIO_WriteOutBits(ROW_PORT, ROW0_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW1_PIN, RESET);
            GPIO_WriteOutBits(ROW_PORT, ROW2_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW3_PIN, SET);
        } else if (row == 2) {
            GPIO_WriteOutBits(ROW_PORT, ROW0_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW1_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW2_PIN, RESET);
            GPIO_WriteOutBits(ROW_PORT, ROW3_PIN, SET);
        } else if (row == 3) {
            GPIO_WriteOutBits(ROW_PORT, ROW0_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW1_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW2_PIN, SET);
            GPIO_WriteOutBits(ROW_PORT, ROW3_PIN, RESET);
        }
        
        // 短暂延时，等待电平稳定
        for (i = 0; i < 5; i++);
        
        // 读取所有列的状态
        col0 = GPIO_ReadInBit(COL_PORT, COL0_PIN);
        col1 = GPIO_ReadInBit(COL_PORT, COL1_PIN);
        col2 = GPIO_ReadInBit(COL_PORT, COL2_PIN);
        col3 = GPIO_ReadInBit(COL_PORT, COL3_PIN);
        
        // 将列状态转换为按键值（按下为0，检测低电平）
        if (col0 == RESET) {
            key_value |= (1 << (row * MATRIX_COLS + 0));
        }
        if (col1 == RESET) {
            key_value |= (1 << (row * MATRIX_COLS + 1));
        }
        if (col2 == RESET) {
            key_value |= (1 << (row * MATRIX_COLS + 2));
        }
        if (col3 == RESET) {
            key_value |= (1 << (row * MATRIX_COLS + 3));
        }
    }
    
    // 恢复所有行为高电平
    GPIO_WriteOutBits(ROW_PORT, ROW0_PIN, SET);
    GPIO_WriteOutBits(ROW_PORT, ROW1_PIN, SET);
    GPIO_WriteOutBits(ROW_PORT, ROW2_PIN, SET);
    GPIO_WriteOutBits(ROW_PORT, ROW3_PIN, SET);
    
    return key_value;
}

/**
  * @brief  处理单个按键状态机
  */
static void Key_ProcessStateMachine(Key_Info *key, unsigned char is_pressed)
{
    switch (key->state) {
        case KEY_STATE_IDLE:
            if (is_pressed) {
                key->press_cnt++;
                if (key->press_cnt >= DEBOUNCE_TIME) {
                    key->state = KEY_STATE_PRESS;
                    key->press_cnt = 0;
                    key->timestamp = scan_counter;
                    
                    // 生成按下事件
                    key_event.key_code = key->code;
                    key_event.event = KEY_EVENT_PRESS;
                    key_event.duration = 0;
                }
            } else {
                key->press_cnt = 0;
            }
            break;
            
        case KEY_STATE_PRESS:
            if (is_pressed) {
                key->long_cnt++;
                if (key->long_cnt >= LONG_PRESS_TIME) {
                    key->state = KEY_STATE_LONG;
                    key->long_cnt = 0;
                    
                    // 生成长按事件
                    key_event.key_code = key->code;
                    key_event.event = KEY_EVENT_LONG_PRESS;
                    key_event.duration = LONG_PRESS_TIME * 10;
                }
            } else {
                key->state = KEY_STATE_RELEASE;
                key->long_cnt = 0;
            }
            break;
            
        case KEY_STATE_LONG:
            if (is_pressed) {
                // 长按保持，可以在这里添加连击功能
            } else {
                key->state = KEY_STATE_RELEASE;
            }
            break;
            
        case KEY_STATE_RELEASE:
            if (!is_pressed) {
                key->press_cnt++;
                if (key->press_cnt >= DEBOUNCE_TIME) {
                    key->state = KEY_STATE_IDLE;
                    key->press_cnt = 0;
                    
                    // 生成释放事件
                    key_event.key_code = key->code;
                    key_event.event = KEY_EVENT_RELEASE;
                    key_event.duration = (scan_counter - key->timestamp) * 10;
                }
            } else {
                key->press_cnt = 0;
                key->state = KEY_STATE_PRESS;
            }
            break;
    }
}

/**
  * @brief  扫描矩阵按键（主扫描函数，10ms调用一次）
  */
void Key_Matrix_Scan(void)
{
    // 提前声明所有局部变量
    unsigned short raw_keys;
    unsigned char key_count = 0;
    unsigned char row, col;
    unsigned char key_index;
    unsigned char is_pressed;
    
    // 获取原始按键值
    raw_keys = Key_Matrix_GetKeyValue();
    
    // 更新扫描计数器
    scan_counter++;
    
    // 清除上次的事件（除非事件还未被读取）
    if (key_event.event != KEY_EVENT_NONE && key_event.key_code != KEY_NONE) {
        // 事件已被处理，可以清除
        if (key_count++ > 0) {
            key_event.event = KEY_EVENT_NONE;
        }
    }
    
    // 遍历所有按键，更新状态
    for (row = 0; row < MATRIX_ROWS; row++) {
        for (col = 0; col < MATRIX_COLS; col++) {
            key_index = row * MATRIX_COLS + col;
            is_pressed = (raw_keys & (1 << key_index)) ? 1 : 0;
            
            Key_ProcessStateMachine(&key_matrix[row][col], is_pressed);
        }
    }
    
    // 更新按键按下状态（位图）
    matrix_key_pressed = (raw_keys != 0);
}

/**
  * @brief  获取按键事件
  * @return 按键事件结构体
  */
Key_Event Key_Matrix_GetEvent(void)
{
    Key_Event event;
    
    event = key_event;
    
    // 读取后清除事件
    if (event.event != KEY_EVENT_NONE) {
        key_event.event = KEY_EVENT_NONE;
        key_event.key_code = KEY_NONE;
    }
    
    return event;
}

/**
  * @brief  检查指定按键是否被按下
  * @param  key_code: 按键编码
  * @return 1:按下，0:未按下
  */
unsigned char Key_Matrix_IsPressed(unsigned char key_code)
{
    // 提前声明所有局部变量
    unsigned char row, col;
    
    // 查找对应的按键
    for (row = 0; row < MATRIX_ROWS; row++) {
        for (col = 0; col < MATRIX_COLS; col++) {
            if (key_matrix[row][col].code == key_code) {
                if (key_matrix[row][col].state != KEY_STATE_IDLE && 
                    key_matrix[row][col].state != KEY_STATE_RELEASE) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
    return 0;
}

/**
  * @brief  清除按键事件
  */
void Key_Matrix_ClearEvent(void)
{
    key_event.key_code = KEY_NONE;
    key_event.event = KEY_EVENT_NONE;
    key_event.duration = 0;
}

