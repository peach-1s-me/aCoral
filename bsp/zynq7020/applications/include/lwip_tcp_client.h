/**
 * @file lwip_tcp_client.h
 * @author 文佳源 (648137125@qq.com)
 * @brief lwip的tcp客户端用户数据传输接口
 * @version 1.1
 * @date 2025-03-27
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-02-25 <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */
#ifndef LWIP_TCP_CLIENT_H
#define LWIP_TCP_CLIENT_H
#include "acoral.h"

extern acoral_u8 is_connected_to_server;

void lwip_client_init(void);

size_t lwip_client_write(
    const acoral_u8 *buf,
    size_t len,
    acoral_u8 *errcode
);
size_t lwip_client_read(
    acoral_u8 *buf,
    size_t len,
    acoral_32 timeout,
    acoral_u8 *errcode
);

#endif
