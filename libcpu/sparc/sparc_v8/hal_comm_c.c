/**
 * @file hal_comm_c.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层通用源文件，通用即与硬件细节无关
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

#include <type.h>
#include <hal_comm.h>
#include <hal_int.h>
///中断嵌套变量，每个核都有一个
acoral_u32 intr_nesting[CFG_MAX_CPU];

/**
 * @brief 中断嵌套初始化
 * 
 */
void hal_intr_nesting_init_comm(void)
{
    acoral_u32 i;
    for(i=0;i<CFG_MAX_CPU;i++)
        intr_nesting[i]=0;
}

/**
 * @brief 获取当前cpu的中断嵌套层数
 * 
 * @return acoral_u32 中断嵌套层数
 */
acoral_u32 hal_get_intr_nesting_comm(void)
{
    acoral_u32 cpu;
    cpu=HAL_GET_CURRENT_CPU();
    return intr_nesting[cpu];
}


/**
 * @brief 减少当前cpu的中断嵌套层数
 * 
 */
void hal_intr_nesting_dec_comm(void)
{
    acoral_u32 cpu;
    cpu=HAL_GET_CURRENT_CPU();
    if(intr_nesting[cpu]>0)
        intr_nesting[cpu]--;
}


/**
 * @brief 增加当前cpu的中断嵌套层数
 * 
 */
void hal_intr_nesting_inc_comm(void)
{
    acoral_u32 cpu;
    cpu=HAL_GET_CURRENT_CPU();
    intr_nesting[cpu]++;
}



