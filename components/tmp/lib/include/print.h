/**
 * @file print.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库串口打印相关头文件
 * @version 2.0
 * @date 2023-10-28
 *
 * @copyright Copyright (c) 2022
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-21 <td>增加注释
 *         <tr><td>v2.0 <td>胡博文 <td>2023-10-28 <td>解决多核打印
 */
#ifndef LIB_PRINT_H
#define LIB_PRINT_H
#include <config.h>
#include <type.h>
#include "xil_printf.h"
//#define acoral_print xil_printf
#define acoral_print smp_print
#define acoral_prints acoral_print
#define acoral_printerr acoral_print

void smp_print( const char *ctrl1, ...);

#endif
