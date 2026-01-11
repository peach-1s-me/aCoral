/**
 * @file hal.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层总头文件
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
#ifndef HAL_H
#define HAL_H
#include <hal_comm.h>
#include <hal_int.h>
#include <hal_mem.h>
#include <hal_thread.h>
#ifdef CFG_SMP
#include <hal_cmp.h>
#include <hal_spinlock.h>
#endif
#endif /* HAL_H_ */
