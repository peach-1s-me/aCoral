/**
 * @file queue.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层队列相关头文件
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
#ifndef KERNEL_QUEUE_H
#define KERNEL_QUEUE_H
#include <list.h>

/**
 * @brief 队列结构体
 * 
 */
typedef struct{
    acoral_list_t head;///<队列头
    acoral_spinlock_t lock;///<自旋锁
    void *data;///<数据
}acoral_queue_t;

void acoral_tick_queue_init(acoral_queue_t *queue);
void acoral_tick_queue_add(acoral_queue_t *queue, acoral_list_t *tnode);
void acoral_tick_queue_del(acoral_queue_t *queue, acoral_list_t *tnode);
void acoral_prio_queue_init(acoral_queue_t *queue);
void acoral_prio_queue_add(acoral_queue_t *queue, acoral_list_t *pnode);
void acoral_prio_queue_del(acoral_queue_t *queue, acoral_list_t *pnode);
void acoral_fifo_queue_init(acoral_queue_t *queue);
void acoral_fifo_queue_v_init(acoral_queue_t *queue, acoral_32 value);
void acoral_fifo_queue_add(acoral_queue_t *queue, acoral_list_t *node);
void acoral_fifo_queue_del(acoral_queue_t *queue, acoral_list_t *node);
void acoral_lifo_queue_init(acoral_queue_t *queue);
void acoral_lifo_queue_v_init(acoral_queue_t *queue, acoral_32 value);
void acoral_lifo_queue_add(acoral_queue_t *queue, acoral_list_t *node);
void acoral_lifo_queue_del(acoral_queue_t *queue, acoral_list_t *node);
#endif /* QUEUE_H_ */
