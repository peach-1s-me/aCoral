/**
 * @file ipc.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程通信相关头文件
 * @version 2.0
 * @date 2025-04-15
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 *         <tr><td>v1.2 <td>文佳源 <td>2025-02-26 <td>修改acoral_ipc_wait_queue_empty返回值类型bool->acoral_bool
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-04-15 <td>增加消息队列、规范代码格式
 */
#ifndef KERNEL_IPC_H
#define KERNEL_IPC_H
#include "queue.h"
#include "res_pool.h"

#define ACORAL_IPC_MUTEX 0 /* ipc类型：互斥量 */
#define ACORAL_IPC_SEM   1 /* ipc类型：信号量 */
#define ACORAL_IPC_MBOX  2 /* ipc类型：邮箱 */
#define ACORAL_IPC_MQ    3 /* ipc类型：消息队列 */

/* 消息对齐大小 */
#define MSG_ALIGN_SIZE 8

#define ACORAL_MPCP_GLOBAL 1 /* mpcp资源标志：全局 */
#define ACORAL_MPCP_LOCAL  0 /* mpcp资源标志：局部 */
#define ACORAL_DPCP_GLOBAL 1 /* dpcp资源标志：全局 */
#define ACORAL_DPCP_LOCAL  0 /* dpcp资源标志：局部 */

/* 未绑定cpu */
#define UNBIND_CPU 0xff

/**
 * @brief 线程通信结构体
 * 
 */
typedef struct
{
    acoral_res_t      res;        /* 资源结构体 */
    acoral_u8         type;       /* ipc类型 */
    acoral_spinlock_t lock;       /* 自旋锁 */
    acoral_16         count;      /* 动态ipc数量(当前数量) */
    acoral_u16        number;     /* ipc数量（最大数量） */
    acoral_queue_t    wait_queue; /* 等待队列, 消息队列接收等待队列 */
    void             *data;       /* 数据 */
} acoral_ipc_t;

/**
 * @brief 消息队列消息结构体
 * 
 */
typedef struct _acoral_mq_message
{
    struct _acoral_mq_message *next;   /* 结点指针 */
    acoral_size                length; /* 长度 */
} acoral_mq_message_t;

/**
 * @brief 消息队列结构体
 * 
 */
typedef struct
{
    acoral_ipc_t *msg_queue_ipc; /* 消息队列ipc指针 */
    void         *msg_buf;       /* 消息队列起始地址 */
    acoral_size   msg_size;      /* 消息大小 */

    void          *mq_head;      /* 消息队列头 */
    void          *mq_tail;      /* 消息队列尾 */
    void          *mq_free;      /* 指向消息队列中空闲消息的首结点 */

    acoral_queue_t send_wait_queue; /* 发送等待队列 */
} acoral_mq_t;


/**
 * @brief mpcp结构体
 * 
 */
typedef struct
{
    acoral_ipc_t *sem;      /* 信号量 */
    acoral_u8 type;         /* 资源标志 */
    acoral_u32 cpu;         /* 所在cpu */
    acoral_u8 prio_ceiling; /* 天花板优先级 */
    acoral_u8 prio_switch;  /* 暂存优先级 */
    acoral_list_t stacking; /* 局部资源栈系统栈链表节点 */
    acoral_spinlock_t lock; /* 自旋锁 */
} acoral_mpcp_t;


/**
 * @brief dpcp结构体
 *
 */
typedef struct
{
    acoral_ipc_t *sem;          /* 信号量 */
    acoral_u32    cpu;          /* 所在cpu */
    acoral_u8     type;         /* 资源标志 */
    acoral_u8     prio_ceiling; /* 天花板优先级 */
    acoral_u8     prio_switch;  /* 暂存优先级 */
    acoral_list_t stacking;     /* 系统栈链表节点 */
    acoral_list_t list;
    void         *data;         /* 数据 */
} acoral_dpcp_t;

/**
 * @brief mpcp局部资源系统结构体
 * 
 */
