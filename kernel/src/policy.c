/**
 * @file policy.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程策略相关源文件
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
#include <type.h>
#include <hal.h>
#include <queue.h>
#include <thread.h>
#include <lsched.h>
#include <policy.h>
#include <print.h>
#include <error.h>
#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
#include <monitor.h>
#endif
///策略队列
acoral_queue_t policy_queue;

void comm_policy_init(void);
void period_policy_init(void);
void timed_policy_init(void);

/**
 * @brief 获取对应策略类型的结构体
 * 
 * @param type 策略类型
 * @return acoral_sched_policy_t* 调度策略结构体指针
 */
static acoral_sched_policy_t *acoral_get_policy_ctrl(acoral_u8 type)
{
    acoral_list_t   *tmp,*head;
    acoral_sched_policy_t  *policy_ctrl;
    head=&policy_queue.head;
    tmp=head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//查找对应策略类型
    {
        policy_ctrl=list_entry(tmp,acoral_sched_policy_t,list);
        if(policy_ctrl->type==type)
            return policy_ctrl;//返回对应结构体指针
    }
    return NULL;//没找到
}
/**
 * @brief 基于策略的线程初始化
 * 
 * @param policy_type 策略类型
 * @param thread 线程
 * @param route 线程运行函数
 * @param args 传递参数
 * @param data 数据
 * @return acoral_id 线程id
 */
static acoral_id acoral_policy_thread_init(acoral_u32 policy_type,acoral_thread_t *thread,void (*route)(void *args),void *args,void *p_data,void *data)
{

    acoral_sched_policy_t *policy_ctrl;
    policy_ctrl=acoral_get_policy_ctrl(policy_type);//获取对应策略类型的结构体
    if(policy_ctrl==NULL||policy_ctrl->policy_thread_init==NULL)//指针检测
    {
        acoral_enter_critical();
        acoral_release_res((acoral_res_t *)thread);
        acoral_exit_critical();
        acoral_printerr("No thread policy support:%d\n",thread->policy);
        return KR_POLICY_ERR_NULL;
    }
    return policy_ctrl->policy_thread_init(thread,route,args,p_data,data);//进入具体策略的线程初始化函数
}

/**
 * @brief 根据策略创建线程
 * 
 * @param route 线程运行函数
 * @param stack_size 线程栈大小
 * @param args 传递参数
 * @param name 线程名字
 * @param stack 线程栈底
 * @param policy_type 策略类型
 * @param data 
 * @return acoral_id 
 */
acoral_id create_thread_by_policy(void (*route)(void *args),acoral_u32 stack_size,void *args,acoral_char *name,void *stack,acoral_u32 policy_type,void *p_data,void *data, acoral_thread_hook_t *hook)
{
#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
    acoral_id ret_id;
#endif
    acoral_thread_t *thread;
    thread=acoral_alloc_thread();//分配tcb内存块
    if(NULL==thread)//分配失败
    {
        acoral_printerr("Alloc thread:%s fail\n",name);
        return KR_POLICY_ERR_THREAD;
    }
    thread->name=name;//线程名字
    stack_size=stack_size&(~3);//四字节对齐
    thread->stack_size=stack_size;//设置栈大小
    if(stack!=NULL)
        thread->stack_buttom=(acoral_u32 *)stack;//设置栈底
    else
        thread->stack_buttom=NULL;
    thread->policy=policy_type;//线程调度策略
    if(hook!=NULL)
    {
        thread->hook.deal_hook = hook->deal_hook;
        thread->hook.release_hook = hook->release_hook;
    }
    else
    {
        thread->hook.deal_hook = NULL;
        thread->hook.release_hook = NULL;
    }
#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
    ret_id = acoral_policy_thread_init(policy_type,thread,route,args,p_data,data);//基于策略的线程初始化函数
    acoral_tinfos_add(thread);
    return ret_id;
#else
    return acoral_policy_thread_init(policy_type,thread,route,args,p_data,data);//基于策略的线程初始化函数
#endif

}

/**
 * @brief 注册策略函数
 * 
 * @param policy 
 */
void acoral_register_sched_policy(acoral_sched_policy_t *policy)
{
    acoral_fifo_queue_add(&policy_queue, &policy->list);//fifo方式添加
}
/**
 * @brief 轮询策略时间处理函数
 * 
 */
void acoral_policy_time_deal(void)
{
    acoral_list_t   *tmp,*head;
    acoral_sched_policy_t  *policy_ctrl;
    head=&policy_queue.head;
    tmp=head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询策略节点
    {
        policy_ctrl=list_entry(tmp,acoral_sched_policy_t,list);
        if(policy_ctrl->time_deal!=NULL)
            policy_ctrl->time_deal();//进入具体策略的时间处理函数
    }
}
/**
 * @brief 基于策略的回收线程函数
 * 
 * @param thread 
 */
void acoral_policy_thread_release(acoral_thread_t *thread)
{
    acoral_sched_policy_t   *policy_ctrl;
    policy_ctrl=acoral_get_policy_ctrl(thread->policy);
    if(policy_ctrl->policy_thread_release!=NULL)
        policy_ctrl->policy_thread_release(thread);
}

/**
 * @brief 线程调度策略初始化
 * 
 */
void acoral_sched_policy_init(void)
{
    acoral_fifo_queue_init(&policy_queue);//初始化策略队列
    comm_policy_init();//普通线程策略初始化
#ifdef CFG_THRD_PERIOD
    period_policy_init();//周期线程策略初始化
#endif
    timed_policy_init();
}

