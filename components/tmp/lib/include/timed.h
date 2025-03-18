/**
 * @file timed.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库时间确定性调度相关头文件
 * @version 1.0
 * @date 2022-09-21
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-21 <td>增加注释
 */
#ifndef LIB_TIMED_H
#define LIB_TIMED_H
#include "type.h"
#include "config.h"

typedef struct _timed_exec_section_t
{
    acoral_u32 exe_time;///<该段执行时间数组
    acoral_u32 start_time;///<该段开始时间
    acoral_func_point sec_func;///<该段执行函数
} timed_exec_section_t;

/**
 * @brief 基于时间确定性线程数据
 * 
 */
typedef struct _timed_thread_config_t
{
    acoral_u32 period;///<周期
    acoral_u8 section_num;///<段数量
    acoral_u8 frequency;///<执行次数
    acoral_u8 time_offset;///<执行开始时间数组地址偏移量(只有使用文件解码时才使用)
    acoral_u8 cpu;///<线程所在cpu
    timed_exec_section_t *exec_time_sections; ///<每段的执行时间参数
} timed_thread_config_t;

/**
 * @brief 时间确定全局配置
 * 
 */
typedef struct _timed_global_config_t
{
    acoral_u8  magic_number[4];
    acoral_u32 hyper_period[CFG_MAX_CPU];
    acoral_u8  timed_thread_number;
} timed_global_config_t;

void timed_decode_without_fs(timed_global_config_t *g_cfg, timed_thread_config_t **t_cfg_list);
void timed_decode(char *filename, acoral_func_point **timed_task);
#endif
