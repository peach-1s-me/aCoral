/**
 * @file dag.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief 并行编程框架源文件
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
#include <acoral.h>
///系统dag数据结构队列
acoral_queue_t acoral_dag_queue;
///用户dag数据结构队列
acoral_queue_t acoral_dag_user_queue;

/**
 * @brief dag线程数据结构体
 *
 */
typedef struct{
    acoral_ipc_t **pend_ipc;///<要获取的信号量指针
    acoral_u32 pend_ipc_cnt;///<要获取的信号量个数
    acoral_ipc_t *post_ipc;///<要释放的信号量指针
    void (*dag_route)(void *args);///<dag执行函数
    void *dag_args;///<dag执行函数参数
}dag_task_data_t;

/**
 * @brief 弱定义用户初始化函数
 *
 */
void __weak dag_user_init()
{

}

/**
 * @brief 系统节点初始化
 * 
 * @param dag 该节点属于的系统dag结构体指针
 * @param user_node 该节点所来源的用户节点
 */
static void dag_node_init(acoral_dag *dag, acoral_dag_user_node *user_node)
{
    acoral_dag_node *node;
    node = acoral_malloc(sizeof(acoral_dag_node));
    node->in_degree = user_node->prev_count;
    node->route = user_node->route;
    node->sem = acoral_sem_create(0);
    node->processor = user_node->processor;
    node->prio = user_node->prio;
    acoral_list_init(&node->list);
    acoral_lifo_queue_add(&dag->dag_node_queue, &node->list);
}

/**
 * @brief 一个系统dag的所有系统节点的创建
 * 
 * @param dag 系统dag
 * @param dag_user 用户dag
 */
static void dag_node_create(acoral_dag *dag, acoral_dag_user *dag_user)
{
    acoral_list_t *tmp,*head;
    acoral_dag_user_node *user_node;
    head = &dag_user->dag_user_node_queue.head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//遍历所有用户节点
    {
        user_node = list_entry(tmp,acoral_dag_user_node,list);
        dag_node_init(dag, user_node);
    }
}

/**
 * @brief 系统边初始化
 * 
 * @param dag 该节点属于的系统dag结构体指针
 * @param user_node 该节点所来源的用户节点
 */
static void dag_edge_init(acoral_dag *dag, acoral_dag_user_node *user_node)
{
    acoral_dag_edge *edge;
    acoral_list_t *tmp,*head;
    acoral_dag_node *prev_node,*next_node;
    head = &dag->dag_node_queue.head;
    //找出该user_node匹配的node，作为接下来边的next_node
    for(tmp=head->next;tmp!=head;tmp=tmp->next)
    {
        next_node = list_entry(tmp,acoral_dag_node,list);
        if(next_node->route == user_node->route)
            break;
    }
    //根据入度数量创建边
    for(int i=0;i<next_node->in_degree;i++)
    {
        edge = acoral_malloc(sizeof(acoral_dag_edge));
        edge->next_node = next_node;
        if((void *)user_node->prev_route[i] == NULL)
        {
            prev_node = NULL;
            edge->prev_node = prev_node;
        }
        else
        {
            for(tmp=head->next;tmp!=head;tmp=tmp->next)
            {
                prev_node = list_entry(tmp,acoral_dag_node,list);
                if(prev_node->route == user_node->route)
                    continue;
                if(prev_node->route == (void *)user_node->prev_route[i])
                {
                    edge->prev_node = prev_node;
                    break;
                }
            }
        }
        acoral_list_init(&edge->list);
        acoral_lifo_queue_add(&dag->dag_edge_queue, &edge->list);
    }
}

/**
 * @brief 一个系统dag的所有系统边的创建
 * 
 * @param dag 系统dag
 * @param dag_user 用户dag
 */
static void dag_edge_create(acoral_dag *dag, acoral_dag_user *dag_user)
{
    acoral_list_t *tmp,*head;
    acoral_dag_user_node *user_node;
    head = &dag_user->dag_user_node_queue.head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)
    {
        user_node = list_entry(tmp,acoral_dag_user_node,list);
        dag_edge_init(dag, user_node);
    }
}

/**
 * @brief 系统dag初始化
 * 
 * @param dag_user 来源的用户dag
 */
static void acoral_dag_init(acoral_dag_user *dag_user)
{
    acoral_dag *dag;
    dag = acoral_malloc(sizeof(acoral_dag));
    acoral_lifo_queue_init(&dag->dag_node_queue);
    acoral_lifo_queue_init(&dag->dag_edge_queue);
    dag->period_time = dag_user->period_time;
    dag_node_create(dag, dag_user);
    dag_edge_create(dag, dag_user);
    acoral_list_init(&dag->list);
    acoral_lifo_queue_add(&acoral_dag_queue, &dag->list);
}

/**
 * @brief 所有系统dag的创建
 * 
 */
static void acoral_dag_create()
{
    acoral_list_t *tmp,*head;
    acoral_dag_user *dag_user;
    head = &acoral_dag_user_queue.head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)
    {
        dag_user = list_entry(tmp,acoral_dag_user,list);
        acoral_dag_init(dag_user);
    }
}

#include "calculate_time.h"
#include "measure.h"
/**
 * @brief dag线程统一运行函数
 *
 * @param args 运行参数
 */
void acoral_dag_task(void *args)
{
    acoral_thread_t *cur=acoral_cur_thread;
    dag_task_data_t *data=cur->data;
    if(data != NULL)
    {
        for(int i=0;i<data->pend_ipc_cnt;i++)
        {
            acoral_sem_pends(data->pend_ipc[i]);//获取前置节点信号量
        }
        data->dag_route(data->dag_args);//该dag节点执行函数
        acoral_sem_posts(data->post_ipc);//释放该节点信号量
    }
    else
    {
        acoral_print("no dag data!\r\n");
    }
}


