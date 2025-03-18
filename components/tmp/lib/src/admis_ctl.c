/**
 * @file admis_ctl.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库准入控制相关源文件
 * @version 1.0
 * @date 2022-11-11
 *
 * @copyright Copyright (c) 2022
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-11-11 <td>增加注释
 */
#include <admis_ctl.h>
#include <mem.h>
#include <timed_thrd.h>
#include <period_thrd.h>
#include <policy.h>
#include <print.h>
#include <cpu.h>
#include <ipi.h>
#include <str.h>
#include <math.h>

/**
 * @brief 资源数据缓存
 *
 */
typedef struct{
    acoral_u8 cpu_is_bind;///<资源绑定cpu标志
    acoral_u8 ceiling_cache;///<优先级天花板缓存
    acoral_u8 type_cache;///<资源类型缓存
    acoral_u8 bind_cnt;///<绑定线程数量
}admis_res_cache;

extern acoral_queue_t acoral_timed_time[CFG_MAX_CPU];
extern acoral_u32 acoral_timed_h_period[CFG_MAX_CPU];

///准入控制线程栈大小
#define ADMIS_CTL_STACK_SIZE TIMED_STACK_SIZE*20
///准入控制线程执行时间
#define CA 50
///第一个新加线程优先级
#define FIRST_PRIO (ACORAL_MINI_PRIO + ACORAL_MAX_PRIO + 1)/2
///cpu负载：之前状态
#define CPU_LOAD_LAST 0
///cpu负载：现在状态
#define CPU_LOAD_THIS 1
///准入控制线程id
acoral_id admis_ctl_task_id;
///准入控制数据表队列
acoral_queue_t acoral_admis_ctl_queue;
///系统新加线程状态位
acoral_u8 acoral_have_new_thread = 0;
///cpu负载
static acoral_fl cpu_load[2][CFG_MAX_CPU];
///cpu负载列表（从小到大）
static acoral_u8 cpu_load_list[CFG_MAX_CPU];
///周期任务数量
static acoral_u8 task_cnt = 0;
///时间确定性任务总执行时间（一个超周期内）
static acoral_u32 timed_time_sum[CFG_MAX_CPU];


/**
 * @brief 准入控制改变线程优先级接口
 *
 * @param ctl_data 准入控制数据表指针
 */
void acoral_admis_ctl_change_prio(acoral_admis_ctl_data *ctl_data)
{
    acoral_thread_change_prio_by_id(ctl_data->task_id, ctl_data->prio);//修改优先级

}

/**
 * @brief 准入控制改变资源状态接口
 *
 * @param res_data 线程资源表指针
 */
void acoral_admis_ctl_change_res(acoral_admis_res_data *res_data)
{
    admis_res_cache *res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
    acoral_dpcp_set((*res_data->dpcp), res_cache->type_cache, res_cache->ceiling_cache);//修改资源属性
}

/**
 * @brief 更新cpu负载列表（排序）
 *
 */
static void cpu_load_list_update(void)
{
    acoral_fl cpu_load_tmp[CFG_MAX_CPU];
    acoral_fl tmp_fl;
    acoral_u8 tmp_u8;
    acoral_memcpy(cpu_load_tmp, cpu_load[CPU_LOAD_THIS], sizeof(acoral_fl)*CFG_MAX_CPU);
    for(int i=0;i<CFG_MAX_CPU;i++)//计算表调度任务的负载
    {
        cpu_load_list[i] = i;
    }
    for(int i=0;i<CFG_MAX_CPU;i++)
    {
        for(int j=1;(i+j)<CFG_MAX_CPU;j++)
        {
            if(cpu_load_tmp[i]>cpu_load_tmp[i+j])
            {
                tmp_fl = cpu_load_tmp[i];
                tmp_u8 = cpu_load_list[i];
                cpu_load_tmp[i] = cpu_load_tmp[i+j];
                cpu_load_list[i] = cpu_load_list[i+j];
                cpu_load_tmp[i+j] = tmp_fl;
                cpu_load_list[i+j] = tmp_u8;
            }
        }
    }
}

/**
 * @brief 实时任务中全局资源临界区的资源利用率
 *
 * @param dpcp dpcp资源结构体指针的指针
 * @return acoral_fl 资源利用率
 */
