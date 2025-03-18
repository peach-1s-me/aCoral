/**
 * @file hal_cmp_c.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层对称多处理器相关源文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>
 */

#include <acoral.h>
#include <hal_cmp.h>
#include "xil_mmu.h"
#include "xil_io.h"
#ifdef CFG_SMP
///cmp自旋锁，用来完成核间的简单等待响应与应答
hal_spinlock_t cmp_lock;
///ARM汇编指令，为启动次核关键指令
#define sev() __asm__("sev")

/**
 * @brief 多核响应函数
 * 
 */
void hal_cmp_ack(void)
{
    HAL_SPIN_UNLOCK(&cmp_lock);
}

/**
 * @brief 多核等待响应函数
 * 
 */
void hal_wait_ack(void)
{
    HAL_SPIN_LOCK(&cmp_lock);
}

/**
 * @brief 启动次核函数
 * 
 */
void hal_start_cpu(acoral_u32 cpu)
{
    sev();
    HAL_WAIT_ACK();//等待次核响应
}

/**
 * @brief 次核准备函数
 * 
 */
void hal_prepare_cpus(void)
{
    Xil_SetTlbAttributes(0xFFFF0000,0x14de2);
    Xil_SetTlbAttributes(0x00000000,0x14de2);
    HAL_SET_FOLLOW_CPU_ENTRY();
    dmb();
    HAL_SPIN_INIT_LOCK(&cmp_lock);
}
#endif
