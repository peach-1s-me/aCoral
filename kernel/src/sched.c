/**
 * @file sched.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层调度相关源文件
 * @version 1.0
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v2.0 <td>胡博文 <td>2023-09-09 <td>临界区与调度锁配合，更改调度时机
 */
#include <type.h>
#include <hal.h>
#include <thread.h>
#include <cpu.h>
#include <int.h>
#include <lsched.h>
///需要调度标志
acoral_u8 need_sched[CFG_MAX_CPU];
///调度锁
acoral_u8 sched_lock[CFG_MAX_CPU];
///临界区嵌套值
acoral_u32 critical_nesting[CFG_MAX_CPU];
///正在运行线程
acoral_thread_t *running_thread[CFG_MAX_CPU];
///最优先就绪线程
acoral_thread_t *ready_thread[CFG_MAX_CPU];
///线程就绪队列实例
acoral_rdy_queue_t acoral_ready_queues[CFG_MAX_CPU];


/**
 * @brief 调度初始化
 * 
 */
void acoral_sched_init(void)
{
    acoral_u8 i;
    for(i=0;i<CFG_MAX_CPU;i++)
    {
        sched_lock[i]=0;//调度锁初始化
        critical_nesting[i]=0;//临界区嵌套初始化
        need_sched[i]=0;//需要调度标志初始化
    }
}
/**
 * @brief 调度锁解锁
 * 
 */
void acoral_sched_unlock(void)
{
    sched_lock[acoral_current_cpu]=0;
    acoral_sched();
}

/**
 * @brief 进入内核临界区
 * 
 */
void acoral_enter_critical(void)
{
    acoral_u32 cpu;
    cpu=HAL_GET_CURRENT_CPU();
    HAL_INTR_DISABLE_SAVE();
    if(!acoral_sched_is_lock)
        acoral_sched_lock();//调度锁上锁
    critical_nesting[cpu]++;
}

/**
 * @brief 退出内核临界区
 * 
 */
void acoral_exit_critical(void)
{
    acoral_u32 cpu;
    cpu=HAL_GET_CURRENT_CPU();
    if(critical_nesting[cpu]>0)
        critical_nesting[cpu]--;
    if(critical_nesting[cpu]==0)//完全退出临界区
    {
        HAL_INTR_RESTORE();
        acoral_sched_unlock();//打开调度锁
    }
}

/**
 * @brief 设置初始线程运行
 * 
 * @param orig 初始线程
 */
void acoral_run_orig_thread(acoral_thread_t *orig)
{
    running_thread[acoral_current_cpu]=orig;
}

/**
 * @brief 设置线程运行
 * 
 * @param thread 要设置的线程
 */
void acoral_set_running_thread(acoral_thread_t *thread)
{
    running_thread[acoral_current_cpu]->state&=~ACORAL_THREAD_STATE_RUNNING;//设置正在运行的线程状态not running
    thread->state|=ACORAL_THREAD_STATE_RUNNING;//设置该线程状态running
    running_thread[acoral_current_cpu]=thread;//改变正在运行的线程为该线程
}

/**
 * @brief 初始化就绪队列
 * 
 */
void acoral_sched_rdyqueue_init()
{
    acoral_thread_prio_array_t *array;
    acoral_rdy_queue_t *rdy_queue;
    acoral_u32 cpu;
    //以线程优先级队列方式初始化
    for(cpu=0;cpu<CFG_MAX_CPU;cpu++)
    {
        rdy_queue=acoral_ready_queues+cpu;
        array=&rdy_queue->array;
        acoral_thread_prio_queue_init(array);
    }
}

/**
 * @brief 就绪队列添加节点
 * 
 * @param new 要添加的线程
 */
void acoral_sched_rdyqueue_add(acoral_thread_t *new)
{
    acoral_rdy_queue_t *rdy_queue;
    acoral_u32 cpu;
    cpu=new->cpu;
    rdy_queue=acoral_ready_queues+cpu;//找到当前核的就绪队列
    acoral_thread_prio_queue_add(&rdy_queue->array,new->prio,&new->ready);//以线程优先级队列方式添加
    new->state&=~ACORAL_THREAD_STATE_SUSPEND;//设置线程状态not suspend
    new->state|=ACORAL_THREAD_STATE_READY;//设置线程状态ready
    //不同核设置id
    if(cpu == 0)
    {
    	new->res.id=new->res.id&~(1<<ACORAL_RES_CPU_BIT);
    }
    else if(cpu == 1)
    {
    	new->res.id=new->res.id|1<<ACORAL_RES_CPU_BIT;
    }
    acoral_set_need_sched(true);//设置需要调度标志
}

