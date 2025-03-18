/**
 * @file thread.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程相关头文件
 * @version 1.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-14 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 */
#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H
#include <config.h>
#include <type.h>
#include <queue.h>
#include <ipc.h>
#include <res_pool.h>
#include <smp.h>

///最多优先级个数
#define ACORAL_MAX_PRIO_NUM ((CFG_MAX_THREAD+1)&0xff)
///最多线程个数
#define ACORAL_MAX_THREAD CFG_MAX_THREAD
///最小线程栈大小
#define ACORAL_MIN_STACK_SIZE CFG_MIN_STACK_SIZE 
///最低优先级
#define ACORAL_MINI_PRIO  ACORAL_MAX_PRIO_NUM-1
///最高优先级
#define ACORAL_MAX_PRIO  1

///idle线程优先级
#define ACORAL_IDLE_PRIO ACORAL_MINI_PRIO
///shell线程优先级
#define ACORAL_SHELL_PRIO ACORAL_MINI_PRIO-1
///daemon线程优先级
#define ACORAL_DAEMON_PRIO ACORAL_MINI_PRIO-2
///时间确定性线程优先级
#define ACORAL_TIMED_PRIO ACORAL_MAX_PRIO

///线程状态类型：基底
#define ACORAL_THREAD_STATE_BASE  0
///线程状态类型：就绪
#define ACORAL_THREAD_STATE_READY (1<<ACORAL_THREAD_STATE_BASE)
///线程状态类型：挂起
#define ACORAL_THREAD_STATE_SUSPEND (1<<(ACORAL_THREAD_STATE_BASE+1))
///线程状态类型：正在运行
#define ACORAL_THREAD_STATE_RUNNING (1<<(ACORAL_THREAD_STATE_BASE+2))
///线程状态类型：退出
#define ACORAL_THREAD_STATE_EXIT (1<<(ACORAL_THREAD_STATE_BASE+3))
///线程状态类型：释放
#define ACORAL_THREAD_STATE_RELEASE (1<<(ACORAL_THREAD_STATE_BASE+4))
///线程状态类型：迁移
#define ACORAL_THREAD_STATE_MOVE (1<<(ACORAL_THREAD_STATE_BASE+5))
///线程状态类型：重载
#define ACORAL_THREAD_STATE_RELOAD (1<<(ACORAL_THREAD_STATE_BASE+6))

///线程抢占类型：时间确定性
#define ACORAL_PREEMPT_TIMED    2
///线程抢占类型：全局
#define ACORAL_PREEMPT_GLOBAL    1
///线程抢占类型：局部
#define ACORAL_PREEMPT_LOCAL     0

///线程优先级位图大小
#define ACORAL_THREAD_PRIO_BITMAP_SIZE ((((ACORAL_MAX_PRIO_NUM+1+7)/8)+sizeof(acoral_u32)-1)/sizeof(acoral_u32))

/**
 * @brief 线程钩子函数
 * 
 */
typedef struct{
    void (*deal_hook)(void *);///>调度处理钩子
    void (*release_hook)(void *);///>回收钩子
}acoral_thread_hook_t;

/**
 * @brief 线程控制块结构体
 * 
 */
typedef struct{
    acoral_res_t res;///<资源
    acoral_u32 *stack;///<线程栈顶
    acoral_u32 *stack_buttom;///<线程栈底
    acoral_u32 stack_size;///<线程栈大小
#ifdef CFG_SMP
    acoral_spinlock_t move_lock;///<迁移自旋锁
#endif
    acoral_u8 state;///<线程状态
    acoral_u8 prio;///<线程优先级
    acoral_u32 cpu;///<线程所在cpu
    acoral_u32 cpu_mask;///<线程cpu屏蔽
    acoral_u8 policy;///<线程调度策略
    acoral_list_t ready;///<与就绪队列有关的链表节点
    acoral_list_t pending;///<请求资源被阻塞时用到的链表节点
    acoral_list_t timing;///<以时间方式阻塞时用到的链表节点
    acoral_list_t delaying;///<延时阻塞时用到的链表节点
    acoral_list_t global_list;///<全局线程管理用到的链表节点
    acoral_u8 preempt_type;///<线程抢占类型
    acoral_ipc_t *ipc;///<使用或正在请求的线程通信结构体指针
    acoral_time delay;///<延时时间
    acoral_time time;///<线程时间，可以是周期时间
    acoral_char *name;///<线程名字
    acoral_id console_id;///<控制台id
    acoral_thread_hook_t hook;///<策略钩子函数
    void *private_data;///<私有数据
    void *data;///<其他数据
}acoral_thread_t;

/**
 * @brief 线程优先级array
 * 
 */
typedef struct acoral_thread_prio_array{
    acoral_u32 num;///<线程数量
    acoral_u32 bitmap[ACORAL_THREAD_PRIO_BITMAP_SIZE];///<线程优先级位图
    acoral_queue_t queue[ACORAL_MAX_PRIO_NUM];///<线程优先级队列
}acoral_thread_prio_array_t;

extern acoral_queue_t acoral_threads_queue;

///创建线程函数
#define acoral_create_thread(route,stack_size,args,name,stack,policy,policy_data,data,hook) create_thread_by_policy(route,stack_size,args,name,stack,policy,policy_data,data,hook);

void acoral_kill_thread(acoral_thread_t *thread);
void acoral_kill_thread_by_id(acoral_id id);

acoral_err acoral_rdy_thread(acoral_thread_t *thread);
acoral_err acoral_rdy_thread_by_id(acoral_u32 thread_id);

acoral_err acoral_suspend_thread(acoral_thread_t *thread);
acoral_err acoral_suspend_thread_by_id(acoral_u32 thread_id);
acoral_err acoral_suspend_self(void);

void acoral_thread_change_prio(acoral_thread_t* thread, acoral_u32 prio);
void acoral_thread_change_prio_by_id(acoral_u32 thread_id, acoral_u32 prio);
void acoral_thread_change_prio_self(acoral_u32 prio);

#ifdef CFG_SMP
acoral_err acoral_moveto_thread(acoral_thread_t *thread,acoral_u32 cpu);
acoral_err acoral_moveto_thread_by_id(acoral_id thread_id,acoral_u32 cpu);
acoral_err acoral_jump_cpu(acoral_u32 cpu);

acoral_err acoral_movein_thread(acoral_thread_t *thread);
acoral_err acoral_movein_thread_by_id(acoral_id thread_id);
#endif

acoral_err acoral_prerelease_thread(acoral_thread_t *thread);
void acoral_release_thread(acoral_res_t *res);

acoral_err acoral_delay_thread(acoral_thread_t* thread,acoral_time time);
acoral_err acoral_delay_ms(acoral_time time);

void acoral_thread_prio_queue_add(acoral_thread_prio_array_t *array,acoral_u8 prio,acoral_list_t *list);
void acoral_thread_prio_queue_del(acoral_thread_prio_array_t *array,acoral_u8 prio,acoral_list_t *list);
acoral_u32 acoral_thread_get_high_prio(acoral_thread_prio_array_t *array);
void acoral_thread_prio_queue_init(acoral_thread_prio_array_t *array);

void acoral_set_thread_console(acoral_id id);
acoral_thread_t *acoral_alloc_thread();
acoral_err acoral_thread_init(acoral_thread_t *thread,void (*route)(void *args),void (*exit)(void),void *args);
void acoral_thread_exit(void);
void acoral_thread_sys_init(void);
#endif