static acoral_fl u_g_time(acoral_dpcp_t **dpcp)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    acoral_fl ret = 0;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->prev;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->prev)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        res_head = &ctl_data->res_queue.head;
        for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
        {
            res_data = list_entry(res_tmp, acoral_admis_res_data, list);
            if(dpcp==res_data->dpcp)
            {
                ret += (acoral_fl)(res_data->critical_cnt*res_data->length_max)/(acoral_fl)ctl_data->period_time;
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief 局部资源延迟
 *
 * @param data 准入控制数据表指针
 * @return acoral_u32 延迟时间
 */
static acoral_u32 latency_b_time(acoral_admis_ctl_data *data)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    acoral_u8 global_n = 0;
    acoral_u32 local_l_max = 0;
    res_head = &data->res_queue.head;
    for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
    {
        res_data = list_entry(res_tmp, acoral_admis_res_data, list);
        res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
        if(res_cache->type_cache==ACORAL_DPCP_GLOBAL)//轮询全局资源，计算获取全局资源次数
            global_n += res_data->critical_cnt;
    }
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->prev;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->prev)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        if(ctl_data->cpu==data->cpu&&ctl_data->prio>data->prio)
        {
            res_head = &ctl_data->res_queue.head;
            for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
            {
                res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                if(res_cache->type_cache==ACORAL_DPCP_LOCAL&&res_cache->ceiling_cache<data->prio)
                {
                    if(res_data->length_max>local_l_max)//计算此核局部资源使用最长临界区
                        local_l_max = res_data->length_max;
                }
            }
        }
        else if(ctl_data->prio<=data->prio)
            break;
    }
    return (global_n+1)*local_l_max;
}

/**
 * @brief 非全局资源临界区延迟
 *
 * @param t 迭代延迟
 * @param data 准入控制数据表指针
 * @return acoral_u32 延迟时间
 */
static acoral_u32 latency_ng_time(acoral_u32 t, acoral_admis_ctl_data *data)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    acoral_u32 c_ng = 0;
    acoral_u32 ret = 0;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        if(ctl_data->cpu==data->cpu&&ctl_data->prio<data->prio)
        {
            c_ng = ctl_data->wcet;
            res_head = &ctl_data->res_queue.head;
            for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
            {
                res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                if(res_cache->type_cache == ACORAL_DPCP_GLOBAL)
                {
                    c_ng -= (res_data->critical_cnt*res_data->length_max);
                }
            }
            ret += ceil((acoral_fl)(t + ctl_data->wcrt - c_ng)/(acoral_fl)ctl_data->period_time)*c_ng;
        }
        else if(ctl_data->prio>=data->prio)
            break;
    }
    return ret;
}

/**
 * @brief
 *
 * @param t
 * @param cpu
 * @return acoral_u32
 */
static acoral_u32 epsilon(acoral_u32 t, acoral_u32 cpu)
{
    acoral_list_t *head,*tmp;
    acoral_u32 ret = 0;
    head = &acoral_timed_time[cpu].head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next->next)
    {
        if(tmp->value<t)
            ret += tmp->next->value;
        else
            break;
    }
    return ret;
}
/**
 * @brief 时间确定性任务抢占延迟
 *
 * @param t 迭代延迟
 * @param cpu 所在cpu
 * @return acoral_u32 延迟时间
 */
static acoral_u32 latency_t_time(acoral_u32 t, acoral_u32 cpu)
{
    acoral_u32 latency_t_max = 0,latency_t_time = 0;
    acoral_list_t *head,*tmp;
    head = &acoral_timed_time[cpu].head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next->next)
    {
        latency_t_time = timed_time_sum[cpu]*floor((acoral_fl)(tmp->value+t)/(acoral_fl)acoral_timed_h_period[cpu])
                + epsilon((tmp->value+t)%acoral_timed_h_period[cpu], cpu) - epsilon(tmp->value, cpu);
        if(latency_t_time>latency_t_max)
            latency_t_max = latency_t_time;
    }
    return latency_t_max;
}

/**
 * @brief 单次请求单个全局资源的延迟
 *
 * @param start 迭代开始延迟
 * @param cpu 所在cpu
 * @param data 准入控制数据表指针
 * @return acoral_u32 延迟时间
 */
