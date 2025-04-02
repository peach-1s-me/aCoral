/**
 * @file monitor.h
 * @author 高久强
 * @brief 线程切换追踪信息头文件
 * @version 2.0
 * @date 2025-04-02
 * 
 * Copyright (c) 2025 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>高久强 <td>2025-02-24 <td>内容
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-04-02 <td>规范代码风格
 */

#ifndef __KERNEL_MONITOR_H__
#define __KERNEL_MONITOR_H__

#include "acoral.h"

#define MONITOR_CMD_CREATE 0x01
#define MONITOR_CMD_SWITCH (MONITOR_CMD_CREATE << 1)

/* CPU核心信息 */
typedef struct
{
    acoral_u32 id;
    acoral_u32 cpu_frequency;
    char       name[32];
} cpu_core_info_t;

/* 线程信息 */
typedef struct
{
    acoral_u32       thread_policy;
    acoral_u32       prio;
    acoral_thread_t *tcb_ptr;
    acoral_u32       period;
    acoral_u32       supercycle;
    acoral_u32       cpu;
    acoral_u32       create_tick;
    acoral_char      thread_name[32];
} thread_info_t;

/* 线程切换信息 */
typedef struct
{
    acoral_u32       cpu;
    acoral_u32       tick;
    acoral_thread_t *from_tcb;
    acoral_thread_t *to_tcb;
} thread_switch_info_t; /* 每个40 Byte */

typedef struct
{
    acoral_u32    total_size;
    acoral_u32    st_idx;
    acoral_u32    ed_idx;
    acoral_u32    info_type;
    acoral_u32    size;
    acoral_u32    item_bsize;
    acoral_u32    overflow;
    thread_info_t threads[CFG_MAX_THREAD]; /* 这个每次发送的数组长度都是该结构体的size字段 */
} thread_info_set_t; /* 占用3kb以上大小 */

typedef struct
{
    acoral_u32 total_size;
    acoral_u32 threshold; /* 取一半 */
    acoral_u32 st_idx;
    acoral_u32 ed_idx;
    acoral_u32 info_type;
    acoral_u32 size;
    acoral_u32 item_bsize;
    acoral_u32 overflow;

    thread_switch_info_t switch_infos[CFG_MAX_THREAD]; /* 这个每次发送的数组长度都是该结构体的size字段 */
} thread_switch_info_set_t; /* 占用7kb以上大小 */

void acoral_monitor_init(void);

void acoral_tinfos_add(acoral_thread_t *thread);
void acoral_tswinfos_add(acoral_thread_t *from, acoral_thread_t *to);

void acoral_infos_send();
#endif
