
/**
 * @file period_threads.c
 * @author 胡博文
 * @brief 周期线程示例
 * @version 0.1
 * @date -
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡博文 <td>- <td>内容
 * </table>
 */
#include <acoral.h>

typedef struct{
    acoral_time time;///<周期时间
    void (*route)(void *args);///<线程运行函数
    void *args;///<传递参数
}period_data_t;

// Period Task 2
void period_func_2(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    2,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(500);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    2,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}

// Period Task 3
void period_func_3(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    3,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(1000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    3,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}

// Period Task 4
void period_func_4(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    4,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(500);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    4,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}
// Period Task 5
void period_func_5(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    5,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(1000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    5,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}
// Period Task 6
void period_func_6(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    6,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(1000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    6,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}

// Period Task 7
void period_func_7(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    7,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(2000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    7,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}

// Period Task 8
void period_func_8(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    8,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(1000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    8,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}

// Period Task 9
void period_func_9(void *args)
{
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;32mSTART\033[0m \r\n",
                    9,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
    run_nop_ms(3000);
#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
    acoral_print("==>Period Task: Func[%d]|cpu[%d]|prio[%d]|period[%dms]| \033[40;31mEND\033[0m \r\n",
                    9,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio,
                    ((period_data_t *)acoral_cur_thread -> private_data) -> time);
#endif
}


void period_threads_init(void){
    acoral_period_policy_data_t *period_policy_data = (acoral_period_policy_data_t *)acoral_vol_malloc(sizeof(acoral_period_policy_data_t) * 8);
    // func 2
    (&period_policy_data[0]) -> prio = 7;
    (&period_policy_data[0]) -> cpu = 1;
    (&period_policy_data[0]) -> time = 5000;   // 周期单位 ms
    acoral_create_thread(period_func_2,
                        1024,
                        NULL,
                        "period_func_2",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[0],
                        NULL,
                        NULL);
    //func 3
    (&period_policy_data[1]) -> prio = 8;
    (&period_policy_data[1]) -> cpu = 0;
    (&period_policy_data[1]) -> time = 10000;
    acoral_create_thread(period_func_3,
                        1024,
                        NULL,
                        "period_func_3",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[1],
                        NULL,
                        NULL);
    //func 4
    (&period_policy_data[2]) -> prio = 9;
    (&period_policy_data[2]) -> cpu = 1;
    (&period_policy_data[2]) -> time = 10000;
    acoral_create_thread(period_func_4,
                        1024,
                        NULL,
                        "period_func_4",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[2],
                        NULL,
                        NULL);
    // func 5
    (&period_policy_data[3]) -> prio = 10;
    (&period_policy_data[3]) -> cpu = 0;
    (&period_policy_data[3]) -> time = 20000;
    acoral_create_thread(period_func_5,
                        1024,
                        NULL,
                        "period_func_5",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[3],
                        NULL,
                        NULL);
    // func 6
    (&period_policy_data[4]) -> prio = 11;
    (&period_policy_data[4]) -> cpu = 1;
    (&period_policy_data[4]) -> time = 20000;
    acoral_create_thread(period_func_6,
                        1024,
                        NULL,
                        "period_func_6",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[4],
                        NULL,
                        NULL);
    // func 7
    (&period_policy_data[5]) -> prio = 12;
    (&period_policy_data[5]) -> cpu = 0;
    (&period_policy_data[5]) -> time = 40000;
    acoral_create_thread(period_func_7,
                        1024,
                        NULL,
                        "period_func_7",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[5],
                        NULL,
                        NULL);
    // func 8
    (&period_policy_data[6]) -> prio = 22;
    (&period_policy_data[6]) -> cpu = 1;
    (&period_policy_data[6]) -> time = 40000;
    acoral_create_thread(period_func_8,
                        1024,
                        NULL,
                        "period_func_8",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[6],
                        NULL,
                        NULL);
    // func 9
    (&period_policy_data[7]) -> prio = 23;
    (&period_policy_data[7]) -> cpu = 0;
    (&period_policy_data[7]) -> time = 60000;
    acoral_create_thread(period_func_9,
                        1024,
                        NULL,
                        "period_func_9",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_policy_data[7],
                        NULL,
                        NULL);

    acoral_vol_free(period_policy_data);
    return;
}
