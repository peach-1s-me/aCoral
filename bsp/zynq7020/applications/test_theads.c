/**
 * @file test_theads.c
 * @author 胡博文
 * @brief 部分内核机制测试代码
 * @version 0.1
 * @date -
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡博文 <td>- <td>内容
 * <tr><td>v1.0 <td>饶洪江 <td>2025/03/17 <td>测试消息队列
 * </table>
 */
#include <acoral.h>
#include <cmd.h>
acoral_id test_thread_task[6];
acoral_id test_ipc_task[6];

void test_suspend()
{
    acoral_print("test_suspend running in cpu%d\r\n", acoral_current_cpu);

    acoral_suspend_thread(acoral_cur_thread);
    if(test_thread_task[2] > 0)
        acoral_print("test_rdy right in cpu%d\r\n", acoral_current_cpu);
    else
        acoral_print("test_suspend error in cpu%d\r\n", acoral_current_cpu);
}

void test_rdy()
{
    acoral_thread_t *test_suspend_thread;
    acoral_print("test_rdy running in cpu%d\r\n", acoral_current_cpu);
    while(test_thread_task[0]<0);
    test_suspend_thread = (acoral_thread_t *)acoral_get_res_by_id(test_thread_task[0]);
    while(!(test_suspend_thread->state&ACORAL_THREAD_STATE_SUSPEND));
    acoral_rdy_thread(test_suspend_thread);
}

void test_move()
{
    acoral_print("test_move running in cpu%d\r\n", acoral_current_cpu);
    acoral_jump_cpu(1);
    acoral_print("test_move running in cpu%d\r\n", acoral_current_cpu);
}

void test_delay_thread()
{
    acoral_print("test_delay_thread running in cpu%d\r\n", acoral_current_cpu);
    acoral_delay_ms(1000);
    acoral_print("test_delay_thread running in cpu%d\r\n", acoral_current_cpu);
}
acoral_ipc_t *my_mutex;
acoral_u8 mutex_flag[2] = {0};
void test_mutex()
{
    acoral_u32 cpu = acoral_current_cpu;
    acoral_mutex_pend(my_mutex);
    acoral_print("test_mutex get by cpu%d\r\n", cpu);
    while(!mutex_flag[cpu]);
    acoral_mutex_post(my_mutex);
}
acoral_ipc_t *my_sem;
acoral_u8 sem_flag[2] = {0};
void test_sem()
{
    acoral_u32 cpu = acoral_current_cpu;
    acoral_sem_pend(my_sem);
    acoral_print("test_sem get by cpu%d\r\n", cpu);
    while(!sem_flag[cpu]);
    acoral_sem_post(my_sem);
}

acoral_mq_t *my_mq;
void send_msg()
{
	acoral_8 send_buffer[20] = "This is a msg!!!";
    acoral_err ret;
	acoral_u32 cpu = acoral_current_cpu;
	acoral_print("[send_msg] run on cpu%d\r\n", cpu);
    while (1)
    {
    	ret = acoral_mq_send(my_mq, send_buffer, sizeof(send_buffer));
        ret = acoral_mq_send_wait(my_mq, send_buffer, sizeof(send_buffer), 20);
        if (KR_OK != ret)
        {
            acoral_print("ERRER : [%d]\r\n", ret);
        }
        acoral_print("[send_msg] send completed\r\n");
        acoral_delay_ms(1000);
    }
}
void recv_msg()
{
	acoral_8 recv_buffer[20] = "mq no msg";
    acoral_err ret;
	acoral_u32 cpu = acoral_current_cpu;
	acoral_print("[recv_msg] run on cpu%d\r\n", cpu);
    while (1)
    {
        ret = acoral_mq_recv(my_mq, recv_buffer, sizeof(recv_buffer), 50);
        if (KR_OK != ret)
        {
            acoral_print("ERRER : [%d]\r\n", ret);
        }
        acoral_print("[recv_msg] %s\r\n", recv_buffer);
        acoral_delay_ms(1000);
    }
}

acoral_dpcp_t *dpcp_sd = NULL,*dpcp_qspi = NULL;
void test_admin_ctl_1(void *args)
{
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_1 start running in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_pend(dpcp_sd);
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_1 get dpcp_sd in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_post(dpcp_sd);

    acoral_dpcp_pend(dpcp_sd);
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_1 get dpcp_sd in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_post(dpcp_sd);
}

void test_admin_ctl_2(void *args)
{
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_2 start running in cpu%d\r\n", acoral_current_cpu);
#endif

    acoral_dpcp_pend(dpcp_sd);
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_2 get dpcp_sd in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_post(dpcp_sd);

    acoral_dpcp_pend(dpcp_qspi);
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_2 get dpcp_qspi in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_post(dpcp_qspi);

    acoral_dpcp_pend(dpcp_qspi);
#ifdef CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE
    acoral_print("test_admin_ctl_2 get dpcp_qspi in cpu%d\r\n", acoral_current_cpu);
#endif
    acoral_dpcp_post(dpcp_qspi);
}

/**
 * @brief ash终端命令之add task
 *
 * @param argc 参数数目
 * @param argv 参数列表
 */
