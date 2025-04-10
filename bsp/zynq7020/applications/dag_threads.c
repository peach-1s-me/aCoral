/**
 * @file dag_threads.c
 * @author 胡博文
 * @brief 并行编程框架线程示例
 * @version 1.1
 * @date 2025-03-27
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡博文 <td>2025-02-24 <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */
#include <acoral.h>


// Node 1
void dag_func_1(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    1,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(2000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    1,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_1_prev[1] = {NULL};
acoral_dag_user_node dag_func_1_node =
{
    .prev_route = dag_func_1_prev,
    .prev_count = 0,
    .route = dag_func_1,
    .args = NULL,
    .processor = 1,
    .prio = 13
};

// Node 2
void dag_func_2(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    2,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(1000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    2,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_2_prev[1] = {dag_func_1};
acoral_dag_user_node dag_func_2_node =
{
    .prev_route = dag_func_2_prev,
    .prev_count = 1,
    .route = dag_func_2,
    .args = NULL,
    .processor = 1,
    .prio = 14
};

// Node 3
void dag_func_3(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    3,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(500);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    3,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_3_prev[1] = {dag_func_1};
acoral_dag_user_node dag_func_3_node =
{
    .prev_route = dag_func_3_prev,
    .prev_count = 1,
    .route = dag_func_3,
    .args = NULL,
    .processor = 0,
    .prio = 15
};

void dag_func_4(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    4,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(1000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    4,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_4_prev[1] = {dag_func_2};
acoral_dag_user_node dag_func_4_node = 
{
    .prev_route = dag_func_4_prev,
    .prev_count = 1,
    .route = dag_func_4,
    .args = NULL,
    .processor = 1,
    .prio = 16
};


// Node 5
void dag_func_5(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    5,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(2000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    5,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_5_prev[2] = {dag_func_2, dag_func_4};
acoral_dag_user_node dag_func_5_node =
{
    .prev_route = dag_func_5_prev,
    .prev_count = 2,
    .route = dag_func_5,
    .args = NULL,
    .processor = 0,
    .prio = 17
};


// Node 6
void dag_func_6(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    6,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(1000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    6,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_6_prev[1] = {dag_func_3};
acoral_dag_user_node dag_func_6_node =
{
    .prev_route = dag_func_6_prev,
    .prev_count = 1,
    .route = dag_func_6,
    .args = NULL,
    .processor = 0,
    .prio = 18
};

// Node 7
void dag_func_7(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    7,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(3000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    7,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_7_prev[1] = {dag_func_4};
acoral_dag_user_node dag_func_7_node =
{
    .prev_route = dag_func_7_prev,
    .prev_count = 1,
    .route = dag_func_7,
    .args = NULL,
    .processor = 1,
    .prio = 19
};

// Node 8
void dag_func_8(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    8,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(1000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    8,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_8_prev[2] = {dag_func_4, dag_func_5};
acoral_dag_user_node dag_func_8_node =
{
    .prev_route = dag_func_8_prev,
    .prev_count = 2,
    .route = dag_func_8,
    .args = NULL,
    .processor = 0,
    .prio = 20
};

// Node 9
void dag_func_9(void *args)
{
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;32mSTART\033[0m \r\n",
                    9,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
    run_nop_ms(1000);
#ifdef CFG_DAG_THREADS_PRINT_ENABLE
    acoral_print("\033[40;33m==>DAG Task\033[0m: Func[%d]|cpu[%d]|prio[%d]| \033[40;31mEND\033[0m \r\n",
                    9,
                    acoral_current_cpu,
                    acoral_cur_thread -> prio );
#endif
}
acoral_func_point dag_func_9_prev[2] = {dag_func_5, dag_func_6};
acoral_dag_user_node dag_func_9_node =
{
    .prev_route = dag_func_9_prev,
    .prev_count = 2,
    .route = dag_func_9,
    .args = NULL,
    .processor = 0,
    .prio = 21
};

// 设置dag任务的周期
acoral_dag_user dag_user_1 =
{
    .period_time = 40000
};


#if (MEASURE_SCHED_DAG == 0)
void dag_user_init()
{
    dag_add_user(&dag_user_1);
    dag_add_user_node(&dag_user_1, &dag_func_1_node);
    dag_add_user_node(&dag_user_1, &dag_func_2_node);
    dag_add_user_node(&dag_user_1, &dag_func_3_node);
    dag_add_user_node(&dag_user_1, &dag_func_4_node);
    dag_add_user_node(&dag_user_1, &dag_func_5_node);
    dag_add_user_node(&dag_user_1, &dag_func_6_node);
    dag_add_user_node(&dag_user_1, &dag_func_7_node);
    dag_add_user_node(&dag_user_1, &dag_func_8_node);
    dag_add_user_node(&dag_user_1, &dag_func_9_node);
}

#endif
