/**
 * @file hal_spinlock.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层自旋锁相关头文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>增加注释
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
#define HAL_MB() __asm__ __volatile__("": : :"memory")

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
    "1: ldrex   %0, [%1]\n"
    "   teq     %0, #0\n"
    "   strexeq %0, %2, [%1]\n"
    "   teqeq   %0, #0\n"
    "   bne     1b"
    : "=&r" (tmp)
    : "r" (&lock->lock), "r" (1)
    : "cc");

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
    acoral_u32 tmp;
    __asm__ __volatile__(
    "   ldrex   %0, [%1]\n"
    "   teq %0, #0\n"
    "   strexeq %0, %2, [%1]"
    : "=&r" (tmp)
    : "r" (&lock->lock), "r" (1)
    : "cc");

    if (tmp == 0)
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
    "   str   %1, [%0]\n"
    :
    : "r" (&lock->lock), "r" (0)
    : "cc");
}
#endif
