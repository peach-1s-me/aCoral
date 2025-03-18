/**
 * @file dag.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief 并行编程框架头文件
 * @version 1.0
 * @date 2023-09-09
 * 
 * @copyright Copyright (c) 2023
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2023-09-09 <td>增加注释
 */
#ifndef LIB_DAG_H
#define LIB_DAG_H
#include <type.h>

/**
 * @brief 用户节点结构体
 * 
 */
typedef struct{
    acoral_func_point *prev_route;///<前驱节点执行函数
    unsigned int prev_count;///<前驱节点个数
    void (*route)(void *args);///<dag执行函数
    void *args;///<dag执行函数参数
    acoral_list_t list;///<用户节点链表
    acoral_u32 processor;///<所在的处理器
    acoral_u8 prio;///<优先级
}acoral_dag_user_node;

/**
 * @brief 用户dag结构体
 * 
 */
typedef struct{
    acoral_time period_time;///<周期时间
    acoral_queue_t dag_user_node_queue;///<用户节点队列
    acoral_list_t list;///<用户dag链表
}acoral_dag_user;

/**
 * @brief 系统节点结构体
 * 
 */
typedef struct{
    void (*route)(void *args);///<dag执行函数
    void *args;///<dag执行函数参数
    acoral_u16 in_degree;///<入度，即前驱节点个数
    acoral_ipc_t *sem;///<该系统节点信号量
    acoral_list_t list;///<系统节点链表
    acoral_u32 processor;///<所在的处理器
    acoral_u8 prio;///<优先级
}acoral_dag_node;

/**
 * @brief 系统边结构体
 * 
 */
typedef struct{
    acoral_dag_node *prev_node;///<前部系统节点
    acoral_dag_node *next_node;///<后部系统节点
    acoral_list_t list;///<系统边链表
}acoral_dag_edge;

/**
 * @brief 系统dag结构体
 * 
 */
typedef struct{
    acoral_time period_time;///<周期时间
    acoral_queue_t dag_node_queue;///<dag图节点队列
    acoral_queue_t dag_edge_queue;///<dag图边队列
    acoral_list_t list;///<系统dag链表
}acoral_dag;

void dag_user_init();
void dag_decode();
void dag_add_user_node(acoral_dag_user *dag_user, acoral_dag_user_node *user_node);
void dag_add_user(acoral_dag_user *dag_user);
#endif
