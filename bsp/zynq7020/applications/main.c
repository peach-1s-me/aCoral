/**
 * @file main.c
 * @author 胡博文
 * @brief 主函数
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
#include "xparameters.h"
#include "xgpiops.h"
#include "xuartps.h"
#include "xscutimer.h"
#include "xscugic.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#include <print.h>
#include "ff.h"
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID
#define LED0	7
#define LED1	8
#define LED2	0
#define KEY0	12
#define KEY1	11
XGpioPs Gpio;

void mio_init()
{
    XGpioPs_Config *ConfigPtr;
    ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
    XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
                    ConfigPtr->BaseAddr);

    XGpioPs_SetDirectionPin(&Gpio, LED0, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, LED0, 1);
    XGpioPs_SetDirectionPin(&Gpio, LED1, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, LED1, 1);
    XGpioPs_SetDirectionPin(&Gpio, LED2, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, LED2, 1);
    XGpioPs_SetDirectionPin(&Gpio, KEY0, 0);
    XGpioPs_SetDirectionPin(&Gpio, KEY1, 0);

    XGpioPs_WritePin(&Gpio, LED0, 1);
    XGpioPs_WritePin(&Gpio, LED1, 1);
    XGpioPs_WritePin(&Gpio, LED2, 1);

}

FATFS fatfs;
FIL fp;
UINT btw;
FRESULT res;

acoral_u8 test[4] = {0x3e,0x13,0xff,0xff};
//typedef struct
//{
//    acoral_u32 h_period[2];///<超周期
//    acoral_u8 thread_count;///<线程数量
//
//    acoral_u8 thread_offset_1;///<线程1数据结构地址偏移量
//
//    acoral_u8 thread_offset_2;///<线程2数据结构地址偏移量
//
//    acoral_u32 period_1;///<线程周期
//    acoral_u8 section_num_1;///<段数量
//    acoral_u8 frequency_1;///<执行次数
//    acoral_u8 time_offset_1;///<开始时间指针地址偏移
//    acoral_u8 cpu_1;///<线程所在cpu
//
//    acoral_u32 period_2;///<线程周期
//    acoral_u8 section_num_2;///<段数量
//    acoral_u8 frequency_2;///<执行次数
//    acoral_u8 time_offset_2;///<执行开始时间数组地址偏移量
//    acoral_u8 cpu_2;///<线程所在cpu
//
//    acoral_u32 exe_time_1[2];///<线程1执行时间数组
//    acoral_u32 start_time_1[2];///<线程1执行开始时间数组
//
//    acoral_u32 exe_time_2[2];///<线程2执行时间数组
//    acoral_u32 start_time_2[8];///<线程2执行结束时间数组
//}send_t;
//
//send_t my_write_data =
//{
//    .h_period = {12000,12000},
//    .thread_count = 2,
//    .thread_offset_1 = 16,
//    .thread_offset_2 = 24,
//
//    .period_1 = 12000,
//    .section_num_1 = 2,
//    .frequency_1 = 1,
//    .time_offset_1 = 32,
//    .cpu_1 = 0,
//
//    .period_2 = 3000,
//    .section_num_2 = 2,
//    .frequency_2 = 4,
//    .time_offset_2 = 48,
//    .cpu_2 = 1,
//
//    .exe_time_1 = {1000,500},
//    .start_time_1 = {5000,7000},
//
//    .exe_time_2 = {100,300},
//    .start_time_2 = {1000,2000,4000,5500,6500,8000,10000,11000}
//};
acoral_u32 rev_time[6] = {};


typedef struct {
    acoral_u32 period;///<线程周期
    acoral_u8 section_num;///<段数量
    acoral_u8 frequency;///<执行次数
    acoral_u8 time_offset;///<开始时间指针地址偏移
    acoral_u8 cpu;///<线程所在cpu
} timed_thrd_section;

typedef struct {
    acoral_u32 exe_time;///<线程执行时间数组
    acoral_u32 start_time;
} exec_time_section;

typedef struct
{
    acoral_u32 h_period[2];///<超周期
    acoral_u8 thread_count;///<线程数量
    acoral_u8 s_nol[3]; ///<对齐占位
    acoral_u8 thread_offsets[9];///<线程数据结构地址偏移量
    timed_thrd_section thrd_sections[9];
    exec_time_section time_section[9];
} new_send_t;

#if (MEASURE_SCHED_TIMED == 0)
new_send_t my_write_data = {
        {40000, 40000}, // 每个核心都有一个超周期
        9,
        {0,0,0},
        {28, 36, 44, 52, 60, 68, 76, 84, 92},
        {
                {// 1
                        40000,
                        1,
                        1,
                        100,
                        0,
                },
                {// 2
                        40000,
                        1,
                        1,
                        108,
                        0,
                },
                {// 3
                        40000,
                        1,
                        1,
                        116,
                        1,
                },
                {// 4
                        40000,
                        1,
                        1,
                        124,
                        0,
                },
                {// 5
                        40000,
                        1,
                        1,
                        132,
                        0,
                },
                {// 6
                        40000,
                        1,
                        1,
                        140,
                        1,
                },
                {// 7
                        40000,
                        1,
                        1,
                        148,
                        1,
                },
                {// 8
                        40000,
                        1,
                        1,
                        156,
                        0,
                },
                {// 9
                        40000,
                        1,
                        1,
                        164,
                        1,
                }
        },
        {
                {// 1
                        3000,
                        0
                },
                {// 2
                        2000,
                        3000
                },
                {// 3
                        1000,
                        3000
                },
                {// 4
                        2000,
                        5000
                },
                {// 5
                        3000,
                        7000
                },
                {// 6
                        2000,
                        4000
                },
                {// 7
                        4000,
                        7000
                },
                {// 8
                        2000,
                        10000
                },
                {// 9
                        2000,
                        11000
                }
        }
};
#else
new_send_t my_write_data = {
        {400, 400}, // 每个核心都有一个超周期
        9,
        {0,0,0},
        {28, 36, 44, 52, 60, 68, 76, 84, 92},
        {
                {// 1
                        400,
                        1,
                        1,
                        100,
                        0,
                },
                {// 2
                        400,
                        1,
                        1,
                        108,
                        0,
                },
                {// 3
                        400,
                        1,
                        1,
                        116,
                        1,
                },
                {// 4
                        400,
                        1,
                        1,
                        124,
                        0,
                },
                {// 5
                        400,
                        1,
                        1,
                        132,
                        0,
                },
                {// 6
                        400,
                        1,
                        1,
                        140,
                        1,
                },
                {// 7
                        400,
                        1,
                        1,
                        148,
                        1,
                },
                {// 8
                        400,
                        1,
                        1,
                        156,
                        0,
                },
                {// 9
                        400,
                        1,
                        1,
                        164,
                        1,
                }
        },
        {
                {// 1
                        30,
                        0
                },
                {// 2
                        20,
                        30
                },
                {// 3
                        10,
                        30
                },
                {// 4
                        20,
                        50
                },
                {// 5
                        30,
                        70
                },
                {// 6
                        20,
                        40
                },
                {// 7
                        40,
                        70
                },
                {// 8
                        20,
                        100
                },
                {// 9
                        20,
                        110
                }
        }
};
#endif




int main()
{
    mio_init();
    // f_mount(&fatfs, "", 0);
    // res = f_open(&fp, "test.tmd",FA_WRITE|FA_CREATE_ALWAYS);
    // res = f_write(&fp,test, 4,&btw);
    // res = f_lseek(&fp, 4);
    // res = f_write(&fp,&my_write_data,sizeof(new_send_t),&btw);
    // res = f_close(&fp);
    // acoral_print("my data written\r\n");
    acoral_start();
    
    while(1)
    {

    }
    return 0;
}
