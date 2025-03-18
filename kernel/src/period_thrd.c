/**
 * @file period_thrd.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层周期线程策略相关源文件
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
 */
#include <type.h>
#include <queue.h>
#include <thread.h>
#include <hal.h>
#include <lsched.h>
#include <policy.h>
#include <mem.h>
#include <timer.h>
#include <period_thrd.h>
#include <print.h>
#include <error.h>
///周期线程策略结构体实例
acoral_sched_policy_t period_policy;
///周期线程队列
acoral_queue_t period_time_queue;

/**
 * @brief 周期线程私有数据结构体
 * 
 */
typedef struct{
    acoral_time time;///<周期时间
    void (*route)(void *args);///<线程运行函数
    void *args;///<传递参数
}period_private_data_t;

/**
 * @brief 周期队列添加节点
 * 
 * @param new 要添加的线程节点
 */
static void acoral_period_time_queue_add(acoral_thread_t *new)
{
    acoral_tick_queue_add(&period_time_queue, &new->timing);//以时刻队列方式添加
}

/**
 * @brief 周期队列删除节点
 *
 * @param old 要删除的线程节点
 */
static void acoral_period_queue_del(acoral_thread_t *old)
{
    acoral_tick_queue_del(&period_time_queue, &old->timing);//以时刻队列方式删除
}

/**
 * @brief 周期线程时间重装载
 * 
 * @param thread 要进行装载的线程
 * @param time 要装载的周期时间
 */
static void period_thread_reload(acoral_thread_t* thread)
{
    thread->time=TIME_TO_TICKS(((period_private_data_t *)thread->private_data)->time);//换算
    acoral_vlist_init(&thread->timing, thread->time);//初始化timing链表
    acoral_period_time_queue_add(thread);//添加进周期队列
}
/**
 * @brief 周期线程退出函数
 * 
 */
static void period_thread_exit()
{
    acoral_suspend_self();
}
/**
 * @brief 周期线程初始化
 * 
 * @param thread 线程结构体指针
 * @param route 线程运行函数
 * @param args 传递参数
 * @param data 数据
 * @return acoral_id 线程id
 */
static acoral_id period_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *p_data,void *data)
{
    acoral_err err;
    acoral_u8 prio;
    acoral_period_policy_data_t *policy_data;
    period_private_data_t *private_data;
    if(thread->policy==ACORAL_SCHED_POLICY_PERIOD)
    {
        //设置周期线程数据
        policy_data = (acoral_period_policy_data_t *)p_data;
        thread->cpu = policy_data->cpu;
        prio = policy_data->prio;
        //设定优先级
        thread->prio = prio;
        //给私有数据分配内存
        private_data=(period_private_data_t *)acoral_vol_malloc(sizeof(period_private_data_t));
        if(private_data==NULL)//检测分配结果
        {
            acoral_printerr("No level2 mem space for private_data:%s\n",thread->name);
            acoral_enter_critical();
            acoral_release_res((acoral_res_t *)thread);
            acoral_exit_critical();
            return KR_MEM_ERR_MALLOC;
        }
        //设置私有数据
        private_data->time=policy_data->time;
        private_data->route=route;
        private_data->args=args;
        thread->private_data=private_data;
        thread->cpu_mask=-1;
    }
    if((err = acoral_thread_init(thread,route,period_thread_exit,args))!=KR_OK)//通用线程初始化
    {
        acoral_printerr("No thread stack:%s\n",thread->name);
        acoral_enter_critical();
        acoral_release_res((acoral_res_t *)thread);
        acoral_exit_critical();
        return err;
    }
    thread->data = data;
    acoral_enter_critical();
    //装载周期线程
    period_thread_reload(thread);
    acoral_exit_critical();
    //将线程就绪，并重新调度
    acoral_rdy_thread(thread);
    return thread->res.id;
}
/**
 * @brief 周期线程释放函数
 * 
 * @param thread 线程结构体指针
 */
static void period_policy_thread_release(acoral_thread_t *thread)
{
    acoral_vol_free(thread->private_data);//回收私有数据内存
    thread->private_data = NULL;
    //回收钩子函数
    if(thread->hook.release_hook!=NULL)
        thread->hook.release_hook(thread);
}

#include "measure.h"
#include "calculate_time.h"
/**
 * @brief 周期线程策略处理
 * 
 */
static void period_time_deal()
{
    acoral_list_t *tmp,*tmp1,*head;
    acoral_thread_t * thread;
    period_private_data_t * private_data;
#if (MEASURE_SCHED_PERIOD == 1)
    acoral_u8 is_valid_measure = 0;
    // #error "hihihi"
    cal_time_start();
#endif
    head=&period_time_queue.head;
    if(acoral_list_empty(head))
        return;
    if(head->next->value>0)//时刻--
    	head->next->value--;
    for(tmp=head->next;tmp!=head;)//轮询找到要被唤醒的线程
    {
    	thread=list_entry(tmp,acoral_thread_t,timing);
        if(tmp->value>0)
            break;
        private_data=thread->private_data;
        tmp1 = tmp->next;
        acoral_period_queue_del(thread);//从队列中删除要被唤醒的线程
        tmp = tmp1;
        if(thread->state&ACORAL_THREAD_STATE_SUSPEND)
        {
//        	if(thread->ipc == NULL)//能让周期线程阻塞于ipc的时候不自动就绪
//        	{
            //处理钩子函数
#if (MEASURE_SCHED_PERIOD == 1)
            is_valid_measure = 1;
#endif
            if(thread->hook.deal_hook!=NULL)
                thread->hook.deal_hook(thread);
			thread->stack=(acoral_u32 *)((acoral_8 *)thread->stack_buttom+thread->stack_size-4);
			HAL_STACK_INIT(&thread->stack,private_data->route,period_thread_exit,private_data->args);
			acoral_rdy_thread(thread);
//        	}
        }
        period_thread_reload(thread);//重装载被唤醒的线程
    }

#if (MEASURE_SCHED_PERIOD == 1)
    extern during_buffer_t sched_buffer;
    double during = cal_time_end();
    if ((during > 0) && (1 == is_valid_measure))
    {
        push_during(&sched_buffer, during);
    }
#endif
}

/**
 * @brief 周期线程策略初始化
 * 
 */
void period_policy_init(void)
{
    acoral_tick_queue_init(&period_time_queue);//初始化周期队列
    period_policy.type=ACORAL_SCHED_POLICY_PERIOD;
    period_policy.policy_thread_init=period_policy_thread_init;
    period_policy.policy_thread_release=period_policy_thread_release;
    period_policy.time_deal=period_time_deal;
    period_policy.name="period";
    acoral_register_sched_policy(&period_policy);//注册策略
}