acoral_u32 h_g_time(acoral_u32 start, acoral_u32 cpu, acoral_admis_ctl_data *data)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp,*head,*tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    acoral_dpcp_t *dpcp_tmp;
    acoral_u32 test_end = data->dl_time;
    acoral_u32 h_g = start;
    acoral_u32 h_g_temp = start;
    acoral_u32 NL_higher = 0;
    ctl_head = &acoral_admis_ctl_queue.head;
    head = &acoral_dpcp_queue[ACORAL_DPCP_GLOBAL][cpu].head;
    while(h_g <= test_end)
    {
        h_g_temp = start;
        for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询该核全局资源
        {
            dpcp_tmp = list_entry(tmp, acoral_dpcp_t, list);
            for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)//轮询任务
            {
                ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
                if(ctl_data->prio<data->prio)
                {
                    res_head = &ctl_data->res_queue.head;
                    for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                    {
                        res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                        if((*res_data->dpcp)==dpcp_tmp)
                        {
                            NL_higher = res_data->critical_cnt*res_data->length_max;
                            h_g_temp += ceil((acoral_fl)(h_g + ctl_data->wcrt - NL_higher)/(acoral_fl)ctl_data->period_time)*NL_higher;
                        }
                    }
                }
                else
                    break;
            }
        }
        h_g_temp += latency_t_time(h_g, cpu);
        if(h_g==h_g_temp)
        {
            return h_g;
        }
        else
            h_g = h_g_temp;
    }
    return test_end + 100;
}
/**
 * @brief 请求全局资源的延迟
 *
 * @param t 迭代延迟
 * @param data 准入控制数据表指针
 * @return acoral_u32 延迟时间
 */
acoral_u32 latency_g_time(acoral_u32 t, acoral_admis_ctl_data *data)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp,*head,*tmp,*tmp1;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    acoral_dpcp_t *dpcp_tmp,*dpcp_tmp1;
    acoral_u8 flag = 0;
    acoral_u32 NL_other = 0,N_this = 0,L_this = 0;
    acoral_u32 PD_g = 0,DD_g = 0;
    acoral_u32 h_g_bound = 0;
    acoral_u32 global_l_max = 0;
    acoral_u32 ret = 0;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(int i=0;i<CFG_MAX_CPU;i++)
    {
        head = &acoral_dpcp_queue[ACORAL_DPCP_GLOBAL][i].head;
        if(data->cpu==i)
        {
            for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询该核全局资源
            {
                dpcp_tmp = list_entry(tmp, acoral_dpcp_t, list);
                for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)//轮询任务
                {
                    ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
                    if(ctl_data!=data)
                    {
                        res_head = &ctl_data->res_queue.head;
                        for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                        {
                            res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                            if((*res_data->dpcp)==dpcp_tmp)//找到使用该核全局资源的任务
                            {
                                NL_other = res_data->critical_cnt*res_data->length_max;
                                ret += ceil((acoral_fl)(t + ctl_data->wcrt - NL_other)/(acoral_fl)ctl_data->period_time)*NL_other;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询该核全局资源
            {
                dpcp_tmp = list_entry(tmp, acoral_dpcp_t, list);
                res_head = &data->res_queue.head;
                for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                {
                    res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                    if((*res_data->dpcp)==dpcp_tmp)//查看此任务是否使用该核全局资源
                    {
                        flag = 1;
                    }
                }
            }
            if(flag==0)//此任务不会使用该核全局资源
                continue;
            for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询该核全局资源
            {
                dpcp_tmp = list_entry(tmp, acoral_dpcp_t, list);
                for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)
                {
                    ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
                    if(ctl_data==data)//此任务
                    {
                        res_head = &ctl_data->res_queue.head;
                        for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                        {
                            res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                            if((*res_data->dpcp)==dpcp_tmp)//查看此任务是否使用该核全局资源
                            {
                                PD_g += res_data->critical_cnt*res_data->length_max;//NL_i
                            }
                        }
                    }
                    else//其他任务
                    {
                        res_head = &ctl_data->res_queue.head;
                        for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                        {
                            res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                            if((*res_data->dpcp)==dpcp_tmp)//找到使用该核全局资源的任务
                            {
                                NL_other = res_data->critical_cnt*res_data->length_max;//NL_other
                                PD_g += ceil((acoral_fl)(t + ctl_data->wcrt - NL_other)/(acoral_fl)ctl_data->period_time)*NL_other;
                            }
                        }
                    }
                }
            }
            PD_g += latency_t_time(t, i);//PD_g计算完成
            for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询该核全局资源
            {
                dpcp_tmp = list_entry(tmp, acoral_dpcp_t, list);
                res_head = &data->res_queue.head;
                for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                {
                    res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                    if((*res_data->dpcp)==dpcp_tmp)//查看此任务是否使用该核全局资源
                    {
                        N_this = res_data->critical_cnt;
                        L_this = res_data->length_max;
                    }
                }
                for(tmp1=head->next;tmp1!=head;tmp1=tmp1->next)//轮询该核全局资源
                {
                    dpcp_tmp1 = list_entry(tmp1, acoral_dpcp_t, list);
                    if(dpcp_tmp1->cpu==dpcp_tmp->cpu)
                    {
                        for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)
                        {
                            ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
                            if(ctl_data->prio>data->prio)
                            {
                                res_head = &ctl_data->res_queue.head;
                                for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
                                {
                                    res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                                    res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                                    if((*res_data->dpcp)==dpcp_tmp1&&res_cache->ceiling_cache<data->prio)//查看此任务是否使用该核全局资源
                                    {
                                        if(res_data->length_max>global_l_max)
                                            global_l_max = res_data->length_max;
                                    }
                                }
                            }
                        }
                    }
                }
                h_g_bound = h_g_time(L_this + global_l_max, i, data);
                if(h_g_bound<=data->dl_time)
                {
                    DD_g += N_this*h_g_bound;
                }
                else
                    return data->dl_time + 100;
            }
            if(PD_g<DD_g)
                ret += PD_g;
            else
                ret += DD_g;
        }
    }
    return ret;
}

/**
 * @brief 总延迟时间迭代
 *
 * @param data 准入控制数据表指针
 * @return acoral_u32 总延迟时间
 */
static acoral_u32 response_time(acoral_admis_ctl_data *data)
{
    acoral_u32 test_end = data->dl_time;
    acoral_u32 response = data->wcet;
    acoral_u32 response_temp = data->wcet;
    acoral_u32 latency_b = 0, latency_ng = 0, latency_t = 0, latency_g = 0;
    while(response <= test_end)
    {
        response_temp = data->wcet;

        latency_ng = latency_ng_time(response, data);
        if(latency_ng > test_end)
            return test_end + 100;
        else
            response_temp += latency_ng;
        latency_t = latency_t_time(response, data->cpu);
        if(latency_t > test_end)
            return test_end + 100;
        else
            response_temp += latency_t;
        latency_g = latency_g_time(response, data);
        if(latency_g > test_end)
            return test_end + 100;
        else
            response_temp += latency_g;
        if(response==response_temp)
        {
            latency_b = latency_b_time(data);
            if(latency_b > test_end)
                return test_end + 100;
            else
                return response + latency_b;
        }
        else
            response = response_temp;
    }
    return test_end + 100;
}

/**
 * @brief 可调度性分析
 *
 * @return acoral_bool 是否可行
 */
static acoral_bool schedule_ansys()
{
    acoral_u32 response_bound;
    acoral_list_t *ctl_head,*ctl_tmp;
    acoral_admis_ctl_data *ctl_data;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->next;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->next)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        if(ctl_data->is_new!=1)
        {
            response_bound = response_time(ctl_data);
            if(response_bound<=ctl_data->dl_time)
                ctl_data->wcrt = response_bound;
            else
                return false;
        }
    }
    return true;
}
/**
 * @brief 根据准入控制数据表创建线程
 *
 */
