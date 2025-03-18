/**
 * @file spinlock.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层自旋锁相关头文件
 * @version 1.0
 * @date 2022-07-08
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 */
#ifndef KERNEL_SPINLOCK_H
#define KERNEL_SPINLOCK_H
#include <config.h>
#include <type.h>
#include <hal_spinlock.h>
///重定义自旋锁结构体，为上层使用
#define acoral_spinlock_t hal_spinlock_t

#ifdef CFG_SMP
///重定义自旋锁初始化
#define acoral_spin_init(v) HAL_SPIN_INIT(v)
///重定义自旋锁初始化（lock初始值为1）
#define acoral_spin_init_lock(v) HAL_SPIN_INIT_LOCK(v)
///重定义自旋锁检测
#define acoral_spin_is_locked(v) HAL_SPIN_IS_LOCKED(v)
///重定义自旋锁上锁（无阻塞）
#define acoral_spin_try_lock(v) HAL_SPIN_TRY_LOCK(v)
///重定义自旋锁上锁（有阻塞）
#define acoral_spin_lock(v) HAL_SPIN_LOCK(v)
///重定义自旋锁解锁
#define acoral_spin_unlock(v) HAL_SPIN_UNLOCK(v)
#else
#define acoral_spin_init(v)
#define acoral_spin_init_lock(v)
#define acoral_spin_is_locked(v) 
#define acoral_spin_try_lock(v) 
#define acoral_spin_lock(v)
#define acoral_spin_unlock(v)
#endif

#endif
