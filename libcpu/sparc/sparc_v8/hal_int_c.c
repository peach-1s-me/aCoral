/**
 * @file hal_int_c.c
 * @author 文佳源，绕洪江
 * @brief hal层通用源文件，通用即与硬件细节无关
 * @version 1.0
 * @date 2025-05-10
 * 
 * @copyright Copyright (c) 2025
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>文佳源，绕洪江 <td>2025-05-10 <td>创建文件
 */
#include <hal_int.h>
#include <config.h>
#include "cat_log.h"

#include "lsched.h"
extern acoral_thread_t *acoral_running_thread[CFG_MAX_CPU];

///中断退出调度标志
acoral_u32 hal_irq_switch[CFG_MAX_CPU] = {0};



///iccpmr中断优先级屏蔽寄存器
#define ICCPMR_REG ( *( ( volatile acoral_u32 * ) 0xF8F00104 ) )
///中断优先级unmask
#define INTR_UNMASK 0xFFUL
///中断优先级mask
#define INTR_MASK 0x90UL

#if 0
/**
 * @brief 中断入口函数，弱定义
 * 
 * @param ulICCIAR 中断寄存器
 */
void __weak hal_all_entry( acoral_u32 ulICCIAR )
{
}
#endif

acoral_u32 get_core(void);
extern volatile acoral_u32 irq_num[8];
void acoral_intr_entry(acoral_u32 irq_num);
// void _3883_HAL_IRQ_ENTRY(acoral_u32 irq1_num, acoral_u32 irq2_num, acoral_u32 sp)
void _3883_HAL_IRQ_ENTRY(acoral_u32 irq1_num, acoral_u32 irq2_num)
{

#if 0
    static last = 0x0;
    if(sp != last)
    {
        last = sp;
        acoral_print("<<>>sp=0x%x\r\n", sp);
    }
#endif

    acoral_u32 acoral_irq_num;

    if(irq1_num == 6 || irq2_num == 14)
    {
        acoral_irq_num = irq2_num;
    }
    else
    {
        acoral_irq_num = irq1_num;
    }
    acoral_u32 core = get_core();
    irq_num[core] = acoral_irq_num;

    // CLOG_TRACE("irq %d triggered", acoral_irq_num);
    acoral_intr_entry(acoral_irq_num);
}

void _3883_HAL_START_OS(void *to_stack);
void HAL_START_OS()
{
    _3883_HAL_START_OS(&(acoral_running_thread[0]->stack));
}

/**
 * @brief 屏蔽优先级低于0x90的中断
 * 
 */
void hal_intr_disable_save(void)
{
    HAL_INTR_DISABLE();
    /* 复位PSR的ET位 */
    // ICCPMR_REG = (acoral_u32)INTR_MASK;
    // __asm volatile (    "dsb        \n"
    //                     "isb        \n" ::: "memory" );
    // HAL_INTR_ENABLE();
}

/**
 * @brief 恢复中断优先级mask
 * 
 */
void hal_intr_restore(void)
{
    // HAL_INTR_DISABLE();
    // /* 置位PSR的ET位 */
    // ICCPMR_REG = (acoral_u32)INTR_UNMASK;
    // __asm volatile (    "dsb        \n"
    //                     "isb        \n" ::: "memory" );
    HAL_INTR_ENABLE();
}