/**
 * @brief dag线程处理钩子函数
 * 
 * @param args 参数
 */
static void dag_deal_hook(void *args)
{
    acoral_thread_t *thread = (acoral_thread_t *)args;
    dag_task_data_t *dt_data = (dag_task_data_t *)thread->data;
    acoral_sem_init(dt_data->post_ipc, 0);
}

/**
 * @brief dag线程回收钩子函数
 * 
 * @param args 参数
 */
static void dag_release_hook(void *args)
{
    acoral_thread_t *thread = (acoral_thread_t *)args;
    dag_task_data_t *dt_data = (dag_task_data_t *)thread->data;
    dt_data->dag_route = NULL;
    dt_data->dag_args = NULL;
    acoral_vol_free(dt_data->pend_ipc);
    dt_data->pend_ipc = NULL;
    dt_data->post_ipc = NULL;
    acoral_vol_free(dt_data);//回收私有数据内存
    thread->data = NULL;
}

/**
 * @brief dag线程调度钩子函数实例
 * 
 */
acoral_thread_hook_t dag_hook = {
    .deal_hook = dag_deal_hook,
    .release_hook = dag_release_hook
};

/**
 * @brief 系统dag到线程的映射
 * 
 */
static void acoral_dag_to_thread()
{
    acoral_u8 edge_cnt = 0;
    void *dag_policy_data;
    dag_task_data_t *dag_task_data;
    acoral_list_t *dag_tmp,*dag_head;
    acoral_list_t *node_tmp,*node_head;
    acoral_list_t *edge_tmp,*edge_head;
    acoral_dag *dag;
    acoral_dag_node *dag_node;
    acoral_dag_edge *dag_edge;
    dag_head = &acoral_dag_queue.head;
    for(dag_tmp=dag_head->next;dag_tmp!=dag_head;dag_tmp=dag_tmp->next)
    {
        dag = list_entry(dag_tmp,acoral_dag,list);
        node_head = &dag->dag_node_queue.head;
        edge_head = &dag->dag_edge_queue.head;
        for(node_tmp=node_head->next;node_tmp!=node_head;node_tmp=node_tmp->next)
        {
            edge_cnt = 0;
            dag_node = list_entry(node_tmp,acoral_dag_node,list);
            //配置dag线程专用数据
            dag_task_data = (dag_task_data_t *)acoral_vol_malloc(sizeof(dag_task_data_t));
            dag_task_data->pend_ipc_cnt = dag_node->in_degree;
            dag_task_data->pend_ipc = (acoral_ipc_t **)acoral_vol_malloc(dag_node->in_degree*sizeof(acoral_ipc_t *));
            dag_task_data->post_ipc = dag_node->sem;
            dag_task_data->dag_route = dag_node->route;
            dag_task_data->dag_args = dag_node->args;
            if(dag_node->in_degree > 0)
            {
                //遍历系统边，找到后部节点为该节点的边，获取前驱结点信号量
                for(edge_tmp=edge_head->next;edge_tmp!=edge_head;edge_tmp=edge_tmp->next)
                {
                    dag_edge = list_entry(edge_tmp,acoral_dag_edge,list);
                    if(dag_edge->next_node == dag_node)
                    {
                        dag_task_data->pend_ipc[edge_cnt] = dag_edge->prev_node->sem;
                        edge_cnt++;
                        if(edge_cnt >= dag_node->in_degree)
                            break;
                    }
                }
            }
            //使用周期线程策略
            dag_policy_data = acoral_vol_malloc(sizeof(acoral_period_policy_data_t));
            ((acoral_period_policy_data_t *)dag_policy_data)->prio = dag_node->prio;
            ((acoral_period_policy_data_t *)dag_policy_data)->cpu = dag_node->processor;
            ((acoral_period_policy_data_t *)dag_policy_data)->time = dag->period_time;
            //创建线程
            acoral_create_thread(acoral_dag_task,
                                    1024,
                                    NULL,
                                    "acoral_dag_task",
                                    NULL,
                                    ACORAL_SCHED_POLICY_PERIOD,
                                    dag_policy_data,
                                    dag_task_data,
                                    &dag_hook);
            acoral_vol_free(dag_policy_data);
        }
    }
}

/**
 * @brief 并行编程框架映射
 * 
 */
void dag_decode()
{
    //相关资源初始化
    acoral_lifo_queue_init(&acoral_dag_queue);
    acoral_lifo_queue_init(&acoral_dag_user_queue);
    //用户节点初始化
    dag_user_init();
    //用户节点映射到数据结构
    acoral_dag_create();
    //数据结构映射到线程
    acoral_dag_to_thread();
}

/**
 * @brief 添加用户节点
 * 
 * @param dag_user 用户dag
 * @param user_node 用户节点
 */
void dag_add_user_node(acoral_dag_user *dag_user, acoral_dag_user_node *user_node)
{
    acoral_list_init(&user_node->list);
    acoral_lifo_queue_add(&dag_user->dag_user_node_queue, &user_node->list);
}

/**
 * @brief 添加用户dag
 * 
 * @param dag_user 用户dag
 */
void dag_add_user(acoral_dag_user *dag_user)
{
    acoral_list_init(&dag_user->list);
    acoral_lifo_queue_init(&dag_user->dag_user_node_queue);
    acoral_lifo_queue_add(&acoral_dag_user_queue, &dag_user->list);
}
