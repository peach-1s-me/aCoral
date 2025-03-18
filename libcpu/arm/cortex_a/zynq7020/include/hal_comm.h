/**
 * @file hal_comm.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层通用头文件，通用即与硬件细节无关
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

#ifndef HAL_COMM_H
#define HAL_COMM_H
#include <type.h>
#include <config.h>

void hal_intr_nesting_init_comm(void);
acoral_u32 hal_get_intr_nesting_comm(void);
void hal_intr_nesting_dec_comm(void);
void hal_intr_nesting_inc_comm(void);

///重定义中断嵌套初始化函数，为上层使用
#define HAL_INTR_NESTING_INIT() hal_intr_nesting_init_comm()
///重定义获取中断嵌套层数函数，为上层使用
#define HAL_GET_INTR_NESTING() hal_get_intr_nesting_comm()
///重定义减少中断嵌套层数函数，为上层使用
#define HAL_INTR_NESTING_DEC() hal_intr_nesting_dec_comm()
///重定义增加中断嵌套层数函数，为上层使用
#define HAL_INTR_NESTING_INC() hal_intr_nesting_inc_comm()

///获取当前cpu，汇编函数
acoral_u32 HAL_GET_CURRENT_CPU(void);

///重定义普通切换上下文函数，为上层使用
#define HAL_COMM_SWITCH() __asm volatile ( "SWI 0" ::: "memory" );
#endif
