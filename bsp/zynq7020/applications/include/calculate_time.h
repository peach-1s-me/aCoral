/**
 * @file calculate_time.h
 * @author 胡博文 (648137125@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-10
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2024-07-10 <td>内容
 * </table>
 */
#ifndef _CACULATE_TIME_H
#define _CACULATE_TIME_H

#define GTC_BASE 0xF8F00200
#define GTC_CTRL    0x08
#define GTC_DATL    0x00
#define GTC_DATH    0x04

void print_float(double num, acoral_u8 keep, acoral_32 width);

void tic(void);

double toc(void);

void cal_time_start();

double cal_time_end();

#endif