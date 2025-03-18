/**
 * @file hal_int_c.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层中断相关源文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>增加注释
 */
#include <hal_int.h>
#include <config.h>
///中断退出调度标志
acoral_u32 hal_irq_switch[CFG_MAX_CPU] = {0};

///iccpmr中断优先级屏蔽寄存器
#define ICCPMR_REG ( *( ( volatile acoral_u32 * ) 0xF8F00104 ) )
///中断优先级unmask
#define INTR_UNMASK 0xFFUL
///中断优先级mask
#define INTR_MASK 0x90UL

/**
 * @brief 中断入口函数，弱定义
 * 
 * @param ulICCIAR 中断寄存器
 */
void __weak hal_all_entry( acoral_u32 ulICCIAR )
{
}

/**
 * @brief 屏蔽优先级低于0x90的中断
 * 
 */
void hal_intr_disable_save(void)
{
    HAL_INTR_DISABLE();
    ICCPMR_REG = (acoral_u32)INTR_MASK;
    __asm volatile (    "dsb        \n"
                        "isb        \n" ::: "memory" );
    HAL_INTR_ENABLE();
}

/**
 * @brief 恢复中断优先级mask
 * 
 */
void hal_intr_restore(void)
{
    HAL_INTR_DISABLE();
    ICCPMR_REG = (acoral_u32)INTR_UNMASK;
    __asm volatile (    "dsb        \n"
                        "isb        \n" ::: "memory" );
    HAL_INTR_ENABLE();
}
