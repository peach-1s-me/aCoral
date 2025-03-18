/**
 * @file hal_thread.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层线程相关头文件
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

#ifndef HAL_THREAD_H
#define HAL_THREAD_H
#include <type.h>


void hal_stack_init(acoral_u32 **stk,void (*route)(),void (*exit)(),void *args);

///重定义hal层文件中的线程栈初始化函数，为上层使用
#define HAL_STACK_INIT(stack,route,exit,args) hal_stack_init(stack,route,exit,args)

#endif
