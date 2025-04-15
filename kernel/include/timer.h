/**
 * @file timer.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层时钟相关头文件
 * @version 2.0
 * @date 2025-04-14
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-12 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-04-14 <td>规范代码风格, 增加TICKS_TO_TIME
 */
#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H
#include "config.h"
#include "type.h"
#include "queue.h"
#include "res_pool.h"
/* 重定义获取ticks函数为变量acoral_ticks，便于使用 */
#define acoral_ticks acoral_get_ticks()
/* 计算time(ms)对应的ticks数量 */
#define TIME_TO_TICKS(time)  (time)*CFG_TICKS_PER_SEC/1000
/* 计算ticks数对应的time(ms) */
#define TICKS_TO_TIME(ticks) (ticks)*1000/CFG_TICKS_PER_SEC

void acoral_ticks_init(void);
acoral_time acoral_get_ticks(void);
void acoral_set_ticks(acoral_time time);
void time_delay_deal(void);
void acoral_time_delay_queue_add(void *new);
void acoral_time_delay_queue_del(void *old);
acoral_ipc_t *acoral_ipc_alloc(void);
void acoral_time_sys_init(void);
#endif

