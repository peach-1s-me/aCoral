/**
 * @file ipi.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层核间中断头文件
 * @version 1.0
 * @date 2022-07-08
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 */
#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H
#ifdef CFG_SMP
#include <config.h>
#include <type.h>
#include <spinlock.h>

extern acoral_u8 ipi_status[CFG_MAX_CPU];

///核间中断状态：ready
#define ACORAL_IPI_READY 1
///核间中断状态：not ready
#define ACORAL_IPI_NOTREADY 0

///核间中断线程命令移位
#define ACORAL_IPI_THREAD_SHIFT 0
///核间中断命令：kill
#define ACORAL_IPI_THREAD_KILL (1<<ACORAL_IPI_THREAD_SHIFT)
///核间中断命令：ready
#define ACORAL_IPI_THREAD_READY (1<<(ACORAL_IPI_THREAD_SHIFT+1))
///核间中断命令：suspend
#define ACORAL_IPI_THREAD_SUSPEND (1<<(ACORAL_IPI_THREAD_SHIFT+2))
///核间中断命令：change prio
#define ACORAL_IPI_THREAD_CHG_PRIO (1<<(ACORAL_IPI_THREAD_SHIFT+3))
///核间中断命令：move to
#define ACORAL_IPI_THREAD_MOVETO (1<<(ACORAL_IPI_THREAD_SHIFT+4))
///核间中断命令：move in
#define ACORAL_IPI_THREAD_MOVEIN (1<<(ACORAL_IPI_THREAD_SHIFT+5))
///核间中断准入控制命令移位
#define ACORAL_IPI_ADMIS_SHIFT 9
///核间中断命令：change prio
#define ACORAL_IPI_ADMIS_CHG_PRIO (1<<ACORAL_IPI_ADMIS_SHIFT)
///核间中断命令：change res
#define ACORAL_IPI_ADMIS_CHG_RES (1<<(ACORAL_IPI_ADMIS_SHIFT+1))

/**
 * @brief 核间命令结构体
 * 
 */
typedef struct{
    acoral_spinlock_t lock;///<自旋锁
    acoral_u32 cmd;///<命令
    acoral_id thread_id;///<线程id
    void *data;///<数据
}acoral_ipi_cmd_t;

void acoral_ipi_send(acoral_u32);
void acoral_core_ipi_init(void);
void acoral_follow_ipi_init(void);
void acoral_ipi_cmd_init(void);
void acoral_ipi_cmd_send(acoral_u32, acoral_u32, acoral_id, void *);
void acoral_ipi_cmd_handler(void *);
#endif
#endif
