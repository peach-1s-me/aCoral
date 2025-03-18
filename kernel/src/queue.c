/**
 * @file queue.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief 时刻队列(差分链表)
 * @version 1.0
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-02 <td>增加注释
 */
#include <queue.h>

/**
 * @brief 时刻队列初始化
 * 
 * @param queue 时刻队列指针
 */
void acoral_tick_queue_init(acoral_queue_t *queue)
{
    acoral_vlist_init(&queue->head, 0);
    acoral_spin_init(&queue->lock);
}

/**
 * @brief 时刻队列添加节点
 * 
 * @param queue 时刻队列指针
 * @param tnode 新链表节点指针
 */
void acoral_tick_queue_add(acoral_queue_t *queue, acoral_list_t *tnode)
{
	acoral_list_t *head,*tmp;
	acoral_32 tick = tnode->value;
	acoral_32 tick_tmp;
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    head=&queue->head;
    for(tmp=head->next;tick_tmp=tick,tmp!=head;tmp=tmp->next)//边减去边比较，寻找添加位置
    {
        tick  = tick - tmp->value;
        if (tick < 0)
            break;
    }
    tnode->value = tick_tmp;//恢复暂存的值
    acoral_list_add(tnode, tmp->prev);//添加节点
    if(tmp != head)
    {
        tmp->value -= tick_tmp;//后续节点减去当前节点的值
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief 时刻队列删除节点
 * 
 * @param queue 时刻队列指针
 * @param tnode 要删除的链表节点指针
 */
void acoral_tick_queue_del(acoral_queue_t *queue, acoral_list_t *tnode)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_del(tnode);//删除节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}


/**
 * @brief 优先级队列初始化
 * 
 * @param queue 优先级队列指针
 */
void acoral_prio_queue_init(acoral_queue_t *queue)
{
    acoral_vlist_init(&queue->head, 0);
    acoral_spin_init(&queue->lock);
}
/**
 * @brief 优先级队列添加节点
 * 
 * @param queue 优先级队列指针
 * @param pnode 新链表节点指针
 */
void acoral_prio_queue_add(acoral_queue_t *queue, acoral_list_t *pnode)
{
    acoral_list_t *head,*tmp;
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    head=&queue->head;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//比较寻找添加位置
    {
        if(tmp->value > pnode->value)
            break;
        if(tmp == tmp->next)
            break;
    }
    acoral_list_add(pnode,tmp->prev);//添加节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief 优先级队列删除节点
 * 
 * @param queue 优先级队列指针
 * @param pnode 要删除的链表节点指针
 */
void acoral_prio_queue_del(acoral_queue_t *queue, acoral_list_t *pnode)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_del(pnode);//删除节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief FIFO队列初始化
 * 
 * @param queue FIFO队列指针
 */
void acoral_fifo_queue_init(acoral_queue_t *queue)
{
    acoral_list_init(&queue->head);
    acoral_spin_init(&queue->lock);
}
/**
 * @brief FIFO队列初始化（队列头有初始值）
 *
 * @param queue FIFO队列指针
 * @param value 队列头值
 */
void acoral_fifo_queue_v_init(acoral_queue_t *queue, acoral_32 value)
{
    acoral_vlist_init(&queue->head, value);
    acoral_spin_init(&queue->lock);
}
/**
 * @brief FIFO队列添加节点
 * 
 * @param queue FIFO队列指针
 * @param node 新链表节点指针
 */
void acoral_fifo_queue_add(acoral_queue_t *queue, acoral_list_t *node)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_add_tail(node, &queue->head);//添加节点到链表尾部，也就是队列头前面
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief FIFO队列删除节点
 * 
 * @param queue FIFO队列指针
 * @param node 要删除的链表节点指针
 */
void acoral_fifo_queue_del(acoral_queue_t *queue, acoral_list_t *node)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_del(node);//删除节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief LIFO队列初始化
 * 
 * @param queue LIFO队列指针
 */
void acoral_lifo_queue_init(acoral_queue_t *queue)
{
    acoral_list_init(&queue->head);
    acoral_spin_init(&queue->lock);
}
/**
 * @brief LIFO队列初始化（队列头有初始值）
 *
 * @param queue LIFO队列指针
 * @param value 队列头值
 */
void acoral_lifo_queue_v_init(acoral_queue_t *queue, acoral_32 value)
{
    acoral_vlist_init(&queue->head, value);
    acoral_spin_init(&queue->lock);
}
/**
 * @brief LIFO队列添加节点
 * 
 * @param queue LIFO队列指针
 * @param node 新链表节点指针
 */
void acoral_lifo_queue_add(acoral_queue_t *queue, acoral_list_t *node)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_add(node, &queue->head);//添加节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
/**
 * @brief LIFO队列删除节点
 * 
 * @param queue LIFO队列指针
 * @param node 要删除的链表节点指针
 */
void acoral_lifo_queue_del(acoral_queue_t *queue, acoral_list_t *node)
{
#ifdef CFG_SMP
    acoral_spin_lock(&queue->lock);
#endif
    acoral_list_del(node);//删除节点
#ifdef CFG_SMP
    acoral_spin_unlock(&queue->lock);
#endif
}