void add_task(acoral_32 argc, acoral_char **argv)
{
    acoral_admis_ctl_data *admis_ctl_data;
    acoral_admis_res_data *admis_res_data;
    static acoral_u8 task_1_first = 1,task_2_first = 1;
    if(argc == 2)//有一个参数
    {
        if(acoral_str_cmp(argv[1], "1")==0)
        {
            acoral_print("add task 1\r\n");
            if(task_1_first)
            {
                admis_ctl_data = acoral_malloc(sizeof(acoral_admis_ctl_data));
                acoral_admis_ctl_data_init(admis_ctl_data,3000,200,3000,
                                            test_admin_ctl_1,
                                            512,
                                            NULL,
                                            "test_admin_ctl_1",
                                            NULL);
                admis_res_data = acoral_malloc(sizeof(acoral_admis_res_data));
                acoral_admis_res_data_init(admis_ctl_data, admis_res_data, &dpcp_sd, 2, 50);
                acoral_have_new_thread = 1;
                task_1_first = 0;
            }
        }
        else if(acoral_str_cmp(argv[1], "2")==0)
        {
            acoral_print("add task 2\r\n");
            if(task_2_first)
            {
                admis_ctl_data = acoral_malloc(sizeof(acoral_admis_ctl_data));
                acoral_admis_ctl_data_init(admis_ctl_data,1500,137,1500,
                                            test_admin_ctl_2,
                                            512,
                                            NULL,
                                            "test_admin_ctl_2",
                                            NULL);

                admis_res_data = acoral_malloc(sizeof(acoral_admis_res_data));
                acoral_admis_res_data_init(admis_ctl_data, admis_res_data, &dpcp_sd, 1, 20);
                admis_res_data = acoral_malloc(sizeof(acoral_admis_res_data));
                acoral_admis_res_data_init(admis_ctl_data, admis_res_data, &dpcp_qspi, 2, 30);
                acoral_have_new_thread = 1;
                task_2_first = 0;
            }
        }
    }
}
/**
 * @brief add task命令结构体
 *
 */
acoral_ash_cmd_t add_task_cmd =
{
    .name = "add_task",
    .exe = add_task,
    .comment = "Add period tasks"
};

/**
 * @brief ash终端命令之asn
 *
 * @param argc 参数数目
 * @param argv 参数列表
 */
void asn(acoral_32 argc, acoral_char **argv)
{
    acoral_prints("Asnforever!!!\r\n");
}
/**
 * @brief asn命令结构体
 *
 */
acoral_ash_cmd_t asn_cmd =
{
    .name = "asn",
    .exe = asn,
    .comment = "Asnforever"
};



void test_suspend_start()
{
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_thread_task[0] = acoral_create_thread(test_suspend,
                                        512,
                                        NULL,
                                        "test_suspend",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_rdy_start()
{
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 1;
    comm_policy_data.prio = 11;
    test_thread_task[0] = acoral_create_thread(test_suspend,
                                        512,
                                        NULL,
                                        "test_suspend",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 12;
    test_thread_task[2] = acoral_create_thread(test_rdy,
                                        256,
                                        NULL,
                                        "test_rdy",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_move_start()
{
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_thread_task[4] = acoral_create_thread(test_move,
                                        512,
                                        NULL,
                                        "test_move",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_delay_thread_start()
{
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_thread_task[5] = acoral_create_thread(test_delay_thread,
                                        512,
                                        NULL,
                                        "test_delay_thread",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_mutex_start()
{
    my_mutex = acoral_mutex_create();
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_ipc_task[0] = acoral_create_thread(test_mutex,
                                        512,
                                        NULL,
                                        "test_mutex",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
    comm_policy_data.cpu = 1;
    comm_policy_data.prio = 11;
    test_ipc_task[1] = acoral_create_thread(test_mutex,
                                        512,
                                        NULL,
                                        "test_mutex",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_sem_start()
{
    my_sem = acoral_sem_create(1);
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_ipc_task[2] = acoral_create_thread(test_sem,
                                        512,
                                        NULL,
                                        "test_sem",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
    comm_policy_data.cpu = 1;
    comm_policy_data.prio = 11;
    test_ipc_task[3] = acoral_create_thread(test_sem,
                                        512,
                                        NULL,
                                        "test_sem",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}

void test_mq_start()
{
    my_mq = acoral_mq_create(4,24);
    acoral_comm_policy_data_t comm_policy_data;
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_ipc_task[4] = acoral_create_thread(send_msg,
                                        512,
                                        NULL,
                                        "send_msg",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
    comm_policy_data.cpu = 0;
    comm_policy_data.prio = 11;
    test_ipc_task[5] = acoral_create_thread(recv_msg,
                                        512,
                                        NULL,
                                        "recv_msg",
                                        NULL,
                                        ACORAL_SCHED_POLICY_COMM,
                                        &comm_policy_data,
                                        NULL,
                                        NULL);
}


void test_shell()
{
    ash_cmd_register(&asn_cmd);
}

void test_admin_ctl_start()
{
    ash_cmd_register(&add_task_cmd);
}

void test_start()
{
//    test_suspend_start();
//    test_rdy_start();
//    test_move_start();
//    test_delay_thread_start();
//    test_mutex_start();
//    test_sem_start();
    test_mq_start();
//    test_shell();
//    test_admin_ctl_start();
}
