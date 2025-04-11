/**
 * @file res_pool.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层内存资源池相关源文件
 * @version 2.0
 * @date 2025-04-10
 *
 * @copyright Copyright (c) 2022 EIC-UESTC
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-27 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-04-10 <td>规范代码风格
 */
#include "type.h"
#include "hal.h"
#include "queue.h"
#include "lsched.h"
#include "mem.h"
#include "print.h"
#include "res_pool.h"
#include "error.h"
/* 全局内存池 */
acoral_pool_t  acoral_pools[ACORAL_MAX_POOLS];
/* 空闲内存池指针 */
acoral_pool_t *acoral_free_res_pool;

/**
 * @brief 创建某一资源内存池
 *
 * @param pool_ctrl 资源内存池控制块指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_create_pool(acoral_pool_ctrl_t *pool_ctrl)
{
    acoral_pool_t *pool;
    if (pool_ctrl->num >= pool_ctrl->max_pools)
    {
        return KR_RES_ERR_MAX_POOL;
    }
    /* pool属性设置 */
    pool = acoral_get_free_pool();
    if (NULL == pool)
    {
        return KR_RES_ERR_NO_POOL;
    }
    pool->id       = (pool_ctrl->type << ACORAL_RES_TYPE_BIT) | pool->id;
    pool->size     = pool_ctrl->size;
    pool->num      = pool_ctrl->num_per_pool;
    pool->base_adr = (void *)acoral_malloc(pool->size * pool->num);
    if (NULL == pool->base_adr)
    {
        return KR_RES_ERR_NO_MEM;
    }
    pool->res_free = pool->base_adr;
    pool->free_num = pool->num;
    pool->ctrl     = pool_ctrl;
    acoral_pool_res_init(pool);
    /* 加入内存池 */
    acoral_fifo_queue_add(&pool_ctrl->total_pools, &pool->total_list);
    acoral_fifo_queue_add(&pool_ctrl->free_pools, &pool->free_list);
    pool_ctrl->num++;
    return 0;
}

/**
 * @brief 释放某一资源内存池所有资源
 *
 * @param pool_ctrl 资源内存池控制块指针
 */
void acoral_release_pool(acoral_pool_ctrl_t *pool_ctrl)
{
    acoral_pool_t *pool;
    acoral_list_t *list, *head;
    head = &pool_ctrl->total_pools.head;
    if (acoral_list_empty(head))
    {
        return;
    }
    for (list = head->next; list != head; list = list->next)
    {
        pool = list_entry(list, acoral_pool_t, free_list);
        acoral_list_del(&pool->total_list);
        acoral_list_del(&pool->free_list);
        acoral_free(pool->base_adr);

        pool->base_adr       = (void *)acoral_free_res_pool;
        pool->id             = pool->id & ACORAL_POOL_INDEX_MASK;
        acoral_free_res_pool = pool;
    }
}

/**
 * @brief 回收某一资源内存池空闲的内存池
 *
 * @param pool_ctrl 资源内存池控制块指针
 */
void acoral_collect_pool(acoral_pool_ctrl_t *pool_ctrl)
{
    acoral_pool_t *pool;
    acoral_list_t *list, *head;
    head = &pool_ctrl->free_pools.head;
    if (acoral_list_empty(head))
    {
        return;
    }
    for (list = head->next; list != head; list = list->next)
    {
        pool = list_entry(list, acoral_pool_t, free_list);
        if (pool->free_num == pool->num)
        {
            acoral_list_del(&pool->total_list);
            acoral_list_del(&pool->free_list);
            acoral_free(pool->base_adr);
            pool->base_adr = (void *)acoral_free_res_pool;
            pool->id = pool->id & ACORAL_POOL_INDEX_MASK;
            acoral_free_res_pool = pool;
        }
    }
}

/**
 * @brief 获取某一资源
 *
 * @param pool_ctrl 资源池控制块
 * @return acoral_res_t* 资源数据块指针
 */
