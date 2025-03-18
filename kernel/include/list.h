/**
 * @file list.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层链表相关头文件
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

#ifndef KERNEL_LIST_H
#define KERNEL_LIST_H
#include <spinlock.h>

///寻找包含某链表节点的结构体首地址
#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/**
 * @brief 链表节点结构体
 * 
 */
struct acoral_list {
    struct acoral_list *next;///<下一个链表节点指针
    struct acoral_list *prev;///<上一个链表节点指针
    acoral_32 value;///<值
#ifdef CFG_SMP
    acoral_spinlock_t lock;///<自旋锁
#endif
};
///重定义链表结构体为一个类型
typedef struct acoral_list acoral_list_t;

///检测链表是否为空
#define acoral_list_empty(head) ((head)->next==(head))

void acoral_list_add(acoral_list_t *new, acoral_list_t *head);
void acoral_list_add_tail(acoral_list_t *new, acoral_list_t *head);
void acoral_list_del(acoral_list_t *entry);
void acoral_list_init(acoral_list_t *ptr);
void acoral_vlist_init(acoral_list_t *ptr, acoral_32 value);
#endif
