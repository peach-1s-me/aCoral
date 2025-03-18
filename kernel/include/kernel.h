/**
 * @file kernel.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层总头文件
 * @version 1.0
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 */
#ifndef KERNEL_H
#define KERNEL_H
#include <error.h>
#include <thread.h>
#include <lsched.h>
#include <int.h>
#include <timer.h>
#include <mem.h>
#include <ipc.h>
#include <policy.h>
#include <comm_thrd.h>
#include <period_thrd.h>
#include <timed_thrd.h>
#include <cpu.h>
#include <smp.h>
#include <list.h>
#include <queue.h>
#include <bitops.h>
#include <res_pool.h>
#include <monitor.h>

#ifdef CFG_SMP
#include <ipi.h>
#endif

#include <config.h>
#include <spinlock.h>
#endif /* KERNEL_H_ */
