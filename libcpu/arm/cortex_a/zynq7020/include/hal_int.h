/**
 * @file hal_int.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层中断相关头文件
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

#ifndef HAL_INT_H
#define HAL_INT_H
#include <type.h>
#include <config.h>
acoral_u32 hal_irq_switch[CFG_MAX_CPU];
void hal_all_entry( acoral_u32 ulICCIAR );

///关中断，汇编函数
void HAL_INTR_DISABLE();
///开中断，汇编函数
void HAL_INTR_ENABLE();

///切换到线程，汇编函数
void HAL_START_OS();

void hal_intr_disable_save(void);
void hal_intr_restore(void);

///重定义屏蔽中断函数
#define HAL_INTR_DISABLE_SAVE() hal_intr_disable_save()
///重定义恢复中断函数
#define HAL_INTR_RESTORE() hal_intr_restore()

#endif
