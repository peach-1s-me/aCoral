/**
 * @file cpu.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层cpu头文件
 * @version 1.0
 * @date 2022-07-11
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-11 <td>增加注释
 */
#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H
#include <config.h>
#include <type.h>
#include <hal_comm.h>
#ifndef CFG_SMP
///单核情况，只有一个核
#define acoral_current_cpu 0
///单核情况，只有一个核
#define acoral_idlest_cpu 0
#else
///重定义获取当前cpu函数
#define acoral_current_cpu HAL_GET_CURRENT_CPU()
///重定义获取最空闲cpu函数
#define acoral_idlest_cpu acoral_get_idlest_cpu()
#endif
acoral_u32 acoral_cpu_is_active(acoral_u32 cpu);
void acoral_cpu_set_active(acoral_u32 cpu);
acoral_u32 acoral_get_idlest_cpu(void);
acoral_u32 acoral_get_idle_maskcpu(acoral_u32 cpu_mask);
#endif