acoral_res_t *acoral_get_res(acoral_pool_ctrl_t *pool_ctrl)
{

    acoral_list_t *first;
    acoral_res_t *res;
    acoral_pool_t *pool;
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&pool_ctrl->lock);
#endif
    first = pool_ctrl->free_pools.head.next;
    if (acoral_list_empty(first)) /* 无空闲池就再创建一个 */
    {
        if (acoral_create_pool(pool_ctrl)) /* 创建失败 */
        {
#ifdef CFG_SMP
            acoral_spin_unlock(&pool_ctrl->lock);
#endif
            acoral_exit_critical();
            return NULL;
        }
        else /* 创建成功 */
        {
            first = pool_ctrl->free_pools.head.next;
        }
    }
    pool = list_entry(first, acoral_pool_t, free_list); /* 获取空闲池 */
    res  = (acoral_res_t *)pool->res_free;               /* 获取空闲资源 */
    /* 移动空闲资源指针 */
    pool->res_free = (void *)((acoral_u8 *)pool->base_adr + res->next_id * pool->size); 
    res->id = ((res->id >> (ACORAL_RES_INDEX_INIT_BIT - ACORAL_RES_INDEX_BIT)) &
              ACORAL_RES_INDEX_MASK) | pool->id;
    pool->free_num--;    /* 减少空闲资源数量 */
    if (!pool->free_num) /* 无空闲资源 */
    {
        acoral_list_del(&pool->free_list); /* 从控制块中的空闲池队列中删除自己 */
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&pool_ctrl->lock);
#endif
    acoral_exit_critical();
    return res;
}

/**
 * @brief 释放某一资源
 *
 * @param res 资源数据块指针
 */
void acoral_release_res(acoral_res_t *res)
{
    acoral_pool_t *pool;
    acoral_u32 index;
    void *tmp;
    acoral_pool_ctrl_t *pool_ctrl;
    pool = acoral_get_pool_by_id(res->id); /* 获取内存池 */
    if (NULL == pool)
    {
        acoral_printerr("Res release Err\n");
        return;
    }
    pool_ctrl = pool->ctrl; /* 获取控制块 */
#ifdef CFG_SMP
    acoral_spin_lock(&pool_ctrl->lock);
#endif
    if ((void *)res < pool->base_adr)
    {
        acoral_printerr("Err Res\n");
        return;
    }
    index = (((acoral_u32)res - (acoral_u32)pool->base_adr) / pool->size); /* 计算index */
    if (index >= pool->num)
    {
        acoral_printerr("Err Res\n");
        return;
    }

    tmp            = pool->res_free;
    pool->res_free = (void *)res;
    res->id        = index << ACORAL_RES_INDEX_INIT_BIT;
    res->next_id   = ((acoral_res_t *)tmp)->id >> ACORAL_RES_INDEX_INIT_BIT;
    pool->free_num++;
    if (acoral_list_empty(&pool->free_list))
    {
        acoral_fifo_queue_add(&pool_ctrl->free_pools, &pool->free_list);
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&pool_ctrl->lock);
#endif
    return;
}

/**
 * @brief 根据资源ID获取某一资源对应的资源池
 *
 * @param res_id 资源ID
 * @return acoral_pool_t* 资源数据块指针
 */
acoral_pool_t *acoral_get_pool_by_id(acoral_id res_id)
{
    acoral_u32 index;
    index = (res_id & ACORAL_POOL_INDEX_MASK) >> ACORAL_POOL_INDEX_BIT;
    if (index < ACORAL_MAX_POOLS)
    {
        return acoral_pools + index;
    }
    return NULL;
}

/**
 * @brief 获取空闲资源池
 *
 * @return acoral_pool_t* 资源内存池控制块指针
 */
