/**
 * @file res_pool.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层内存资源池相关头文件
 * @version 1.0
 * @date 2022-09-27
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-27 <td>增加注释
 */
#ifndef KERNEL_RES_POOL_H
#define KERNEL_RES_POOL_H
#include <type.h>
#include <list.h>
#include <spinlock.h>
///最大资源内存池数量
#define ACORAL_MAX_POOLS 40

///资源内存池类型：线程
#define ACORAL_RES_THREAD   1
///资源内存池类型：线程通信
#define ACORAL_RES_IPC    2

// #define ACORAL_RES_USER     6

///资源内存池index掩码
#define ACORAL_POOL_INDEX_MASK 0x000003FF
///资源内存池index偏移量
#define ACORAL_POOL_INDEX_BIT 0
///资源类型掩码
#define ACORAL_RES_TYPE_MASK   0x00003c00
///资源类型偏移量
#define ACORAL_RES_TYPE_BIT 10
///资源index掩码
#define ACORAL_RES_INDEX_MASK  0x00FFC000
///资源index偏移量
#define ACORAL_RES_INDEX_BIT 14
///资源index初始偏移量
#define ACORAL_RES_INDEX_INIT_BIT 16
///资源所在cpu掩码
#define ACORAL_RES_CPU_MASK    0xFF000000
///资源所在cpu偏移量
#define ACORAL_RES_CPU_BIT 24


///根据资源ID获取资源类型
#define ACORAL_RES_TYPE(id) ((id&ACORAL_RES_TYPE_MASK)>>ACORAL_RES_TYPE_BIT)
///根据资源ID获取资源所在cpu
#define ACORAL_RES_CPU(id) ((id&ACORAL_RES_CPU_MASK)>>ACORAL_RES_CPU_BIT)

/**
 * @brief 资源数据块
 * 
 */
typedef union {
    acoral_id id;///<资源id
    acoral_u16 next_id;///<下个资源id
}acoral_res_t;

/**
 * @brief 资源api结构体
 * 
 */
typedef struct{
    void (*release_res)(acoral_res_t *res);///<回收资源函数
}acoral_res_api_t;

/**
 * @brief 内存池控制块结构体
 * 
 */
typedef struct {
    acoral_u32 type;///<类型
    acoral_u32 size;///<单个资源内存大小
    acoral_u32 num_per_pool;///<每个池的资源数量
    acoral_u32 num;///<用于运行中
    acoral_u32 max_pools;///<最大池数量  
    acoral_queue_t total_pools;///<所有池队列
    acoral_queue_t free_pools;///<空闲池队列
    acoral_res_api_t *api;///<资源api
    acoral_spinlock_t lock;///<自旋锁
    acoral_u8 *name;///<名称
}acoral_pool_ctrl_t;

/**
 * @brief 内存池结构体
 * 
 */
typedef struct {
    void *base_adr;///<这个有两个作用，在为空闲的时候，它指向下一个pool，否则为它管理的资源的基地址
    void *res_free;///<首个空闲资源指针
    acoral_id id;///<内存池id
    acoral_u32 size;///<单个资源内存大小
    acoral_u32 num;///<池中资源数量
    acoral_u32 free_num;///<池中空闲资源数量
    acoral_pool_ctrl_t *ctrl;///<内存池控制块
    acoral_list_t total_list;///<所有池链表
    acoral_list_t free_list;///<空闲池链表
    acoral_spinlock_t lock;///<自旋锁
}acoral_pool_t;

acoral_err acoral_create_pool(acoral_pool_ctrl_t *pool_ctrl);
void acoral_release_pool(acoral_pool_ctrl_t *pool_ctrl);
void acoral_collect_pool(acoral_pool_ctrl_t *pool_ctrl);
acoral_res_t *acoral_get_res(acoral_pool_ctrl_t *pool_ctrl);
void acoral_release_res(acoral_res_t *res);
acoral_pool_t *acoral_get_pool_by_id(acoral_id res_id);
acoral_pool_t *acoral_get_free_pool();
acoral_res_t *acoral_get_res_by_id(acoral_id id);
void acoral_pool_res_init(acoral_pool_t * pool);
void acoral_pool_ctrl_init(acoral_pool_ctrl_t *pool_ctrl);
// void acoral_obj_pool_init(acoral_pool_ctrl_t *pool_ctrl);
// void acoral_obj_pool_release(acoral_pool_ctrl_t *pool_ctrl);
// void *acoral_get_obj(acoral_pool_ctrl_t *pool_ctrl);
// void acoral_free_obj(void *obj);
void acoral_res_pool_sys_init(void);

#endif
