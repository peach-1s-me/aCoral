/**
 * @file bitops.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层位图相关头文件
 * @version 1.0
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-02 <td>增加注释
 */
#ifndef KERNEL_BITOPS_H
#define KERNEL_BITOPS_H
#include <type.h>
acoral_u32 acoral_ffs(acoral_u32 word);
acoral_u32 acoral_find_first_bit(const acoral_u32 *b,acoral_u32 length);
void acoral_set_bit(acoral_32 nr,acoral_u32 *p);
void acoral_clear_bit(acoral_32 nr,acoral_u32 *p);
acoral_u32 acoral_get_bit(acoral_32 nr,acoral_u32 *bitmap);
#endif