/**
 * @brief 就绪队列删除节点
 * 
 * @param old 要删除的线程
 */
void acoral_sched_rdyqueue_del(acoral_thread_t *old)
{
    acoral_32 cpu;
    acoral_rdy_queue_t *rdy_queue;
    cpu=old->cpu;
    rdy_queue=&acoral_ready_queues[cpu];//找到当前核的就绪队列
    acoral_thread_prio_queue_del(&rdy_queue->array,old->prio,&old->ready);//以线程优先级队列方式删除
    old->state&=~ACORAL_THREAD_STATE_READY;//设置线程状态not ready
    old->state&=~ACORAL_THREAD_STATE_RUNNING;//设置线程状态not running
    old->state|=ACORAL_THREAD_STATE_SUSPEND;//设置线程状态suspend
    acoral_set_need_sched(true);//设置需要调度标志
}

/**
 * @brief 切换上下文函数
 *
 */
void acoral_switch_context()
{
    acoral_u8 can_sched = 0;
    acoral_thread_t *prev;
    acoral_thread_t *next;
    acoral_set_need_sched(false);
    prev=acoral_cur_thread;
    acoral_select_thread();//选择最高优先级线程
    next=acoral_ready_thread;//next为最优先就绪线程
    if((prev!=next)||(prev->state&ACORAL_THREAD_STATE_RELOAD))//需要切换的两个线程不同，或者线程为重载状态
    {
        if(prev->state==ACORAL_THREAD_STATE_EXIT)//prev线程是退出状态
        {
#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
            acoral_tswinfos_add(prev, next);
#endif
            acoral_set_running_thread((void *)next);//设置next线程为running线程
            prev->state=ACORAL_THREAD_STATE_RELEASE;//设置prev线程状态release
            return;
        }
        if(prev->state&ACORAL_THREAD_STATE_SUSPEND)
        {
            can_sched = 1;
        }
        else
        {
            //prev是全局且next是局部或者prev是时间确定性的情况下不可调度
            if(!(((prev->preempt_type == ACORAL_PREEMPT_GLOBAL)&&(next->preempt_type == ACORAL_PREEMPT_LOCAL))||(prev->preempt_type == ACORAL_PREEMPT_TIMED)))
            {
                can_sched = 1;
            }
        }
        if(can_sched)
        {
#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
            acoral_tswinfos_add(prev, next);
#endif
            acoral_set_running_thread((void *)next);//设置next线程为running线程
            if(prev->state&ACORAL_THREAD_STATE_RELOAD)//prev线程是重载状态
            {
                prev->state&=~ACORAL_THREAD_STATE_RELOAD;
                return;
            }
#ifdef CFG_SMP
            if(prev->state&ACORAL_THREAD_STATE_MOVE)//prev线程是迁移状态
            {
                prev->state&=~ACORAL_THREAD_STATE_MOVE;//设置prev线程为非迁移状态
                acoral_spin_unlock(&prev->move_lock);
                return;
            }
#endif
        }
    }
}

/**
 * @brief 选择最高优先级线程
 * 
 */
void acoral_select_thread()
{
    acoral_u32 cpu;
    acoral_u32 index;
    acoral_rdy_queue_t *rdy_queue;
    acoral_thread_prio_array_t *array;
    acoral_list_t *head;
    acoral_thread_t *thread;
    acoral_queue_t *queue;
    cpu=acoral_current_cpu;
    rdy_queue=acoral_ready_queues+cpu;//找到当前核的就绪队列
    array=&rdy_queue->array;//得到线程优先级array
    index = acoral_thread_get_high_prio(array);//找出就绪队列中优先级最高的线程的优先级
    //通过array找到线程
    queue = array->queue + index;
    head=&queue->head;
    thread=list_entry(head->next, acoral_thread_t, ready);
    acoral_set_ready_thread(thread);//设置最优先就绪线程
}

#include "calculate_time.h"
acoral_u8 start_measure_context_switch = 0;
/**
 * @brief 调度线程
 * 
 */
void acoral_sched(void)
{
    double switch_during = 0;
    /*如果不需要调度，则返回*/
    if(!acoral_need_sched)
        return;
    //中断中
    if(acoral_intr_nesting)
        return;
    //调度锁
    if(acoral_sched_is_lock)
        return;
    /*如果还没有开始调度，则返回*/
    if(!acoral_sched_enable)
        return;
#if (MEASURE_CONSEXT_SWITCH == 1)
    if(1 == start_measure_context_switch)
    {
        cal_time_start();
    }
#endif
    HAL_COMM_SWITCH();
    return;
}