static void admis_ctl_create(void)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    acoral_period_policy_data_t period_policy_data;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->prev;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->prev)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        if(ctl_data->prio!=ctl_data->prio_origin)//修改线程与资源的信息，包括新加线程的
        {
            ctl_data->prio_origin = ctl_data->prio;
            if(ctl_data->is_new)//新加的线程可以正式创建了
            {
                period_policy_data.cpu = ctl_data->cpu;
                period_policy_data.prio = ctl_data->prio;
                period_policy_data.time = ctl_data->period_time;
                ctl_data->task_id = acoral_create_thread(ctl_data->route,
                                                        ctl_data->stack_size,
                                                        ctl_data->args,
                                                        ctl_data->name,
                                                        ctl_data->stack,
                                                        ACORAL_SCHED_POLICY_PERIOD,
                                                        &period_policy_data,
                                                        NULL,
                                                        NULL);
                ctl_data->is_new = 0;
            }
            else//剩余线程需要对TCB做更改
            {
                if(ctl_data->cpu == acoral_current_cpu)//本核的线程直接更改
                {
                    acoral_admis_ctl_change_prio(ctl_data);
                }
                else//不是本核的线程就调用核间中断来更改
                {
#ifdef CFG_SMP
                    acoral_ipi_cmd_send(ctl_data->cpu, ACORAL_IPI_ADMIS_CHG_PRIO, ctl_data->task_id, ctl_data);//ipi中断
#endif
                }
            }
            res_head = &ctl_data->res_queue.head;
            for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)
            {
                res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                if(res_cache->cpu_is_bind==0)
                    res_cache->cpu_is_bind = 1;
                if((res_cache->ceiling_cache!=(*res_data->dpcp)->prio_ceiling)||(res_cache->type_cache!=(*res_data->dpcp)->type))
                {
                    if((*res_data->dpcp)->cpu == acoral_current_cpu)//本核的资源直接更改
                    {
                        acoral_admis_ctl_change_res(res_data);
                    }
                    else//不是本核的资源就调用核间中断来更改
                    {
#ifdef CFG_SMP
                        acoral_ipi_cmd_send((*res_data->dpcp)->cpu, ACORAL_IPI_ADMIS_CHG_RES, ctl_data->task_id, res_data);//ipi中断
#endif
                    }
                }
            }
        }
    }
    for(int i=0;i<CFG_MAX_CPU;i++)
    {
        if(cpu_load[CPU_LOAD_LAST][i]!=cpu_load[CPU_LOAD_THIS][i])
            cpu_load[CPU_LOAD_LAST][i] = cpu_load[CPU_LOAD_THIS][i];
    }
}
/**
 * @brief 回退准入控制数据表内容
 *
 */
