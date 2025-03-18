/**
 * @file measure.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试系统开销
 * @version 0.1
 * @date 2024-07-05
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2024-07-05 <td>上下文切换开销测试
 * <tr><td>v1.0 <td>饶洪江 <td>2024-07-07 <td>核间迁移开销测试
 * <tr><td>v1.0 <td>文佳源 <td>2024-07-07 <td>调度开销测试-x
 * <tr><td>v2.0 <td>文佳源 <td>2024-07-10 <td>使用定时器重做
 * </table>
 */
#include <acoral.h>
#include "measure.h"
#include "calculate_time.h"

#define MEASURE_TASK_PRIO           ACORAL_MAX_PRIO + 1
#define MEASURE_TASK_STACK_SIZE     512

/* 输出测量结果 */
void measure_done(const char *measure_name, during_buffer_t *buf)
{
    acoral_print("measure %s done\r\n", measure_name);

    acoral_print("record %u data, ", buf->record_num);

    switch (buf->time_unit)
    {
        case TIME_UNIT_US:
        {
            printf("time unit is us\r\n");
            break;
        }
        case TIME_UNIT_MS:
        {
            printf("time unit is ms\r\n");
            break;
        }
        default:
        {
            printf("ERROR: unknown time unit\r\n");
            break;
        }
    }

    acoral_u32 i = 0;
    for (i = 0; i < SINGLE_MEASURE_MAX_TIME; i++)
    {
        if(buf->during[i] != 0)
        {
            acoral_print("%u\t%u\r\n", i, buf->during[i]);
        }
    }
}

/* 保存测量的数据 */
acoral_err push_during(during_buffer_t *buf, double during)
{
    // switch_during_array[switch_during_idx++] = during;
    switch (buf->time_unit)
    {
        case TIME_UNIT_US:
        {
            during *= 1000;
            break;
        }
        case TIME_UNIT_MS:
        {
            break;
        }
        default:
        {
            printf("ERROR: unknown time unit\r\n");
            break;
        }
    }
    
    if((acoral_32)during < SINGLE_MEASURE_MAX_TIME)
    {
        buf->during[(acoral_32)during]++;
        buf->record_num++;
    }
    else
    {
        printf("ERROR: during(%f) bigger than SINGLE_MEASURE_MAX_TIME(%u)\r\n", during,SINGLE_MEASURE_MAX_TIME);
    }
}

#if (MEASURE_CONSEXT_SWITCH == 1)

#define SWITCH_TIMES            100000

extern acoral_u8 start_measure_context_switch;

during_buffer_t context_switch_buffer = {
    .during = {0},
    .record_num = 0,
    .time_unit = TIME_UNIT_US
};

acoral_id js0_id = -1;
acoral_id js1_id = -1;

void just_switch_0(void *args)
{ 
    acoral_print("js0 enter\r\n");
    while(1)
    {
        if(context_switch_buffer.record_num == SWITCH_TIMES)
        {
            start_measure_context_switch = 0;
            measure_done("context switch", &context_switch_buffer);
            break;
        }

        acoral_enter_critical();
        acoral_thread_change_prio_self(MEASURE_TASK_PRIO + 1);
        acoral_thread_change_prio_by_id(js1_id, MEASURE_TASK_PRIO);
        acoral_exit_critical();
        if(1 == start_measure_context_switch)
        {
            double during = cal_time_end();
            if(during > 0)
            {
                push_during(&context_switch_buffer, during);   
            }
        }
    }
}

void just_switch_1(void *args)
{
    acoral_print("js1 enter\r\n");
    while(1)
    {
        if(context_switch_buffer.record_num == SWITCH_TIMES)
        {
            start_measure_context_switch = 0;
            measure_done("context switch", &context_switch_buffer);
            break;
        }

        acoral_enter_critical();
        acoral_thread_change_prio_self(MEASURE_TASK_PRIO + 1);
        acoral_thread_change_prio_by_id(js0_id, MEASURE_TASK_PRIO);
        acoral_exit_critical();
        if(1 == start_measure_context_switch)
        {
            double during = cal_time_end();
            if(during > 0)
            {
                push_during(&context_switch_buffer, during);   
            }
        }
    }
}

