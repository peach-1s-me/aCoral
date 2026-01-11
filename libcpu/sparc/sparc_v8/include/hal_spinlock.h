/**
 * @file hal_spinlock.h
 * @author 文佳源，绕洪江
 * @brief hal层自旋锁相关头文件
 * @version 1.0
 * @date 2025-05-10
 * 
 * @copyright Copyright (c) 2025
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>文佳源，绕洪江 <td>2025-05-10 <td>创建文件
 */

#ifndef HAL_SPINLOCK_H
#define HAL_SPINLOCK_H
#include <type.h>
#include <hal_int.h>
/**
 * @brief   自旋锁结构体
 */
typedef struct{
    volatile acoral_32 lock;///<自旋锁
}hal_spinlock_t;

///内存屏障
#define HAL_MB() __asm__ __volatile__("MEMBAR #sync": : :"memory")

///自旋锁初始化为未锁状态
#define HAL_SPIN_INIT(v)        ((v)->lock = 0)
///自旋锁初始化为上锁状态
#define HAL_SPIN_INIT_LOCK(v)   ((v)->lock = 1)
///检测自旋锁是否上锁，是则返回1
#define HAL_SPIN_IS_LOCKED(x)   ((x)->lock != 0)
///重定义尝试上锁函数，为上层使用
#define HAL_SPIN_TRYLOCK(v)     hal_spin_trylock(v)
///重定义上锁函数，为上层使用
#define HAL_SPIN_LOCK(v)        hal_spin_lock(v)
///重定义解锁函数，为上层使用
#define HAL_SPIN_UNLOCK(v)      hal_spin_unlock(v)

/**
 * @brief   自旋锁上锁
 * 
 * @param   lock    自旋锁
 */
static inline void hal_spin_lock(hal_spinlock_t *lock)
{
    acoral_u32 tmp;
    __asm__ __volatile__(
    "1: ld   [%1], %0 \n"    /* 加载锁值到tmp */
    "   cmp  %0, 0    \n"    /* SPARC使用cmp比较 */
    "   bne  1b       \n"    /* 如果不为0(锁被持有)，循环等待 */
    "   nop           \n"    /* 延迟槽 */
    "   mov  1, %0    \n"    /* 准备值1 */
    "   swap [%1], %0 \n"    /* 原子交换 */
    "   cmp  %0, 0    \n"    /* 测试原值 */
    "   bne  1b       \n"    /* 如果原值不为0(交换失败)，重试 */
    "   nop           \n"    /* 延迟槽 */
    : "=&r" (tmp)
    : "r" (&lock->lock)
    : "cc", "memory");

    HAL_MB();
}
/**
 * @brief   自旋锁尝试上锁
 * 
 * @param   lock    自旋锁
 * @return  acoral_32   上锁成功 1;否则 0
 */
static inline acoral_32 hal_spin_trylock(hal_spinlock_t *lock)
{
    acoral_u32 tmp = 1; /* 要交换的值 */
    acoral_u32 old;
    __asm__ __volatile__(
    "swap [%1], %0\n"    /* 原子交换 lock->lock 和 tmp 的值 */
    : "=r" (old)         /* 输出: old 存储 lock->lock 的旧值 */
    : "r" (&lock->lock), /* 输入: lock 的地址 */
      "0" (tmp)          /* 输入: tmp 的值，并绑定到输出 %0 */
    : "memory");

    if (old == 0)
    {
        HAL_MB();
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
 * @brief   自旋锁解锁
 * 
 * @param   lock    自旋锁
 */
static inline void hal_spin_unlock(hal_spinlock_t *lock)
{
    __asm__ __volatile__(
    "st %%g0, [%0]\n"
    :
    : "r" (&lock->lock)
    : "memory");
}
#endif
