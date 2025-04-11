/**
 * @file timed_thrd.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层时间确定性线程策略相关源文件
 * @version 1.2
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-27 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-20 <td>time_deal的bug，核0没有安排线程就都不运行了，改变head的检测返回
 *         <tr><td>v1.2 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 */
#include <type.h>
#include <cpu.h>
#include <queue.h>
#include <lsched.h>
#include <hal.h>
#include <mem.h>
#include <timer.h>
#include <policy.h>
#include <timed_thrd.h>
#include <print.h>
#include <error.h>

///时间确定性线程策略结构体实例
acoral_sched_policy_t timed_policy;
///时间确定性线程队列
acoral_queue_t timed_time_queue[CFG_MAX_CPU];

/**
 * @brief 时间确定性线程私有数据结构体
 *
 */
typedef struct{
    acoral_time *start_time;///<开始执行时间
    acoral_time *exe_time;///<结束执行时间
    acoral_ipc_t **ipc;///<信号量指针
    acoral_u8 section_num;///<段数量
    acoral_u8 frequency;///<执行次数
    acoral_u8 section;///<执行段
    acoral_u8 run_num;///<执行次序
    void (*route)(void *args);///<线程运行函数
    void (**section_route)(void *section_args);///<段执行函数
    void **section_args;///<段执行函数参数
    void *args;///<传递参数
}timed_private_data_t;

/**
 * @brief 时间确定性线程队列添加节点
 * 
 * @param new 要添加的线程tcb指针
 */
static void acoral_timed_time_queue_add(acoral_thread_t *new)
{
    acoral_u32 cpu = new->cpu;
    acoral_tick_queue_add(&timed_time_queue[cpu], &new->timing);//以时刻队列方式添加
}
/**
 * @brief 时间确定性线程队列删除节点
 * 
 * @param old 要删除的线程tcb指针
 */
static void acoral_timed_queue_del(acoral_thread_t *old)
{
    acoral_u32 cpu = old->cpu;
    acoral_tick_queue_del(&timed_time_queue[cpu], &old->timing);//以时刻队列方式添加
}
/**
 * @brief 起始时间装载
 * 
 * @param thread 线程tcb指针
 */
static void timed_start_time_reload(acoral_thread_t *thread)
{
    static acoral_time time_last = 0;
    timed_private_data_t *private_data=thread->private_data;
    acoral_u32 offset =(acoral_u32)((private_data->run_num-1)*(private_data->section_num+1));
    offset += private_data->section-1;//计算起始时间数组偏移量
    if((private_data->run_num==1)&&(private_data->section==1))
        time_last = 0;
    time_last=thread->time;//当前时间
    thread->time=TIME_TO_TICKS(*(private_data->start_time + offset));//换算
    acoral_vlist_init(&thread->timing, thread->time - time_last);//初始化timing链表
    acoral_timed_time_queue_add(thread);//添加进周期队列
}
/**
 * @brief 时间确定性线程退出函数
 * 
 */
static void timed_thread_exit()
{
    acoral_suspend_self();
}

/**
 * @brief 时间确定性线程初始化
 * 
 * @param thread 线程结构体指针
 * @param route 线程运行函数
 * @param args 传递参数
 * @param data 数据
 * @return acoral_id 线程id
 */
static acoral_id timed_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *p_data,void *data)
{
    acoral_err err;
    acoral_timed_policy_data_t *policy_data;
    timed_private_data_t *private_data;
    if(thread->policy==ACORAL_SCHED_POLICY_TIMED)
    {
        //设置时间确定性线程数据
        policy_data = (acoral_timed_policy_data_t *)p_data;
        thread->cpu = policy_data->cpu;
        thread->prio = ACORAL_TIMED_PRIO;
        //给私有数据分配内存
        private_data=(timed_private_data_t *)acoral_vol_malloc(sizeof(timed_private_data_t));
        if(private_data==NULL)//检测分配结果
        {
            acoral_printerr("No level2 mem space for private_data:%s\n",thread->name);
            acoral_enter_critical();
            acoral_release_res((acoral_res_t *)thread);
            acoral_exit_critical();
            return KR_MEM_ERR_MALLOC;
        }
        //设置私有数据
        private_data->start_time = policy_data->start_time;
        private_data->exe_time = policy_data->exe_time;
        private_data->section_num = policy_data->section_num;
        private_data->frequency = policy_data->frequency;
        private_data->section = 1;
        private_data->run_num = 1;
        private_data->route = route;
        private_data->section_route = policy_data->section_route;
        private_data->section_args = policy_data->section_args;
        private_data->args = args;
        private_data->ipc = (acoral_ipc_t **)acoral_vol_malloc(private_data->section_num*sizeof(acoral_ipc_t *));
        for(acoral_u32 i=0;i<private_data->section_num;i++)
        {
            private_data->ipc[i]=acoral_sem_create(0);
        }
        thread->private_data=private_data;
        thread->cpu_mask=-1;
    }
    if((err = acoral_thread_init(thread,route,timed_thread_exit,args))!=0)//通用线程初始化
    {
        acoral_printerr("No thread stack:%s\n",thread->name);
        acoral_enter_critical();
        acoral_release_res((acoral_res_t *)thread);
        acoral_exit_critical();
        return err;
    }
    thread->data = data;
    thread->preempt_type = ACORAL_PREEMPT_TIMED;
    acoral_enter_critical();
    timed_start_time_reload(thread);
    acoral_exit_critical();
    //将线程就绪，并重新调度
    acoral_rdy_thread(thread);
    return thread->res.id;
}