typedef struct
{
    acoral_u8      prio;  /* 局部资源系统优先级 */
    acoral_queue_t stack; /* 局部资源系统栈 */
    acoral_queue_t wait;  /* 局部资源系统等待队列 */
} acoral_mpcp_system_t;

/**
 * @brief dpcp局部资源系统结构体
 * 
 */
typedef struct
{
    acoral_u8      prio;  /* 系统优先级 */
    acoral_queue_t stack; /* 系统优先级栈 */
    acoral_queue_t wait;  /* 系统等待队列 */
} acoral_dpcp_system_t;

extern acoral_queue_t acoral_dpcp_queue[2][CFG_MAX_CPU];

void acoral_ipc_sys_init(void);
void acoral_ipc_pool_init(void);
acoral_ipc_t *acoral_ipc_alloc(void);
void acoral_ipc_free(acoral_ipc_t *ipc);
void acoral_ipc_init(acoral_ipc_t *ipc);

acoral_bool acoral_ipc_wait_queue_empty(acoral_ipc_t *ipc);
void *acoral_ipc_high_thread(acoral_ipc_t *ipc);
void acoral_ipc_wait_queue_add(acoral_ipc_t *ipc, void *new);
void acoral_ipc_wait_queue_del(acoral_ipc_t *ipc, void *old);

acoral_err acoral_mutex_init(acoral_ipc_t* ipc);
acoral_ipc_t *acoral_mutex_create(void);
acoral_err acoral_mutex_del(acoral_ipc_t* ipc);
acoral_err acoral_mutex_pend(acoral_ipc_t* ipc);
acoral_err acoral_mutex_post(acoral_ipc_t* ipc);
acoral_err acoral_mutex_trypend(acoral_ipc_t* ipc);

acoral_err acoral_sem_init(acoral_ipc_t *ipc, acoral_u32 sem_num);
acoral_ipc_t *acoral_sem_create(acoral_u32 sem_num);
acoral_err acoral_sem_del(acoral_ipc_t *ipc);
acoral_err acoral_sem_pends(acoral_ipc_t *ipc);
acoral_err acoral_sem_pend(acoral_ipc_t *ipc);
acoral_err acoral_sem_posts(acoral_ipc_t *ipc);
acoral_err acoral_sem_post(acoral_ipc_t *ipc);
acoral_32 acoral_sem_getnum(acoral_ipc_t *ipc);

acoral_err acoral_mq_init(
    acoral_mq_t *mq, 
    void        *msg_buf, 
    acoral_size  buf_size, 
    acoral_size  msg_size
);
acoral_err acoral_mq_detach(acoral_mq_t *mq);
acoral_mq_t *acoral_mq_create(acoral_u16 max_msg_number, acoral_size msg_size);
acoral_err acoral_mq_del(acoral_mq_t *mq);
acoral_err acoral_mq_send(
    acoral_mq_t *mq, 
    const void  *buffer,
    acoral_size  size
);
acoral_err acoral_mq_send_wait(
    acoral_mq_t *mq,
    const void  *buffer,
    acoral_size  size,
    acoral_32    timeout
);
acoral_err acoral_mq_recv(
    acoral_mq_t *mq,
    void        *buffer,
    acoral_size  size,
    acoral_32    timeout
);

void acoral_mpcp_system_init(void);
acoral_mpcp_t *acoral_mpcp_create(acoral_u8 type, acoral_u8 prio_ceiling);
acoral_err acoral_mpcp_del(acoral_mpcp_t *mpcp);
acoral_err acoral_mpcp_pend(acoral_mpcp_t *mpcp);
acoral_err acoral_mpcp_post(acoral_mpcp_t *mpcp);

void acoral_dpcp_system_init(void);
void acoral_dpcp_set(acoral_dpcp_t *dpcp, acoral_u8 type, acoral_u8 prio_ceiling);
acoral_dpcp_t *acoral_dpcp_create(void);
acoral_err acoral_dpcp_del(acoral_dpcp_t *dpcp);
acoral_err acoral_dpcp_pend(acoral_dpcp_t *dpcp);
acoral_err acoral_dpcp_post(acoral_dpcp_t *dpcp);
#endif
