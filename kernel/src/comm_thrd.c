/**
 * @file comm_thrd.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层普通线程策略相关源文件
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
#include <cpu.h>
#include <queue.h>
#include <lsched.h>
#include <hal.h>
#include <policy.h>
#include <comm_thrd.h>
#include <print.h>
#include <error.h>
///普通线程策略结构体实例
acoral_sched_policy_t comm_policy;
/**
 * @brief 普通线程初始化
 * 
 * @param thread 线程结构体指针
 * @param route 线程运行函数
 * @param args 传递参数
 * @param data 数据
 * @return acoral_id 线程id
 */
static acoral_id comm_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *p_data,void *data)
{
    acoral_err err;
    acoral_u8 prio;
    acoral_comm_policy_data_t *policy_data;
    if(thread->policy==ACORAL_SCHED_POLICY_COMM)
    {
        policy_data = (acoral_comm_policy_data_t *)p_data;
        thread->cpu = policy_data->cpu;
        prio = policy_data->prio;
        //设定优先级
        thread->prio = prio;
    }
    if((err = acoral_thread_init(thread,route,acoral_thread_exit,args))!=KR_OK)//通用线程初始化
    {
        acoral_printerr("No thread stack:%s\n",thread->name);
        acoral_enter_critical();
        acoral_release_res((acoral_res_t *)thread);
        acoral_exit_critical();
        return err;
    }
    thread->data = data;
    //将线程就绪，并重新调度
    acoral_rdy_thread(thread);
    return thread->res.id;
}
/**
 * @brief 普通线程策略初始化
 * 
 */
void comm_policy_init(void)
{
    comm_policy.type=ACORAL_SCHED_POLICY_COMM;
    comm_policy.policy_thread_init=comm_policy_thread_init;
    comm_policy.policy_thread_release=NULL;
    comm_policy.time_deal=NULL;
    comm_policy.name="comm";
    acoral_register_sched_policy(&comm_policy);//注册策略
}
