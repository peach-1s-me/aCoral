/**
 * @file bitops.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层位图相关头文件
 * @version 1.0
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-02 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-03-27 <td>规范代码风格
 */
#ifndef KERNEL_BITOPS_H
#define KERNEL_BITOPS_H

#include "type.h"

acoral_u32 acoral_find_first_set(acoral_u32 bitmap);
acoral_u32 acoral_find_first_bit(const acoral_u32 *bitmaps, acoral_u32 length);
void acoral_set_bit(acoral_32 position, acoral_u32 *bitmap);
void acoral_clear_bit(acoral_32 position, acoral_u32 *bitmap);
acoral_u32 acoral_get_bit(acoral_32 nr, acoral_u32 *bitmap);

#endif