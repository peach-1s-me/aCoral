/**
 * @file admis_ctl.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库准入控制相关头文件
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
#ifndef LIB_ADMIN_CTL_H
#define LIB_ADMIN_CTL_H
#include <type.h>
#include <queue.h>
#include <ipc.h>

/**
 * @brief 线程资源表结构
 * 
 */
typedef struct{
    acoral_list_t list;///<链表
    acoral_u8 critical_cnt;///<资源临界区次数（对每个线程）
    acoral_u32 length_max;///<资源最大临界区长度（对每个线程）
    acoral_dpcp_t **dpcp;///<dpcp资源结构体指针的指针
}acoral_admis_res_data;

/**
 * @brief 准入控制数据表结构
 * 
 */
typedef struct{
    acoral_list_t list;///<链表
    acoral_u32 cpu;///<所在cpu
    acoral_u32 dl_time;///<截止时间
    acoral_u32 wcet;///<最坏执行时间
    acoral_u32 wcrt;///<最坏响应时间
    acoral_u32 period_time;///<周期时间
    void (*route)(void *args);///<线程运行函数
    void *args;///<传递参数
    acoral_u32 stack_size;///<线程栈大小
    acoral_char *name;///<线程名字
    void *stack;///<线程栈底
    acoral_queue_t res_queue;///<线程所用资源队列
    acoral_u8 prio;///<优先级（可能会被改动）
    acoral_u8 prio_origin;///<修改前优先级
    acoral_u8 is_new;///<新加线程标志
    acoral_id task_id;///<线程id
}acoral_admis_ctl_data;

extern acoral_u8 acoral_have_new_thread;

void acoral_admis_ctl_change_prio(acoral_admis_ctl_data *ctl_data);
void acoral_admis_ctl_change_res(acoral_admis_res_data *res_data);
void acoral_admis_ctl_data_init(acoral_admis_ctl_data *data, acoral_u32 dl_time, acoral_u32 wcet, acoral_u32 period_time,
        void (*route)(void *args), acoral_u32 stack_size, void *args, acoral_char *name, void *stack);
void acoral_admis_res_data_init(acoral_admis_ctl_data *ctl_data, acoral_admis_res_data *res_data, acoral_dpcp_t **dpcp, acoral_u8 critical_cnt, acoral_u32 length_max);
void acoral_admis_ctl_init(acoral_u32 cpu, acoral_u32 max_time_min);
#endif
