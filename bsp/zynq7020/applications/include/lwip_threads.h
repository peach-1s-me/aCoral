/**
 * @file lwip_threads.h
 * @author 文佳源 (648137125@qq.com)
 * @brief 
 * @version 1.1
 * @date 2025-03-27
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2024-08-10 <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */
#ifndef _LWIP_THREADS_H
#define _LWIP_THREADS_H
#include <acoral.h>

// acoral_32 lwip_deamon_init(void);
void lwip_app_thread_init(void);

#endif