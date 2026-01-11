/**
 * @file hal_cmp.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层对称多处理器相关头文件
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

#ifndef HAL_CMP_H
#define HAL_CMP_H
#include <type.h>

#ifdef CFG_SMP
void hal_prepare_cpus(void);
void hal_start_cpu(acoral_u32);
void hal_cmp_ack(void);
void hal_wait_ack(void);

///重定义次核准备函数，为上层使用
#define HAL_PREPARE_CPUS() hal_prepare_cpus()
///重定义启动次核函数，为上层使用
#define HAL_START_CPU(cpu) hal_start_cpu(cpu)

///重定义多核响应函数，为上层使用
#define HAL_CMP_ACK()	hal_cmp_ack()
///重定义多核等待响应函数，为上层使用
#define HAL_WAIT_ACK()	hal_wait_ack()

///设置次核启动入口地址，汇编函数
void HAL_SET_FOLLOW_CPU_ENTRY(void);
///线程迁移上下文切换，汇编函数
void HAL_MOVE_CONTEXT_SWITCH(acoral_u32 **, acoral_u32 **, volatile acoral_32 *, acoral_u32);
///线程中断迁移上下文切换，汇编函数
void HAL_INTR_MOVE_CONTEXT_SWITCH(acoral_u32 **, acoral_u32 **, volatile acoral_32 *, acoral_u32);
#endif
#endif
