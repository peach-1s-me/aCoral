/**
 * @file hal_mem.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层内存相关头文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>增加注释
 */

#ifndef HAL_MEM_H
#define HAL_MEM_H

///链接文件符号的代码定义，heap头
extern acoral_u32 _heap_start[];
///链接文件符号的代码定义，heap尾
extern acoral_u32 _heap_end[];

///重定义链接文件中的heap头，为上层使用
#define HAL_HEAP_START  _heap_start
///重定义链接文件中的heap尾，为上层使用
#define HAL_HEAP_END    _heap_end

void hal_mem_init(void);

///重定义hal层文件中的内存初始化函数，为上层使用
#define HAL_MEM_INIT()  hal_mem_init()

#endif
