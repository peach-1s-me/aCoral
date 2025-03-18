/**
 * @file timed.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库时间确定性调度相关源文件
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
#include <timed.h>
#include "ff.h"
#include <acoral.h>
#include <admis_ctl.h>
/// 时间确定性线程id指针
acoral_id *timed_task_id;
/// 时间确定性线程开始执行时间队列
acoral_queue_t acoral_timed_time[CFG_MAX_CPU];
/// 超周期
acoral_u32 acoral_timed_h_period[CFG_MAX_CPU];

/**
 * @brief 基于表的文件检测
 *
 * @param test 数组
 * @return acoral_bool 错误返回值
 */
static acoral_bool timed_test(acoral_u8 *test)
{
    if ((test[0] == 0x3e) || (test[1] == 0x13) || (test[2] == 0xff) || (test[3] == 0xff))
        return TRUE;
    else
        return FALSE;
}

void timed_decode_without_fs(timed_global_config_t *g_cfg, timed_thread_config_t **t_cfg_list)
{
    acoral_u32  max_exe_time[CFG_MAX_CPU] = {0}; /* 最大执行时间 */
    acoral_char task_name[19] = "acoral_timed_task_"; /* 默认任务名称 */
    acoral_u32 task_stack_size = 0; /* 任务栈空间大小 */
    acoral_timed_policy_data_t timed_policy_data;

    if (NULL == g_cfg || NULL == t_cfg_list)
    {
        acoral_print("ERROR: NULL pointer\r\n");
        return;
    }

    for (int i = 0; i < CFG_MAX_CPU; i++)
    {
        acoral_prio_queue_init(&acoral_timed_time[i]);
    }

    if (timed_test(g_cfg->magic_number) == FALSE)
    {
        acoral_print("ERROR: read file head fail\r\n");
        return;
    }

    // res = f_read(&fp, acoral_timed_h_period, 4*CFG_MAX_CPU, &btw);//超周期
    /* 获取每个cpu对应的超周期 */
    for (int i = 0; i < CFG_MAX_CPU; i++)
    {
        acoral_timed_h_period[i] = g_cfg->hyper_period[i];
    }

    // res = f_read(&fp, buff, 1, &btw);//获取线程数量到buff[0]
    // thread_offset = acoral_vol_malloc(buff[0]);//提前分配内存

    /* 获取线程数量 */
    acoral_u8 timed_thread_number = g_cfg->timed_thread_number;

    timed_task_id = acoral_vol_malloc(timed_thread_number * sizeof(acoral_id)); // 提前分配内存

    // res = f_lseek(&fp, 16);
    // res = f_read(&fp, thread_offset, buff[0], &btw); // 获取各线程偏移
    acoral_print("start create timed thread\r\n");
    for (int i = 0; i < timed_thread_number; i++)
    {
        // res = f_lseek(&fp, thread_offset[i]);                                // 移动到线程数据结构位置
        // res = f_read(&fp, &timed_data, sizeof(timed_thread_config_t), &btw); // 获取线程数据结构
        acoral_u8 start_time_cnt = (t_cfg_list[i])->frequency * ((t_cfg_list[i])->section_num + 1);
        timed_policy_data.cpu = (t_cfg_list[i])->cpu;
        timed_policy_data.start_time = acoral_vol_malloc(4 * start_time_cnt);
        timed_policy_data.exe_time = acoral_vol_malloc(4 * (t_cfg_list[i])->section_num);
        timed_policy_data.section_num = (t_cfg_list[i])->section_num;
        timed_policy_data.frequency = (t_cfg_list[i])->frequency;

        task_stack_size = TIMED_STACK_SIZE * ((t_cfg_list[i])->section_num + 1);

        timed_policy_data.section_route = acoral_vol_malloc(timed_policy_data.section_num * 4); // 提前分配内存
        timed_policy_data.section_args = acoral_vol_malloc(timed_policy_data.section_num * 4);  // 提前分配内存
        for (int j = 0; j < timed_policy_data.section_num; j++)                                 // 设置段函数和函数参数
        {
            timed_policy_data.section_route[j] = (t_cfg_list[i])->exec_time_sections->sec_func+j;
            timed_policy_data.section_args[j] = NULL;
        }

        // res = f_lseek(&fp, timed_data.time_offset); // 移动到执行时间数组地址

        // res = f_read(&fp, timed_policy_data.exe_time, 4 * timed_data.section_num, &btw);
        /* 获取每段执行时间 */
        for(int j=0; j<(t_cfg_list[i])->section_num; j++)
        {
            timed_policy_data.exe_time[j] = (t_cfg_list[i])->exec_time_sections->exe_time;
        }
        

        for (int j = 0; j < timed_policy_data.frequency; j++) // 开始执行时间设置
        {
            // res = f_read(&fp, timed_policy_data.start_time + (timed_data.section_num + 1) * j, 4 * timed_data.section_num, &btw);
            for(int j=0; j<(t_cfg_list[i])->section_num; j++)
            {
                timed_policy_data.start_time[((t_cfg_list[i])->section_num+1)*j] = (t_cfg_list[i])->exec_time_sections->start_time;
            }

            timed_policy_data.start_time[((t_cfg_list[i])->section_num + 1) * (j + 1) - 1] = (j + 1) * (t_cfg_list[i])->period;
            for (int k = 0; k < (t_cfg_list[i])->section_num; k++)
            {
                acoral_list_t *time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                acoral_vlist_init(time_list, timed_policy_data.start_time[((t_cfg_list[i])->section_num + 1) * j + k]);
                acoral_prio_queue_add(&acoral_timed_time[timed_policy_data.cpu], time_list);
                time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                acoral_vlist_init(time_list, timed_policy_data.start_time[((t_cfg_list[i])->section_num + 1) * j + k] + timed_policy_data.exe_time[k]);
                acoral_prio_queue_add(&acoral_timed_time[timed_policy_data.cpu], time_list);
            }
        }

        for (int j = 0; j < (t_cfg_list[i])->section_num; j++)
        {
            if (max_exe_time[timed_policy_data.cpu] < timed_policy_data.exe_time[j])
            {
                max_exe_time[timed_policy_data.cpu] = timed_policy_data.exe_time[j];
            }
        }

        task_name[18] = '0' + i; // 更改线程名字末尾
        timed_task_id[i] = acoral_create_thread(acoral_timed_task,
                                                task_stack_size,
                                                NULL,
                                                task_name,
                                                NULL,
                                                ACORAL_SCHED_POLICY_TIMED,
                                                &timed_policy_data,
                                                NULL,
                                                NULL);
        acoral_print("%ust timed thread created\r\n", i);
    }
#ifdef CFG_ADMIS_CTRL_ENABLE
    if (max_exe_time[0] <= max_exe_time[1])
        acoral_admis_ctl_init(0, max_exe_time[0]);
    else
        acoral_admis_ctl_init(1, max_exe_time[1]);
#endif
    // res = f_close(&fp);
}

