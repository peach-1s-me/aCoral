/**
 * @file smp.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层对称多处理器头文件
 * @version 1.0
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 */
#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H
#include <config.h>
#include <type.h>

volatile extern acoral_u32 acoral_sched_enable;
extern acoral_id idle_id;
extern acoral_id daemon_id;

void acoral_start(void);
void init(void *args);
#endif
