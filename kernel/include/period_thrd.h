/**
 * @file period_thrd.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层周期线程策略相关头文件
 * @version 2.0
 * @date 2025-04-03
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v2.0 <td>文佳源 <td>2025-04-03 <td>规范代码风格
 */
#ifndef KERNEL_PERIOD_THRD_H
#define KERNEL_PERIOD_THRD_H

#include "type.h"

/* 周期调度策略数据结构体 */
typedef struct
{
    acoral_u32  cpu;  /* 所在cpu */
    acoral_u8   prio; /* 优先级 */
    acoral_time time; /* 周期时间 */
} acoral_period_policy_data_t;

void period_policy_init(void);

#endif