/**
 * @brief 基于表的文件解码函数
 *
 * @param filename 文件名
 */
void timed_decode(char *filename, acoral_func_point **timed_task)
{
    FIL fp;
    UINT btw;
    FRESULT res;
    acoral_u8 buff[4] = {};
    acoral_u8 *thread_offset;
    acoral_u8 start_time_cnt = 0;
    timed_thread_config_t timed_data;
    acoral_timed_policy_data_t timed_policy_data;
    acoral_char task_name[19] = "acoral_timed_task_";
    acoral_char name_number = '0';
    acoral_u32 task_stack_size = 0;
    acoral_u32 max_exe_time[CFG_MAX_CPU] = {};
    acoral_list_t *time_list;

    for (int i = 0; i < CFG_MAX_CPU; i++)
    {
        acoral_prio_queue_init(&acoral_timed_time[i]);
    }

    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK)
    {
        acoral_print("ERROR: open file fail\r\n");
        return;
    }
    res = f_read(&fp, buff, 4, &btw); // 文件头测试
    if (timed_test(buff) == FALSE)
    {
        acoral_print("ERROR: read file head fail\r\n");
        return;
    }

    res = f_read(&fp, acoral_timed_h_period, 4 * CFG_MAX_CPU, &btw); // 超周期
    res = f_read(&fp, buff, 1, &btw);                                // 获取线程数量到buff[0]

    thread_offset = acoral_vol_malloc(buff[0]);                     // 提前分配内存
    timed_task_id = acoral_vol_malloc(buff[0] * sizeof(acoral_id)); // 提前分配内存

    res = f_lseek(&fp, 16);
    res = f_read(&fp, thread_offset, buff[0], &btw); // 获取各线程偏移
    acoral_print("start create timed thread\r\n");
    for (int i = 0; i < buff[0]; i++)
    {
        res = f_lseek(&fp, thread_offset[i]);                                // 移动到线程数据结构位置
        res = f_read(&fp, &timed_data, sizeof(timed_thread_config_t), &btw); // 获取线程数据结构
        start_time_cnt = timed_data.frequency * (timed_data.section_num + 1);
        timed_policy_data.cpu = timed_data.cpu;
        timed_policy_data.start_time = acoral_vol_malloc(4 * start_time_cnt);
        timed_policy_data.exe_time = acoral_vol_malloc(4 * timed_data.section_num);
        timed_policy_data.section_num = timed_data.section_num;
        timed_policy_data.frequency = timed_data.frequency;

        task_stack_size = TIMED_STACK_SIZE * (timed_data.section_num + 1);

        timed_policy_data.section_route = acoral_vol_malloc(timed_policy_data.section_num * 4); // 提前分配内存
        timed_policy_data.section_args = acoral_vol_malloc(timed_policy_data.section_num * 4);  // 提前分配内存
        for (int k = 0; k < timed_policy_data.section_num; k++)                                 // 设置段函数和函数参数
        {
            timed_policy_data.section_route[k] = (void *)timed_task[i][k];
            timed_policy_data.section_args[k] = NULL;
        }

        res = f_lseek(&fp, timed_data.time_offset); // 移动到执行时间数组地址

        res = f_read(&fp, timed_policy_data.exe_time, 4 * timed_data.section_num, &btw);

        for (int k = 0; k < timed_policy_data.frequency; k++) // 开始执行时间设置
        {
            res = f_read(&fp, timed_policy_data.start_time + (timed_data.section_num + 1) * k, 4 * timed_data.section_num, &btw);
            timed_policy_data.start_time[(timed_data.section_num + 1) * (k + 1) - 1] = (k + 1) * timed_data.period;
            for (int j = 0; j < timed_data.section_num; j++)
            {
                time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                acoral_vlist_init(time_list, timed_policy_data.start_time[(timed_data.section_num + 1) * k + j]);
                acoral_prio_queue_add(&acoral_timed_time[timed_policy_data.cpu], time_list);
                time_list = acoral_vol_malloc(sizeof(acoral_list_t));
                acoral_vlist_init(time_list, timed_policy_data.start_time[(timed_data.section_num + 1) * k + j] + timed_policy_data.exe_time[j]);
                acoral_prio_queue_add(&acoral_timed_time[timed_policy_data.cpu], time_list);
            }
        }

        for (int k = 0; k < timed_data.section_num; k++)
        {
            if (max_exe_time[timed_policy_data.cpu] < timed_policy_data.exe_time[k])
            {
                max_exe_time[timed_policy_data.cpu] = timed_policy_data.exe_time[k];
            }
        }
        name_number = i + 48;
        task_name[18] = name_number; // 更改线程名字
        timed_task_id[i] = acoral_create_thread(acoral_timed_task,
                                                task_stack_size,
                                                NULL,
                                                task_name,
                                                NULL,
                                                ACORAL_SCHED_POLICY_TIMED,
                                                &timed_policy_data,
                                                NULL,
                                                NULL);
        acoral_print("%ust timed thread created\r\n", i);
    }
#ifdef CFG_ADMIS_CTRL_ENABLE
    if (max_exe_time[0] <= max_exe_time[1])
        acoral_admis_ctl_init(0, max_exe_time[0]);
    else
        acoral_admis_ctl_init(1, max_exe_time[1]);
#endif
    res = f_close(&fp);
}
