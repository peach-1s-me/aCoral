/**
 * @file lwip_test_client.c
 * @author 文佳源 (648137125@qq.com)
 * @brief tcp客户端的demo,使用lwip_tcp_client的接口
 * @version 0.1
 * @date 2025-02-25
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-02-25 <td>内容
 * </table>
 */
#include "lwip_tcp_client.h"

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_CUSTOM_TRANSPORT_MTU* STREAM_HISTORY

void lwip_test_client_thread_entry(void *args)
{
    static int count = 0;
    char write_buffer[40];
    char read_buffer[40];
    acoral_u8 err;

    while(0 == is_connected_to_server)
    {
        acoral_print("waiting for connect to server...\r\n");
        acoral_delay_ms(1000);
    }

    while(1)
    {
        sprintf(write_buffer, "The sending count is [%d]", count++);
        lwip_client_write(write_buffer, strlen(write_buffer), &err);
        if(0 != err)
        {
            acoral_print("[ERROR] write error:err %d\r\n", err);
        }

        acoral_delay_ms(1000);

        lwip_client_read(read_buffer, strlen(write_buffer), 1, &err);
        if(0 != err)
        {
            acoral_print("[ERROR] read error:err %d\r\n", err);
        }
        else
        {
            read_buffer[strlen(write_buffer)] = '\0';
            acoral_print("read:->%s\r\n", read_buffer);
        }
    }
}

acoral_32 lwip_app_thread_init(void)
{
    acoral_32 err = -1;

    acoral_print("[lwip_app_thread_init] create app thread\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu = 0;  /* 指定运行的cpu */
    p_data.prio = 3; /* 指定优先级 */
    acoral_id ltc_id = acoral_create_thread(
        lwip_test_client_thread_entry,
        4096,
        NULL,
        "lwip_test_client",
        NULL,
        ACORAL_SCHED_POLICY_COMM,
        &p_data,
        NULL,
        NULL);
    if (ltc_id == -1)
    {
        while (1)
            ;
    }
}
