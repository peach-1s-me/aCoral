/**
 * @file measure.h
 * @author 文佳源 (648137125@qq.com)
 * @brief 开销测试
 * @version 0.1
 * @date 2024-07-05
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2024-07-05 <td>内容
 * </table>
 */
#ifndef _MEASURE_H
#define _MEASURE_H

#define SINGLE_MEASURE_MAX_TIME       1000   /* 每次测量的最长时间数据 */

#define TIME_UNIT_US    0
#define TIME_UNIT_MS    1

typedef struct _during_buffer_t
{
    acoral_u32 during[SINGLE_MEASURE_MAX_TIME]; /* 下标表示切换开销时间 */
    acoral_u32 record_num;                      /* 已经记录的时间 */
    acoral_u32 time_unit;                       /* 时间单位 */
} during_buffer_t;

acoral_err push_during(during_buffer_t *buf, double during);

#endif