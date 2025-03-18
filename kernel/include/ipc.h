/**
 * @file ipc.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程通信相关头文件
 * @version 1.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 *         <tr><td>v1.2 <td>文佳源 <td>2025-02-26 <td>修改acoral_ipc_wait_queue_empty返回值类型bool->acoral_bool
 */
#ifndef KERNEL_IPC_H
#define KERNEL_IPC_H
#include <queue.h>
#include <res_pool.h>

///ipc类型：互斥量
#define ACORAL_IPC_MUTEX 0
///ipc类型：信号量
#define ACORAL_IPC_SEM 1
///ipc类型：邮箱
#define ACORAL_IPC_MBOX 2

///mpcp资源标志：全局
#define ACORAL_MPCP_GLOBAL  1
///mpcp资源标志：局部
#define ACORAL_MPCP_LOCAL   0
///dpcp资源标志：全局
#define ACORAL_DPCP_GLOBAL  1
///dpcp资源标志：局部
#define ACORAL_DPCP_LOCAL   0

///未绑定cpu
#define UNBIND_CPU 0xff

/**
 * @brief 线程通信结构体
 * 
 */
typedef struct{
    acoral_res_t res;///<资源结构体
    acoral_u8 type;///<ipc类型
    acoral_spinlock_t lock;///<自旋锁
    acoral_16 count;///<动态ipc数量
    acoral_u16 number;///<ipc数量
    acoral_queue_t wait_queue;///<等待队列
    acoral_char *name;///<名字
    void *data;///<数据
}acoral_ipc_t;

/**
 * @brief mpcp结构体
 * 
 */
typedef struct{
    acoral_ipc_t *sem;///<信号量
    acoral_u8 type;///<资源标志
    acoral_u32 cpu;///<所在cpu
    acoral_u8 prio_ceiling;///<天花板优先级
    acoral_u8 prio_switch;///<暂存优先级
    acoral_list_t stacking;///<局部资源栈系统栈链表节点
    acoral_spinlock_t lock;///<自旋锁
}acoral_mpcp_t;


/**
 * @brief dpcp结构体
 *
 */
typedef struct{
    acoral_ipc_t *sem;///<信号量
    acoral_u32 cpu;///<所在cpu
    acoral_u8 type;///<资源标志
    acoral_u8 prio_ceiling;///<天花板优先级
    acoral_u8 prio_switch;///<暂存优先级
    acoral_list_t stacking;///<系统栈链表节点
    acoral_list_t list;
    void *data;///<数据
}acoral_dpcp_t;

/**
 * @brief mpcp局部资源系统结构体
 * 
 */
typedef struct{
    acoral_u8 prio;///<局部资源系统优先级
    acoral_queue_t stack;///<局部资源系统栈
    acoral_queue_t wait;///<局部资源系统等待队列
}acoral_mpcp_system_t;

/**
 * @brief dpcp局部资源系统结构体
 * 
 */
typedef struct{
    acoral_u8 prio;///<系统优先级
    acoral_queue_t stack;///<系统优先级栈
    acoral_queue_t wait;///<系统等待队列
}acoral_dpcp_system_t;

extern acoral_queue_t acoral_dpcp_queue[2][CFG_MAX_CPU];

void acoral_ipc_sys_init(void);
void acoral_ipc_pool_init(void);
acoral_ipc_t *acoral_ipc_alloc(void);
void acoral_ipc_init(acoral_ipc_t *);

acoral_bool acoral_ipc_wait_queue_empty(acoral_ipc_t *);
void *acoral_ipc_high_thread(acoral_ipc_t *);
void acoral_ipc_wait_queue_add(acoral_ipc_t *,void *);
void acoral_ipc_wait_queue_del(acoral_ipc_t *, void *);

acoral_err acoral_mutex_init(acoral_ipc_t*);
acoral_ipc_t *acoral_mutex_create(void);
acoral_err acoral_mutex_del(acoral_ipc_t*);
acoral_err acoral_mutex_pend(acoral_ipc_t*);
acoral_err acoral_mutex_post(acoral_ipc_t*);
acoral_err acoral_mutex_trypend(acoral_ipc_t*);

acoral_err acoral_sem_init(acoral_ipc_t *,acoral_u32);
acoral_ipc_t *acoral_sem_create(acoral_u32);
acoral_err acoral_sem_del(acoral_ipc_t *);
acoral_err acoral_sem_pends(acoral_ipc_t *);
acoral_err acoral_sem_pend(acoral_ipc_t *);
acoral_err acoral_sem_posts(acoral_ipc_t *);
acoral_err acoral_sem_post(acoral_ipc_t *);
acoral_32 acoral_sem_getnum(acoral_ipc_t *);

void acoral_mpcp_system_init(void);
acoral_mpcp_t *acoral_mpcp_create(acoral_u8, acoral_u8);
acoral_err acoral_mpcp_del(acoral_mpcp_t *);
acoral_err acoral_mpcp_pend(acoral_mpcp_t *);
acoral_err acoral_mpcp_post(acoral_mpcp_t *);

void acoral_dpcp_system_init(void);
void acoral_dpcp_set(acoral_dpcp_t *dpcp, acoral_u8 type, acoral_u8 prio_ceiling);
acoral_dpcp_t *acoral_dpcp_create(void);
acoral_err acoral_dpcp_del(acoral_dpcp_t *dpcp);
acoral_err acoral_dpcp_pend(acoral_dpcp_t *dpcp);
acoral_err acoral_dpcp_post(acoral_dpcp_t *dpcp);
#endif
