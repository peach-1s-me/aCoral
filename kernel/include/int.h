/**
 * @file int.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层中断系统头文件
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
#ifndef KERNEL_INT_H
#define KERNEL_INT_H
#include <config.h>
#include <type.h>
#include "xscugic.h"

typedef void (*acoral_intr_callback_t)(void *data);

extern XScuGic int_ctrl[CFG_MAX_CPU];
///重定义开中断
#define acoral_intr_enable() HAL_INTR_ENABLE()
///重定义开关中断
#define acoral_intr_disable() HAL_INTR_DISABLE()
///重定义中断嵌套获取
#define acoral_intr_nesting HAL_GET_INTR_NESTING()
///重定义增加中断嵌套
#define acoral_intr_nesting_inc() HAL_INTR_NESTING_INC()
///重定义减少中断嵌套
#define acoral_intr_nesting_dec() HAL_INTR_NESTING_DEC()
///重定义中断入口
#define acoral_intr_entry(ulICCIAR)	hal_all_entry(ulICCIAR)

void acoral_intr_sys_init(void);
void acoral_intr_init(acoral_u32 cpu);

void acoral_intr_callback_register(acoral_u32 cpu, acoral_u32 intr_id, acoral_intr_callback_t callback, void *callback_arg);
void acoral_intr_enable_by_id(acoral_u32 cpu, acoral_u32 intr_id);
void acoral_intr_disable_by_id(acoral_u32 cpu, acoral_u32 intr_id);

void acoral_intr_exit(void);
#endif
