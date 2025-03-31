/**
 * @file kernel.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层总头文件
 * @version 2.0
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-03-31 <td>规范代码风格
 */
#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H
#include "error.h"
#include "thread.h"
#include "lsched.h"
#include "int.h"
#include "timer.h"
#include "mem.h"
#include "ipc.h"
#include "policy.h"
#include "comm_thrd.h"
#include "period_thrd.h"
#include "timed_thrd.h"
#include "cpu.h"
#include "smp.h"
#include "list.h"
#include "queue.h"
#include "bitops.h"
#include "res_pool.h"
#include "monitor.h"

#ifdef CFG_SMP
#include "ipi.h"
#endif /* CFG_SMP */

#include "config.h"
#include "spinlock.h"
#endif