acoral_pool_t *acoral_get_free_pool()
{

    acoral_pool_t *tmp;
    acoral_enter_critical();
    tmp = acoral_free_res_pool;
    if (NULL != tmp)
    {
#ifdef CFG_SMP
        acoral_spin_lock(&tmp->lock);
#endif
        acoral_free_res_pool = *(void **)tmp->base_adr;
#ifdef CFG_SMP
        acoral_spin_unlock(&tmp->lock);
#endif
    }
    acoral_exit_critical();
    return tmp;
}

/**
 * @brief 根据资源ID获取某一资源对应的资源数据块
 *
 * @param id 资源ID
 * @return acoral_res_t* 资源数据块指针
 */
acoral_res_t *acoral_get_res_by_id(acoral_id id)
{
    acoral_pool_t *pool;
    acoral_u32 index;
    pool = acoral_get_pool_by_id(id);
    if (NULL == pool)
    {
        return NULL;
    }
    index = (id & ACORAL_RES_INDEX_MASK) >> ACORAL_RES_INDEX_BIT;
    return (acoral_res_t *)((acoral_u8 *)pool->base_adr + index * pool->size);
}

/**
 * @brief 资源池中资源初始化
 *
 * @param pool 资源池指针
 */
void acoral_pool_res_init(acoral_pool_t *pool)
{
    acoral_res_t *res;
    acoral_u32 i;
    acoral_u8 *pblk;
    acoral_u32 blks;
    blks = pool->num;
    res  = (acoral_res_t *)pool->base_adr;
    pblk = (acoral_u8 *)pool->base_adr + pool->size;
    /* 遍历并初始化 */
    for (i = 0; i < (blks - 1); i++)
    {
        res->id      = i << ACORAL_RES_INDEX_INIT_BIT;
        res->next_id = i + 1;
        res          = (acoral_res_t *)pblk;
        pblk        += pool->size;
    }
    res->id      = (blks - 1) << ACORAL_RES_INDEX_INIT_BIT;
    res->next_id = 0;
}

/**
 * @brief 资源池初始化
 *
 */
static void acoral_pools_init(void)
{
    acoral_pool_t *pool;
    acoral_u32 i;

    pool = &acoral_pools[0];
    for (i = 0; i < (ACORAL_MAX_POOLS - 1); i++) /* 遍历并初始化 */
    {
        pool->base_adr = (void *)&acoral_pools[i + 1];
        pool->id       = i;
        pool++;
        acoral_list_init(&pool->total_list);
        acoral_list_init(&pool->free_list);
        acoral_spin_init(&pool->lock);
    }
    pool->base_adr       = (void *)0;
    acoral_free_res_pool = &acoral_pools[0];
    acoral_list_init(&acoral_free_res_pool->total_list);
    acoral_list_init(&acoral_free_res_pool->free_list);
    acoral_spin_init(&acoral_free_res_pool->lock);
}

/**
 * @brief 资源内存池控制块初始化
 *
 * @param pool_ctrl 资源内存池控制块指针
 */
void acoral_pool_ctrl_init(acoral_pool_ctrl_t *pool_ctrl)
{
    acoral_u32 size;
    pool_ctrl->num = 0;
    acoral_fifo_queue_init(&pool_ctrl->total_pools);
    acoral_fifo_queue_init(&pool_ctrl->free_pools);
    acoral_spin_init(&pool_ctrl->lock);
    /* 调整pool的对象个数以最大化利用分配了的内存 */
    size = acoral_malloc_size(pool_ctrl->size * pool_ctrl->num_per_pool);
    if (size < pool_ctrl->size)
    {
        pool_ctrl->num_per_pool = 0;
    }
    else
    {
        pool_ctrl->num_per_pool = size / pool_ctrl->size;
        acoral_create_pool(pool_ctrl);
    }
}

/**
 * @brief 资源内存池管理系统初始化
 *
 */
void acoral_res_pool_sys_init(void)
{
    acoral_pools_init();
}
