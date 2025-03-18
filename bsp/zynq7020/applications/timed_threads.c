/**
 * @file timed_threads.c
 * @author 胡博文
 * @brief 时间确定线程示例
 * @version 0.1
 * @date 2025-02-24
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-02-24 <td>内容
 * </table>
 */

#include "timed_threads.h"
//#include "led_intr_ip.h"
#include <acoral.h>

// Task_1
void task_1_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        1,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(2000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    1,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_1_1_exec_sec[1] = {
    {
        .exe_time = 3000,
        .start_time = 0,
        .sec_func =  task_1_1
    }
};
timed_thread_config_t task_1_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 0,
    .exec_time_sections = task_1_1_exec_sec
};

// Task_2
void task_2_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        2,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(1000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    2,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_2_1_exec_sec[1] = {
    {
        .exe_time = 2000,
        .start_time = 3000,
        .sec_func =  task_2_1
    }
};
timed_thread_config_t task_2_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 0,
    .exec_time_sections = task_2_1_exec_sec
};

// Task_3
void task_3_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        3,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(500);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    3,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_3_1_exec_sec[1] = {
    {
        .exe_time = 1000,
        .start_time = 3000,
        .sec_func =  task_3_1
    }
};
timed_thread_config_t task_3_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 1,
    .exec_time_sections = task_3_1_exec_sec
};

// Task_4
void task_4_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        4,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(1000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    4,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_4_1_exec_sec[1] = {
    {
        .exe_time = 2000,
        .start_time = 5000,
        .sec_func =  task_4_1
    }
};
timed_thread_config_t task_4_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 0,
    .exec_time_sections = task_4_1_exec_sec
};

// Task_5
void task_5_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        5,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(2000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    5,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_5_1_exec_sec[1] = {
    {
        .exe_time = 3000,
        .start_time = 7000,
        .sec_func =  task_5_1
    }
};
timed_thread_config_t task_5_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 0,
    .exec_time_sections = task_5_1_exec_sec
};

// Task_6
void task_6_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        6,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(1000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==> Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    6,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_6_1_exec_sec[1] = {
    {
        .exe_time = 2000,
        .start_time = 4000,
        .sec_func =  task_6_1
    }
};
timed_thread_config_t task_6_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 1,
    .exec_time_sections = task_6_1_exec_sec
};

// Task_7
void task_7_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        7,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(3000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    7,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_7_1_exec_sec[1] = {
    {
        .exe_time = 4000,
        .start_time = 7000,
        .sec_func =  task_7_1
    }
};
timed_thread_config_t task_7_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 1,
    .exec_time_sections = task_7_1_exec_sec
};

// Task_8
void task_8_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        8,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(1000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    8,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_8_1_exec_sec[1] = {
    {
        .exe_time = 2000,
        .start_time = 10000,
        .sec_func =  task_8_1
    }
};
timed_thread_config_t task_8_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 0,
    .exec_time_sections = task_8_1_exec_sec
};

// Task_9
void task_9_1()
{
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;32mSTART\033[0m \r\n",
                        9,
                        acoral_current_cpu );
#endif
#if (MEASURE_SCHED_TIMED == 0)
    run_nop_ms(1000);
#endif
#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
    acoral_print("\033[40;35m==>Timed Task\033[0m: Func[%d]|cpu[%d]| \033[40;31mEND\033[0m \r\n",
                    9,
                    acoral_current_cpu );
#endif
}
timed_exec_section_t task_9_1_exec_sec[1] = {
    {
        .exe_time = 2000,
        .start_time = 11000,
        .sec_func =  task_9_1
    }
};
timed_thread_config_t task_9_1_tcfg = {
    .period = 40000,
    .section_num = 1,
    .frequency = 1,
    .cpu = 1,
    .exec_time_sections = task_9_1_exec_sec
};

timed_thread_config_t *timed_tcfg_list[] = {
    &task_1_1_tcfg,
    &task_2_1_tcfg,
    &task_3_1_tcfg,
    &task_4_1_tcfg,
    &task_5_1_tcfg,
    &task_6_1_tcfg,
    &task_7_1_tcfg,
    &task_8_1_tcfg,
    &task_9_1_tcfg,
};

timed_global_config_t timed_gcfg = {
    .magic_number = {0x3e,0x13,0xff,0xff},
    .hyper_period = {40000, 40000},
    .timed_thread_number = sizeof(timed_tcfg_list)/sizeof(timed_thread_config_t *)
};

void timed_threads_init(void)
{
    timed_decode_without_fs(&timed_gcfg, timed_tcfg_list);
}