void measure_consext_switch(void)
{
    acoral_print("[measure] create measure threads\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu=acoral_current_cpu;
    p_data.prio=MEASURE_TASK_PRIO;
    js0_id=acoral_create_thread(
                just_switch_0,
                MEASURE_TASK_STACK_SIZE,
                NULL,
                "just_switch_0",
                NULL,
                ACORAL_SCHED_POLICY_COMM,
                &p_data,
                NULL,
                NULL
            );
    if(js0_id==-1)
    {
        while(1);
    }
    p_data.cpu=acoral_current_cpu;
    p_data.prio=MEASURE_TASK_PRIO;
    js1_id=acoral_create_thread(
                just_switch_1,
                MEASURE_TASK_STACK_SIZE,
                NULL,
                "just_switch_1",
                NULL,
                ACORAL_SCHED_POLICY_COMM,
                &p_data,
                NULL,
                NULL
            );
    if(js1_id==-1)
    {
        while(1);
    }    

    start_measure_context_switch = 1;
}

#endif /* #if (MEASURE_CONSEXT_SWITCH == 1) */

#if (MEASURE_MOVE_THREAD == 1)

#define MOVE_TIMES            100000

during_buffer_t move_buffer = {
    .during = {0},
    .record_num = 0,
    .time_unit = TIME_UNIT_US
};

acoral_id ep_id = -1;
acoral_id mt_id = -1;

void empty_entry(void *args)
{
	acoral_print("Empty thread start running\r\n");
	while(1);
}

void move_thread(void *args)
{
    acoral_print("measure move enter\r\n");
    while(1)
    {
        if (move_buffer.record_num == MOVE_TIMES)
        {
            measure_done("move thread", &move_buffer);
            break;
        }

        acoral_enter_critical();
        if(((acoral_thread_t *)acoral_get_res_by_id(ep_id))->cpu == 0)
        {
            cal_time_start();
        	acoral_moveto_thread_by_id(ep_id, 1);
        }
        else
        {
            cal_time_start();
        	acoral_moveto_thread_by_id(ep_id, 0);
        }
        acoral_exit_critical();
        double during = cal_time_end();
        if (during > 0)
        {
            push_during(&move_buffer, during);
        }
    }
}

void measure_move_thread()
{
    acoral_print("[measure move] create empty thread, which is used to be moved\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu = 1;
    p_data.prio = MEASURE_TASK_PRIO + 1;
    ep_id = acoral_create_thread(
    			empty_entry,
                MEASURE_TASK_STACK_SIZE,
                NULL,
                "empty_task",
                NULL,
                ACORAL_SCHED_POLICY_COMM,
                &p_data,
                NULL,
                NULL
            );
    if(ep_id == -1)
    {
        while(1);
    }
    acoral_print("[measure] create measure threads\r\n");
    p_data.cpu = 0;
    p_data.prio = MEASURE_TASK_PRIO;
    mt_id=acoral_create_thread(
    		    move_thread,
                MEASURE_TASK_STACK_SIZE,
                NULL,
                "move_thread",
                NULL,
                ACORAL_SCHED_POLICY_COMM,
                &p_data,
                NULL,
                NULL
            );
    if(mt_id == -1)
    {
        while(1);
    }
}
#endif /* #if (MEASURE_MOVE_THREAD == 1) */

#if (MEASURE_SCHEDULE == 1)

    #define SCHEDULE_TIMES            10000

    during_buffer_t sched_buffer = {
        .during = {0},
        .record_num = 0,
        .time_unit = TIME_UNIT_US
    };

    void wait_measure_schedule(void *args)
    {
        acoral_print("wait measure schedule enter\r\n");
        while(1)
        {
            if (sched_buffer.record_num >= SCHEDULE_TIMES)
            {
                measure_done("schedule", &sched_buffer);
                break;
            }
        }
    }

    #if (MEASURE_SCHED_PERIOD == 1)
    void empty_period_entry(void *args)
    {

    }

    #elif (MEASURE_SCHED_DAG == 1)

    acoral_u8 measure_dag_done = 0;

    void empty_dag1_entry(void *args)
    { 
        // acoral_print("empty dag 1\r\n"); 
#if (MEASURE_SCHED_DAG == 1)
        cal_time_start();
#endif
        }
    acoral_func_point empty_dag1_prev[1] = {NULL};
    acoral_dag_user_node empty_dag1_node =
    {
        .prev_route = empty_dag1_prev,
        .prev_count = 0,
        .route = empty_dag1_entry,
        .args = NULL,
        .processor = 1,
        .prio = MEASURE_TASK_PRIO
    };
    void empty_dag2_entry(void *args)
    { 
        // acoral_print("empty dag 2\r\n"); 
#if (MEASURE_SCHED_DAG == 1)
        extern during_buffer_t sched_buffer;
        double during = cal_time_end();
        // printf("%u %f\r\n", sched_buffer.record_num, during);
        if (during > 0)
        {
            push_during(&sched_buffer, during);
            if(
                (sched_buffer.record_num == SCHEDULE_TIMES) &&
                (measure_dag_done == 0)
            )
            {
                measure_dag_done = 1;
                measure_done("measure schedule dag", &sched_buffer);
            }
        }
#endif

    }
    acoral_func_point empty_dag2_prev[1] = {empty_dag1_entry};
    acoral_dag_user_node empty_dag2_node =
    {
        .prev_route = empty_dag2_prev,
        .prev_count = 0,
        .route = empty_dag2_entry,
        .args = NULL,
        .processor = 0,
        .prio = MEASURE_TASK_PRIO
    };


    // 设置dag任务的周期
    acoral_dag_user empty_dag_user =
    {
        .period_time = 5
    };

    void dag_user_init()
    {
        dag_add_user(&empty_dag_user);
        dag_add_user_node(&empty_dag_user, &empty_dag1_node);
        dag_add_user_node(&empty_dag_user, &empty_dag2_node);
    }

    #elif (MEASURE_SCHED_TIMED == 1)

#else
        #error "one schedule policy should be choosed"
    #endif

void measure_schedule(void)
{
    acoral_print("[measure] create schedule measure threads\r\n");

    #if (MEASURE_SCHED_PERIOD == 1)
        acoral_comm_policy_data_t p_data;
        p_data.cpu = acoral_current_cpu;
        p_data.prio = MEASURE_TASK_PRIO + 1;
        acoral_id wmc_id = acoral_create_thread(
            wait_measure_schedule,
            MEASURE_TASK_STACK_SIZE,
            NULL,
            "wait_measure",
            NULL,
            ACORAL_SCHED_POLICY_COMM,
            &p_data,
            NULL,
            NULL);
        if (wmc_id == -1)
        {
            while (1);
        }

        #define PERIOD_THREAD_NUM 5

        acoral_id epp_id[PERIOD_THREAD_NUM] = {-1};

        acoral_print("measure period schedule\r\n");
        acoral_period_policy_data_t period_priv_data;
        period_priv_data.cpu=1;
        period_priv_data.prio=MEASURE_TASK_PRIO;
        period_priv_data.time=2;
        acoral_u8 i=0;
        for(i=0; i<PERIOD_THREAD_NUM; i++)
        {
            epp_id[i]=acoral_create_thread(
                        empty_period_entry,
                        MEASURE_TASK_STACK_SIZE,
                        NULL,
                        "empty_period",
                        NULL,
                        ACORAL_SCHED_POLICY_PERIOD,
                        &period_priv_data,
                        NULL,
                        NULL
                    );
            if(epp_id[i]==-1)
            {
                while(1);
            }
        }
    #elif (MEASURE_SCHED_DAG == 1)
        acoral_print("measure dag schedule\r\n");
        dag_decode();
        
    #elif (MEASURE_SCHED_TIMED == 1)
        acoral_print("measure timed schedule\r\n");
        acoral_comm_policy_data_t p_data;
        p_data.cpu = acoral_current_cpu;
        p_data.prio = MEASURE_TASK_PRIO + 1;
        acoral_id wmc_id = acoral_create_thread(
            wait_measure_schedule,
            MEASURE_TASK_STACK_SIZE,
            NULL,
            "wait_measure",
            NULL,
            ACORAL_SCHED_POLICY_COMM,
            &p_data,
            NULL,
            NULL);
        if (wmc_id == -1)
        {
            while (1);
        }
    #else
        #error "one schedule policy should be choosed"
    #endif
}
#endif /* #if (MEASURE_SCHEDULE == 1) */

void measure(void)
{
#if (MEASURE_CONSEXT_SWITCH == 1)
    acoral_print("== test context switch ==\r\n");
    measure_consext_switch();
#endif /* #if (MEASURE_CONSEXT_SWITCH == 1) */

#if (MEASURE_MOVE_THREAD == 1)
    acoral_print("== test move thread ==\r\n");
    measure_move_thread();
#endif /* #if (MEASURE_MOVE_THREAD == 1) */

#if (MEASURE_SCHEDULE == 1)
    acoral_print("== test schedule ==\r\n");
    measure_schedule();
#endif /* #if (MEASURE_SCHEDULE == 1) */
}
