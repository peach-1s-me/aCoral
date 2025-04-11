/**
 * @file smp.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层对称多处理器头文件
 * @version 2.0
 * @date 2025-04-11
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-04-11 <td>规范代码风格
 */
#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H
#include "config.h"
#include "type.h"

acoral_id get_daemon_id(void);
acoral_u32 acoral_sched_is_start(void);
void acoral_set_sched_start(acoral_u32 value);

void acoral_release_queue_add(acoral_list_t *node);
void acoral_start(void);
void init(void *args);
#endif
