#include "ht32.h"

// 全局系统毫秒计数器
vu32 tick_ct = 0; 

/**
  * @brief  SysTick 配置
  * @note   设置每 1ms 触发一次中断
  */
void SysTick_Configuration(void)
{
    // 1. 选择 FCLK 作为时钟源 (即 HCLK = 48MHz)
    SYSTICK_ClockSourceConfig(SYSTICK_SRC_FCLK);
    
    // 2. 设置重装载值实现 1ms 定时
    // SystemCoreClock = 48,000,000, 1ms 需要 48,000 个 ticks
    SYSTICK_SetReloadValue(SystemCoreClock / 1000);
    
    // 3. 开启中断
    SYSTICK_IntConfig(ENABLE);
    
    // 4. 清除计数值并使能计数器
    SYSTICK_CounterCmd(SYSTICK_COUNTER_CLEAR);
    SYSTICK_CounterCmd(SYSTICK_COUNTER_ENABLE);
}

/**
  * @brief  SysTick 中断服务函数
  */
void SysTick_Handler(void)
{
    tick_ct++; // 每 1ms 自增，用于 delay_ms 和系统计时
}

/**
  * @brief  微秒延时
  * @param  nus: 要延时的微秒数
  */
void delay_us(u32 nus)
{		
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD;             // 获取重装载值 (48000-1)
    
    // HT32F52352 在 48MHz 下，1us 需要 48 个 ticks
    ticks = nus * (SystemCoreClock / 1000000); 
    
    told = SysTick->VAL;                    // 记录刚进入时的数值
    while(1)
    {
        tnow = SysTick->VAL;	
        if(tnow != told)
        {	    
            // SysTick 是向下计数的递减计数器
            if(tnow < told) 
                tcnt += told - tnow;	    // 正常递减
            else 
                tcnt += reload - tnow + told; // 发生了重装载（跨越了 1ms 边界）
            
            told = tnow;
            if(tcnt >= ticks) break;        // 到达目标时长，退出循环
        }  
    }
}

/**
  * @brief  毫秒延时
  * @param  nms: 要延时的毫秒数
  */
void delay_ms(u32 nms)
{
    u32 start_tick = tick_ct;
    // 使用差值计算，防止 tick_ct 溢出导致死循环
    // 同时不再清零 tick_ct，保证系统全局时间不被打断
    while ((tick_ct - start_tick) < nms);
}

/**
  * @brief  秒延时
  */
void delay_s(u32 ns)
{
    delay_ms(ns * 1000);
}
