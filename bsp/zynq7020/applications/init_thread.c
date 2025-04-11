/**
 * @file init_thread.c
 * @author 胡博文
 * @brief 初始化线程
 * @version 1.1
 * @date 2025-03-27
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡博文 <td>- <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */

#include <acoral.h>
#include "xgpiops.h"
#include "xuartps.h"
//#include "led_intr_ip.h"
#include "xil_printf.h"
#include <timed.h>
#include <shell.h>
#include <cmd.h>
#include "timed_threads.h"
#include "test_threads.h"
#include "period_threads.h"
// #include <lwip_threads.h>
#include "lwip_tcp_client.h"
#include <math.h>
#include "measure.h"
#include "mmu.h"
//#define AXI_INTR_ID XPAR_FABRIC_LED_INTR_IP_0_IRQ_INTR

//acoral_u32 delay_cnt = 0;
//acoral_u8 delay_flag = 0;
//float delay_time = 0;
//
//void cal_start()
//{
//    if(delay_flag == 0)
//    {
//        delay_cnt = Xil_In32(0xF8F00200);// 0xF8F00208   0xF8F00200
//        delay_flag = 1;
//    }
//}
//
//void cal_end()
//{
//    if(delay_flag == 1)
//    {
//        delay_cnt = Xil_In32(0xF8F00200) - delay_cnt;// 0xF8F00208   0xF8F00200
//        delay_flag = 2;
//        delay_time = delay_cnt*2.0/666666687.0;
//    }
//}

void axi_gpio_handler(void *CallBackRef)
{
//    LED_INTR_IP_mWriteReg(XPAR_LED_INTR_IP_0_S1_AXI_BASEADDR,
//            LED_INTR_IP_S1_AXI_SLV_REG0_OFFSET,
//            0);
    acoral_print("fpga work done!!\r\n");
}


void init(void *args)
{
    /*系统时钟初始化*/
    acoral_ticks_init();
    
    acoral_set_sched_start(true);

    /*软件延时初始化*/
#ifdef CFG_SOFT_DELAY
    soft_delay_init();
#endif

#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
    acoral_monitor_init();
#endif
#ifdef CFG_SHELL
    acoral_ash_shell_init();
#endif

    /* 初始化mmu，！必须在网络等外设初始化前调用 */
    mmu_init();

    /* 初始化lwip demo应用 */
    // lwip_app_thread_init();
    // lwip_client_init();
    // lwip_app_thread_init();
    
//    XScuGic_Connect( &int_ctrl[0], AXI_INTR_ID, (Xil_ExceptionHandler) axi_gpio_handler, ( void * ) &int_ctrl[0] );
//    XScuGic_Enable( &int_ctrl[0], AXI_INTR_ID );
//    XScuGic_SetPriorityTriggerType(&int_ctrl[0], AXI_INTR_ID,
//                    0xA0, 0x1);

#ifdef CFG_TIMED_THREADS_ENABLE
    acoral_print("timed decode\r\n");
    // timed_decode("test.tmd", my_timed_task);//表调度解码
    timed_threads_init();
#endif

#ifdef CFG_DAG_THREADS_ENABLE
    dag_decode();
#endif
#ifdef CFG_PERIOD_THREADS_ENABLE
    period_threads_init();
#endif

#ifdef CFG_TEST_THREADS_ENABLE
    test_start();
#endif

    measure();
}
