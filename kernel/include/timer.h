/**
 * @file timer.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层时钟相关头文件
 * @version 1.0
 * @date 2022-07-12
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-12 <td>增加注释
 */
#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H
#include <config.h>
#include <type.h>
#include <queue.h>
#include <res_pool.h>
///重定义获取ticks函数为变量acoral_ticks，便于使用
#define acoral_ticks acoral_get_ticks()
///计算time(ms)对应的ticks数量
#define TIME_TO_TICKS(time) (time)*CFG_TICKS_PER_SEC/1000

void acoral_ticks_init(void);
acoral_time acoral_get_ticks(void);
void acoral_set_ticks(acoral_time time);
void time_delay_deal(void);
void acoral_time_delay_queue_add(void*);
acoral_ipc_t *acoral_ipc_alloc(void);
void acoral_time_sys_init(void);
#endif

