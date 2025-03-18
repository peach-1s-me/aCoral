/**
 * @file lsched.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层调度相关头文件
 * @version 1.0
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 */
#ifndef KERNEL_LSCHED_H
#define KERNEL_LSCHED_H
#include <thread.h>
#include <cpu.h>
extern acoral_u8 need_sched[];
extern acoral_u8 sched_lock[];
extern acoral_thread_t *running_thread[];
extern acoral_thread_t *ready_thread[];
/**
 * @brief 线程就绪队列组结构体
 * 
 */
typedef struct{
    acoral_thread_prio_array_t array;///<线程优先级array
}acoral_rdy_queue_t;

///重定义调度锁上锁为函数
#define acoral_sched_lock() (sched_lock[acoral_current_cpu]=1)
void acoral_sched_unlock();
///重定义检测调度锁为函数
#define acoral_sched_is_lock (sched_lock[acoral_current_cpu])

void acoral_enter_critical(void);
void acoral_exit_critical(void);

///重定义检测需要调度标志为函数
#define acoral_need_sched need_sched[acoral_current_cpu]
///重定义设置需要调度标志为函数
#define acoral_set_need_sched(val) (need_sched[acoral_current_cpu]=(val))

///重定义获取正在运行线程为函数
#define acoral_get_running_thread(cpu) (running_thread[(cpu)])
void acoral_set_running_thread(acoral_thread_t *thread);

///重定义获取最优先就绪线程为函数
#define acoral_get_ready_thread() (ready_thread[acoral_current_cpu])
///重定义设置最优先就绪线程为函数
#define acoral_set_ready_thread(thread) (ready_thread[acoral_current_cpu]=(thread))
///重定义获取最优先就绪线程函数为变量
#define acoral_ready_thread acoral_get_ready_thread()

///重定义获取当前核正在运行函数为当前线程变量
#define acoral_cur_thread acoral_get_running_thread(acoral_current_cpu)

void acoral_sched_init();

void acoral_sched_rdyqueue_init(void);
void acoral_sched_rdyqueue_add(acoral_thread_t *new);
void acoral_sched_rdyqueue_del(acoral_thread_t *old);

void acoral_sched(void);
void acoral_select_thread();
void acoral_run_orig_thread(acoral_thread_t *orig);
#endif
