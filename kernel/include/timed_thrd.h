/**
 * @file timed_thrd.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层时间确定性线程策略相关头文件
 * @version 1.0
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-27 <td>增加注释
 */
#ifndef KERNEL_TIMED_THRD_H
#define KERNEL_TIMED_THRD_H
#include <type.h>
///时间确定性线程栈基本大小
#define TIMED_STACK_SIZE 256
/**
 * @brief 时间确定性线程策略数据结构体
 *
 */
typedef struct{
    acoral_32 cpu;///<所在cpu
    acoral_time *start_time;///<开始执行时间
    acoral_time *exe_time;///<执行时间
    acoral_u8 section_num;///<段数量
    acoral_u8 frequency;///<执行次数
    acoral_func_point *section_route;///<段执行函数列表
    void **section_args;///<段执行函数参数
}acoral_timed_policy_data_t;
void acoral_timed_task(void *args);
#endif