static void admis_ctl_restore(void)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp;
    acoral_admis_ctl_data *ctl_data;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    ctl_head = &acoral_admis_ctl_queue.head;
    for(ctl_tmp=ctl_head->prev;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->prev)
    {
        ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
        if(ctl_data->prio!=ctl_data->prio_origin)
        {
            ctl_data->prio = ctl_data->prio_origin;//修改回原本优先级
            res_head = &ctl_data->res_queue.head;
            for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)//轮询资源，修改cache变量为原本状态
            {
                res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                if(res_cache->cpu_is_bind==0)
                {
                    acoral_lifo_queue_del(&acoral_dpcp_queue[ACORAL_DPCP_LOCAL][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                    (*res_data->dpcp)->cpu = UNBIND_CPU;
                }
                if(res_cache->bind_cnt>0)
                {
                    if(res_cache->ceiling_cache!=(*res_data->dpcp)->prio_ceiling)
                    {
                        res_cache->ceiling_cache = (*res_data->dpcp)->prio_ceiling;
                    }
                    if(res_cache->type_cache!=(*res_data->dpcp)->type)
                    {
                        acoral_lifo_queue_del(&acoral_dpcp_queue[res_cache->type_cache][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                        res_cache->type_cache = (*res_data->dpcp)->type;
                        acoral_lifo_queue_add(&acoral_dpcp_queue[res_cache->type_cache][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                    }
                }
            }
        }
    }
    for(int i=0;i<CFG_MAX_CPU;i++)
    {
        if(cpu_load[CPU_LOAD_LAST][i]!=cpu_load[CPU_LOAD_THIS][i])
            cpu_load[CPU_LOAD_THIS][i] = cpu_load[CPU_LOAD_LAST][i];
    }
}
/**
 * @brief 准入控制线程
 *
 * @param args 传递参数
 */
void acoral_admis_ctl_task(void *args)
{
    acoral_list_t *ctl_head,*res_head,*ctl_tmp,*res_tmp,*tmp1;
    acoral_admis_ctl_data *ctl_data,ctl_data_tmp;
    acoral_admis_ctl_data *ctl_data_prev = NULL,*ctl_data_next = NULL;
    acoral_admis_res_data *res_data;
    admis_res_cache *res_cache;
    acoral_bool admis_ok = false;
    static acoral_u8 first_thread = 1,this_is_first = 0;
#ifdef CFG_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("admis_ctrl_task is running in cpu%d!!!\r\n", acoral_current_cpu);
#endif
    if(acoral_have_new_thread)
    {
#ifdef CFG_ADMIS_CTRL_PRINT_ENABLE
        acoral_print("have new thread!!!\r\n");
#endif
        ctl_head = &acoral_admis_ctl_queue.head;
        for(ctl_tmp=ctl_head->prev;ctl_tmp!=ctl_head;ctl_tmp=ctl_tmp->prev)
        {
            ctl_data = list_entry(ctl_tmp, acoral_admis_ctl_data, list);
            if(ctl_data->is_new)
            {
                task_cnt++;
                cpu_load_list_update();
                for(int i=0;i<CFG_MAX_CPU;i++)
                {
                    ctl_data->cpu = cpu_load_list[i];//分配cpu
                    cpu_load[CPU_LOAD_THIS][ctl_data->cpu] += (acoral_fl)ctl_data->wcet/(acoral_fl)ctl_data->period_time;//+Ca/Ta
                    if(first_thread)
                    {
                        ctl_data->prio = FIRST_PRIO;
                        this_is_first = 1;
                        first_thread = 0;
                    }
                    else
                    {
                        if(ctl_tmp->prev==ctl_head)
                        {
                            ctl_data_prev = &ctl_data_tmp;
                            ctl_data_tmp.prio = ACORAL_MAX_PRIO + 1;
                            ctl_data_next = list_entry(ctl_tmp->next, acoral_admis_ctl_data, list);
                        }
                        else if(ctl_tmp->next==ctl_head)
                        {
                            ctl_data_next = &ctl_data_tmp;
                            ctl_data_prev = list_entry(ctl_tmp->prev, acoral_admis_ctl_data, list);
                            ctl_data_tmp.prio = ACORAL_MINI_PRIO;
                        }
                        else
                        {
                            ctl_data_prev = list_entry(ctl_tmp->prev, acoral_admis_ctl_data, list);
                            ctl_data_next = list_entry(ctl_tmp->next, acoral_admis_ctl_data, list);
                        }
                        ctl_data->prio = (ctl_data_prev->prio + ctl_data_next->prio)/2;
                        if(ctl_data->prio==ctl_data_prev->prio)//优先级重合了
                        {
                        }
                    }
                    res_head = &ctl_data->res_queue.head;
                    for(res_tmp=res_head->next;res_tmp!=res_head;res_tmp=res_tmp->next)//轮询线程所用资源，用cache变量暂存更改
                    {
                        res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                        res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                        if((*res_data->dpcp)->cpu == UNBIND_CPU)
                        {
                            (*res_data->dpcp)->cpu = ctl_data->cpu;
                            acoral_lifo_queue_add(&acoral_dpcp_queue[ACORAL_DPCP_LOCAL][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                        }
                        else if(ctl_data->cpu!=(*res_data->dpcp)->cpu)
                        {
                            res_cache->type_cache = ACORAL_DPCP_GLOBAL;
                            cpu_load[CPU_LOAD_THIS][(*res_data->dpcp)->cpu] += u_g_time(res_data->dpcp);
                            acoral_lifo_queue_del(&acoral_dpcp_queue[ACORAL_DPCP_LOCAL][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                            acoral_lifo_queue_add(&acoral_dpcp_queue[ACORAL_DPCP_GLOBAL][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                        }
                        if(ctl_data->prio<res_cache->ceiling_cache)
                            res_cache->ceiling_cache = ctl_data->prio;
                        if(res_cache->type_cache == ACORAL_DPCP_GLOBAL)
                        {
                            cpu_load[CPU_LOAD_THIS][ctl_data->cpu] -= (acoral_fl)(res_data->critical_cnt*res_data->length_max)/(acoral_fl)ctl_data->period_time;
                        }
                    }
                    ctl_data->is_new = 0;
                    admis_ok = schedule_ansys();
                    ctl_data->is_new = 1;
                    if(admis_ok)//可行
                    {
                        this_is_first = 0;
                        admis_ctl_create();
#ifdef CFG_ADMIS_CTRL_PRINT_ENABLE
                        acoral_print("%s can be added to cpu%d\r\n", ctl_data->name, ctl_data->cpu);
#endif
                        break;
                    }
                    else
                    {
                        if(this_is_first)
                        {
                            this_is_first = 0;
                            first_thread = 1;
                        }
#ifdef CFG_ADMIS_CTRL_PRINT_ENABLE
                        acoral_print("%s can't be added to cpu%d\r\n", ctl_data->name, ctl_data->cpu);
#endif
                        admis_ctl_restore();
                    }
                }
                if(ctl_data->is_new)//删除新加任务
                {
                    task_cnt--;
                    res_head = &ctl_data->res_queue.head;
                    for(res_tmp=res_head->next;res_tmp!=res_head;)//轮询资源，回收内存
                    {
                        res_data = list_entry(res_tmp, acoral_admis_res_data, list);
                        res_cache = (admis_res_cache *)(*res_data->dpcp)->data;
                        if(res_cache->bind_cnt>0)
                            res_cache->bind_cnt--;
                        if(res_cache->bind_cnt==0)
                        {
                            if(res_cache->cpu_is_bind==1)
                            {
                                acoral_lifo_queue_del(&acoral_dpcp_queue[ACORAL_DPCP_LOCAL][(*res_data->dpcp)->cpu], &(*res_data->dpcp)->list);
                                (*res_data->dpcp)->cpu = UNBIND_CPU;
                            }
                            acoral_dpcp_del((*res_data->dpcp));
                            *res_data->dpcp = NULL;
                        }
                        tmp1 = res_tmp->next;
                        acoral_lifo_queue_del(&ctl_data->res_queue, res_tmp);
                        acoral_free(res_data);
                        res_tmp = tmp1;
                    }
                    tmp1 = ctl_tmp->next;//上一个链表节点，在for循环内会变成下一个
                    acoral_prio_queue_del(&acoral_admis_ctl_queue, &ctl_data->list);
                    acoral_free(ctl_data);
                    ctl_data = NULL;
                    ctl_tmp = tmp1;
                }
            }
        }
        acoral_have_new_thread = 0;
    }
}

/**
 * @brief 初始化准入控制数据表
 *
 * @param data 准入控制数据表指针
 * @param dl_time 截止时间
 * @param wcet 最坏执行时间
 * @param period_time 周期时间
 * @param route 线程运行函数
 * @param stack_size 线程栈大小
 * @param args 传递参数
 * @param name 线程名字
 * @param stack 线程栈底
 */
void acoral_admis_ctl_data_init(acoral_admis_ctl_data *data, acoral_u32 dl_time, acoral_u32 wcet, acoral_u32 period_time,
        void (*route)(void *args), acoral_u32 stack_size, void *args, acoral_char *name, void *stack)
{
    data->dl_time = dl_time;
    data->is_new = 1;
    data->period_time = period_time;
    data->wcet = wcet;
    data->wcrt = dl_time;
    data->prio = ACORAL_MINI_PRIO;
    data->prio_origin = ACORAL_MINI_PRIO;
    data->route = route;
    data->stack_size = stack_size;
    data->args = args;
    data->name = name;
    data->stack = stack;
    acoral_vlist_init(&data->list, dl_time);
    acoral_lifo_queue_init(&data->res_queue);
    acoral_prio_queue_add(&acoral_admis_ctl_queue, &data->list);
}

/**
 * @brief 线程资源表结构
 *
 * @param ctl_data 准入控制数据表指针
 * @param res_data 线程资源表指针
 * @param dpcp dpcp资源结构体指针的指针
 * @param critical_cnt 资源临界区次数（对每个线程）
 * @param length_max 资源最大临界区长度（对每个线程）
 */
void acoral_admis_res_data_init(acoral_admis_ctl_data *ctl_data, acoral_admis_res_data *res_data, acoral_dpcp_t **dpcp, acoral_u8 critical_cnt, acoral_u32 length_max)
{

    admis_res_cache *res_cache = NULL;
    if((*dpcp)==NULL)
    {
        (*dpcp) = acoral_dpcp_create();
        res_cache = acoral_vol_malloc(sizeof(admis_res_cache));
        res_cache->cpu_is_bind = 0;
        res_cache->ceiling_cache = (*dpcp)->prio_ceiling;
        res_cache->type_cache = (*dpcp)->type;
        res_cache->bind_cnt = 0;
        (*dpcp)->data = (void *)res_cache;
    }
    res_cache = (*dpcp)->data;
    res_data->dpcp = dpcp;
    res_data->critical_cnt = critical_cnt;
    res_data->length_max = length_max;
    res_cache->bind_cnt++;
    acoral_list_init(&res_data->list);
    acoral_lifo_queue_add(&ctl_data->res_queue, &res_data->list);
}
/**
 * @brief 准入控制流程初始化
 *
 * @param cpu 所在cpu
 * @param max_time_min 时间确定性线程最大执行时间
 */
void acoral_admis_ctl_init(acoral_u32 cpu, acoral_u32 max_time_min)
{
    acoral_timed_policy_data_t timed_policy_data;
    acoral_queue_t start_time_queue;
    acoral_list_t *start_time_list;
    acoral_list_t *time_list;
    acoral_list_t *head,*tmp,*tmp1;
    acoral_u16 start_time_cnt = 0;
    acoral_u32 interval = 2*max_time_min + 3*CA;
    acoral_u8 flag = 1;
    int k=0;//for循环使用
    acoral_prio_queue_init(&acoral_admis_ctl_queue);
    acoral_prio_queue_init(&start_time_queue);

    timed_policy_data.cpu = cpu;
    timed_policy_data.section_num = 1;
    timed_policy_data.exe_time = acoral_vol_malloc(4);//提前分配内存
    timed_policy_data.exe_time[0] = CA;
    timed_policy_data.section_route = acoral_vol_malloc(4);//提前分配内存
    timed_policy_data.section_args = acoral_vol_malloc(4);//提前分配内存
    timed_policy_data.section_route[0] = acoral_admis_ctl_task;
    timed_policy_data.section_args[0] = NULL;

    head = &acoral_timed_time[cpu].head;
    if(acoral_list_empty(head))//该核上没有表调度任务
    {
        start_time_cnt++;
        acoral_timed_h_period[cpu] = CA*5;
        start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
        acoral_vlist_init(start_time_list, 0);
        acoral_prio_queue_add(&start_time_queue, start_time_list);
        time_list = acoral_vol_malloc(sizeof(acoral_list_t));
        acoral_vlist_init(time_list, 0);
        acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
        time_list = acoral_vol_malloc(sizeof(acoral_list_t));
        acoral_vlist_init(time_list, CA);
        acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
    }
    else//该核上有表调度任务
    {
        for(tmp=head->next->next;tmp!=head->next;tmp=tmp->next->next)//表任务执行时间后是否有位置
        {
            start_time_cnt++;
            start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(start_time_list, tmp->value);
            acoral_prio_queue_add(&start_time_queue, start_time_list);
            tmp->value += CA;
        }
        for(tmp=head->next;tmp!=head;tmp=tmp->next->next)//表任务执行时间前是否有位置
        {
            if((tmp->value-tmp->prev->value)>=CA)
            {
                start_time_cnt++;
                start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                tmp->value -= CA;
                acoral_vlist_init(start_time_list, tmp->value);
                acoral_prio_queue_add(&start_time_queue, start_time_list);
            }
        }
        if(head->next->value>=CA)//控制周期开始时是否有位置
        {
            start_time_cnt++;
            start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(start_time_list, 0);
            acoral_prio_queue_add(&start_time_queue, start_time_list);
            time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(time_list, 0);
            acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
            time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(time_list, CA);
            acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
        }
        if((acoral_timed_h_period[cpu]-head->prev->value)>=CA)//控制周期结束前是否有位置
        {
            start_time_cnt++;
            start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(start_time_list, acoral_timed_h_period[cpu]-CA);
            acoral_prio_queue_add(&start_time_queue, start_time_list);
            time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(time_list, acoral_timed_h_period[cpu]-CA);
            acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
            time_list = acoral_vol_malloc(sizeof(acoral_list_t));
            acoral_vlist_init(time_list, acoral_timed_h_period[cpu]);
            acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
        }
        head = &start_time_queue.head;
        while(flag)//迭代使得间隔不超过interval
        {
            flag = 0;//flag先设为0
            for(tmp=head->next->next;tmp!=head;tmp=tmp->next)
            {
                if((tmp->value-tmp->prev->value)>interval)
                {
                    flag = 1;//如果有一次循环中不存在间隔超过interval的情况，则flag依旧为0，否则为1
                    start_time_cnt++;
                    start_time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                    acoral_vlist_init(start_time_list, tmp->prev->value+interval);
                    acoral_prio_queue_add(&start_time_queue, start_time_list);
                    time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                    acoral_vlist_init(time_list, tmp->prev->value+interval);
                    acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
                    time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                    acoral_vlist_init(time_list, tmp->prev->value+interval+CA);
                    acoral_prio_queue_add(&acoral_timed_time[cpu], time_list);
                }
            }
        }
    }
    head = &start_time_queue.head;
    timed_policy_data.frequency = start_time_cnt;
    timed_policy_data.start_time = acoral_vol_malloc(4*start_time_cnt*2);
    for(tmp=head->next,k=0;tmp!=head;tmp=tmp->next,k=k+2)
    {
        timed_policy_data.start_time[k] = tmp->value;
        timed_policy_data.start_time[k+1] = tmp->value+CA;
    }
    if(timed_policy_data.start_time[start_time_cnt*2 - 1] != acoral_timed_h_period[cpu])
        timed_policy_data.start_time[start_time_cnt*2 - 1] = acoral_timed_h_period[cpu];
    admis_ctl_task_id = acoral_create_thread(acoral_timed_task,
                                            ADMIS_CTL_STACK_SIZE,
                                            NULL,
                                            "admis_ctl_task",
                                            NULL,
                                            ACORAL_SCHED_POLICY_TIMED,
                                            &timed_policy_data,
                                            NULL,
                                            NULL);
    for(int i=0;i<CFG_MAX_CPU;i++)//整合表调度时间段
    {
        head = &acoral_timed_time[i].head;
        for(tmp=head->next->next;;)
        {
            tmp1 = tmp->next->next;
            if(tmp->next!=head)
            {
                if(tmp->value==tmp->next->value)
                {
                    acoral_prio_queue_del(&acoral_timed_time[cpu], tmp);
                    acoral_prio_queue_del(&acoral_timed_time[cpu], tmp->next);
                }
            }
            else
                break;
            tmp = tmp1;
        }
    }

    for(int i=0;i<CFG_MAX_CPU;i++)//计算表调度任务的负载与可调度性分许用到的数据
    {
        head = &acoral_timed_time[i].head;
        for(tmp=head->next->next;tmp!=head->next;tmp=tmp->next->next)
        {
            tmp->value -= tmp->prev->value;
            timed_time_sum[i] += tmp->value;
        }
        cpu_load[CPU_LOAD_LAST][i] += (acoral_fl)timed_time_sum[i]/(acoral_fl)acoral_timed_h_period[i];
        cpu_load[CPU_LOAD_THIS][i] = cpu_load[CPU_LOAD_LAST][i];
    }
}
