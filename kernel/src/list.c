/**
 * @file list.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层链表相关源文件
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
#include <list.h>
/**
 * @brief 添加链表节点（尾部添加）
 * 
 * @param new 新链表节点
 * @param head 要添加到的链表节点
 */
void acoral_list_add(acoral_list_t *new, acoral_list_t *head)
{
#ifdef CFG_SMP
	acoral_list_t *temp_next = head->next;
    acoral_spin_lock(&new->lock);
    acoral_spin_lock(&head->lock);
    if(head != temp_next)
        acoral_spin_lock(&temp_next->lock);
#endif
    new->prev = head;
    new->next = head->next;
    head->next->prev = new;
    head->next = new;
#ifdef CFG_SMP
    acoral_spin_unlock(&new->lock);
    acoral_spin_unlock(&head->lock);
    if(head != temp_next)
        acoral_spin_unlock(&temp_next->lock);
#endif
}
/**
 * @brief 添加链表节点（头部添加）
 * 
 * @param new 新链表节点
 * @param head 要添加到的链表节点
 */
void acoral_list_add_tail(acoral_list_t *new, acoral_list_t *head)
{
#ifdef CFG_SMP
	acoral_list_t *temp_prev = head->prev;
    acoral_spin_lock(&new->lock);
    acoral_spin_lock(&head->lock);
    if(head != temp_prev)
        acoral_spin_lock(&temp_prev->lock);
#endif
    new->prev = head->prev;
    new->next = head;
    head->prev->next = new;
    head->prev = new;
#ifdef CFG_SMP
    acoral_spin_unlock(&new->lock);
    acoral_spin_unlock(&head->lock);
    if(head != temp_prev)
        acoral_spin_unlock(&temp_prev->lock);
#endif
}
/**
 * @brief 删除链表节点
 * 
 * @param entry 要删除的链表节点指针
 */
void acoral_list_del(acoral_list_t *entry)
{
#ifdef CFG_SMP
	acoral_list_t *temp_prev = entry->prev;
	acoral_list_t *temp_next = entry->next;
    acoral_spin_lock(&temp_prev->lock);
    acoral_spin_lock(&entry->lock);
    if(temp_prev != temp_next)
        acoral_spin_lock(&temp_next->lock);
#endif
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->next = entry;
    entry->prev = entry;
#ifdef CFG_SMP
    acoral_spin_unlock(&temp_prev->lock);
    acoral_spin_unlock(&entry->lock);
    if(temp_prev != temp_next)
        acoral_spin_unlock(&temp_next->lock);
#endif
}
/**
 * @brief 链表节点初始化（无值）
 * 
 * @param ptr 链表节点指针
 */
void acoral_list_init(acoral_list_t *ptr)
{
    ptr->next = ptr;
    ptr->prev = ptr;
#ifdef CFG_SMP
    acoral_spin_init(&ptr->lock);
#endif
}
/**
 * @brief 链表节点初始化（有值）
 * 
 * @param ptr 链表节点指针
 * @param value 初始化的值
 */
void acoral_vlist_init(acoral_list_t *ptr, acoral_32 value)
{
    ptr->next = ptr;
    ptr->prev = ptr;
    ptr->value = value;
#ifdef CFG_SMP
    acoral_spin_init(&ptr->lock);
#endif
}