/**
 * @brief 时间确定性调度通用线程
 * 
 * @param args 参数
 */
void acoral_timed_task(void *args)
{
    acoral_thread_t *cur=acoral_cur_thread;
    timed_private_data_t *private_data=cur->private_data;
    for(int i=0;i<private_data->section_num;i++)
    {
        acoral_sem_pend(private_data->ipc[i]);//获取该段信号量
        private_data->section_route[i](private_data->section_args[i]);
    }
}


/**
 * @brief 时间确定性线程释放函数
 *
 * @param thread 线程结构体指针
 */
static void timed_policy_thread_release(acoral_thread_t *thread)
{
    timed_private_data_t *private_data = thread->private_data;
    acoral_vol_free(private_data->section_route);
    private_data->section_route = NULL;
    acoral_vol_free(private_data->section_args);
    private_data->section_args = NULL;
    acoral_vol_free(private_data->ipc);
    private_data->ipc = NULL;
    acoral_vol_free(thread->private_data);//回收私有数据内存
    thread->private_data = NULL;
    //回收钩子函数
    if(thread->hook.release_hook!=NULL)
        thread->hook.release_hook(thread);
}
#include "calculate_time.h"
#include "measure.h"
/**
 * @brief 时间确定性线程策略处理函数
 * 
 */
static void timed_time_deal()
{
    acoral_list_t *tmp,*tmp1,*head;
    acoral_thread_t *thread;
    timed_private_data_t *private_data;
    static acoral_thread_t *last_timed_thread[CFG_MAX_CPU];
    acoral_u8 timed_error = 0;//时间确定性线程错误

#if (MEASURE_SCHED_TIMED == 1)
    acoral_u8 is_valid_measure = 0;
    // #error "hihihi"
    cal_time_start();
#endif

    for(acoral_u8 cpu=0;cpu<CFG_MAX_CPU;cpu++)
    {
        head=&timed_time_queue[cpu].head;
        if(acoral_list_empty(head))
        {
            //最后的循环才return
            if(cpu==CFG_MAX_CPU - 1)
                return;
            else
                continue;
        }
        if(head->next->value>0)//时刻--
            head->next->value--;
        for(tmp=head->next;tmp!=head;)//轮询找到要被唤醒的线程
        {
            thread=list_entry(tmp,acoral_thread_t,timing);
            if(tmp->value>0)
                break;
#if (MEASURE_SCHED_TIMED == 1)
            is_valid_measure = 1;
#endif
            if(last_timed_thread[cpu] == acoral_cur_thread)//当前运行的线程还没停止
            {
                private_data=last_timed_thread[cpu]->private_data;//读取当前线程私有数据
                acoral_suspend_thread(last_timed_thread[cpu]);//挂起当前运行线程
                last_timed_thread[cpu]->state|=ACORAL_THREAD_STATE_RELOAD;//设置当前运行线程为重载状态
                private_data->section = private_data->section_num + 1;//直接装载下个周期起始时间
                timed_error = 1;
            }
            private_data=thread->private_data;
            tmp1 = tmp->next;
            acoral_timed_queue_del(thread);//从队列中删除要被唤醒的线程
            tmp = tmp1;
            last_timed_thread[cpu] = thread;
            if(thread->ipc!=NULL)//在段中
            {
                if(private_data->section<=private_data->section_num)
                {
                    acoral_sem_post(private_data->ipc[private_data->section-1]);
                    private_data->section++;
                }
            }
            else//周期到达
            {
                if(!timed_error)//线程错误会运行到这，设置变量以跳过
                {
                    private_data->run_num++;//下一实例准备执行
                    private_data->section=1;//初始化段标记
                    //超周期装载
                    if(private_data->run_num>private_data->frequency)//在可执行次数范围外
                    {
                        private_data->run_num=1;
                        thread->time = 0;
                    }
                    //初始化信号量
                    for(acoral_u8 i=0;i<private_data->section_num;i++)
                        acoral_sem_init(private_data->ipc[i], 0);
                    if(thread->hook.deal_hook!=NULL)
                        thread->hook.deal_hook(thread);
                    thread->stack=(acoral_u32 *)((acoral_8 *)thread->stack_buttom+thread->stack_size-4);
                    HAL_STACK_INIT(&thread->stack,private_data->route,timed_thread_exit,private_data->args);
                    acoral_rdy_thread(thread);
                }
            }
            //起始时间装载
            if(private_data->run_num<=private_data->frequency)
                timed_start_time_reload(thread);
        }
    }

#if (MEASURE_SCHED_TIMED == 1)
    extern during_buffer_t sched_buffer;
    double during = cal_time_end();
    if ((during > 0) && (1 == is_valid_measure))
    {
        push_during(&sched_buffer, during);
    }
#endif
}

/**
 * @brief 时间确定性线程策略初始化
 *
 */
void timed_policy_init(void)
{
    for(acoral_u8 cpu=0;cpu<CFG_MAX_CPU;cpu++)
    {
        acoral_tick_queue_init(&timed_time_queue[cpu]);//初始化时间确定性线程队列
    }
    acoral_list_init(&timed_policy.list);
    timed_policy.type=ACORAL_SCHED_POLICY_TIMED;
    timed_policy.policy_thread_init=timed_policy_thread_init;
    timed_policy.policy_thread_release=timed_policy_thread_release;
    timed_policy.time_deal=timed_time_deal;
    timed_policy.name="timed";
    acoral_register_sched_policy(&timed_policy);//注册策略
}
