/**
 * @file policy.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程策略相关头文件
 * @version 1.0
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-14 <td>增加注释
 */
#ifndef KERNEL_POLICY_H
#define KERNEL_POLICY_H
#include <thread.h>
///policy类型：基底
#define ACORAL_SCHED_POLICY_BASE 20
///policy类型：普通线程策略
#define ACORAL_SCHED_POLICY_COMM ACORAL_SCHED_POLICY_BASE+1
///policy类型：周期线程策略
#define ACORAL_SCHED_POLICY_PERIOD ACORAL_SCHED_POLICY_BASE+2
///policy类型：时间确定性线程策略
#define ACORAL_SCHED_POLICY_TIMED ACORAL_SCHED_POLICY_BASE+3
/**
 * @brief 调度策略结构体
 * 
 */
typedef struct{
    acoral_list_t list;///<链表节点
    acoral_u8 type;///<策略类型
    acoral_id (*policy_thread_init)(acoral_thread_t *,void (*route)(void *args),void *,void *,void *);///<策略线程初始化函数
    void (*policy_thread_release)(acoral_thread_t *);///<策略回收释放函数
    void (*time_deal)();///<策略时间处理函数
    acoral_char *name;///<策略名称
}acoral_sched_policy_t;

acoral_id create_thread_by_policy(void (*route)(void *args),acoral_u32 stack_size,void *args,acoral_char *name,void *stack,acoral_u32 sched_policy,void *p_data, void *data, acoral_thread_hook_t *hook);
void acoral_register_sched_policy(acoral_sched_policy_t *policy);
void acoral_policy_time_deal(void);
void acoral_policy_thread_release(acoral_thread_t *thread);
void acoral_sched_policy_init(void);
#endif
