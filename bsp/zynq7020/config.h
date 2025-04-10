/**
 * @file config.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief acoral配置文件
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
#ifndef ACORAL_CONFIG_H
#define ACORAL_CONFIG_H

/*
 * mem configuration
 */
///内存配置：伙伴系统使能
#define CFG_MEM_BUDDY
///内存配置：二级内存使能
#define CFG_MEM_VOL
///内存配置：可任意分配内存大小
#define CFG_MEM_VOL_SIZE (327680)

/*
 * thread configuration
 */
///线程配置：分组周期调度使能
#define CFG_THRD_PERIOD 1
///线程配置：硬实时优先级数量
#define CFG_HARD_RT_PRIO_NUM (0)
///线程配置：最大线程数量
#define CFG_MAX_THREAD (60)
///线程配置：最小栈空间大小
#define CFG_MIN_STACK_SIZE (128)

/*
 * event configuration
 */
///事件配置：信号量使能
#define CFG_IPC_SEM 1

/*
 * timer configuration
 */
///基石时钟配置：时钟tick与秒的转换倍率
#define CFG_TICKS_PER_SEC (1000)

/*
 * cmp configuration
 */
///多核配置
#define CFG_SMP
//#undef CFG_SMP
#ifndef CFG_SMP
///多核配置：单核
#define CFG_MAX_CPU 1
#else
///多核配置：最大cpu数量
#define CFG_MAX_CPU 2
#endif

/*
 * User configuration
 */
#if 0
///用户配置: 周期任务测试案例 Period Threads Ctrl
#define CFG_PERIOD_THREADS_ENABLE
#define CFG_PERIOD_THREADS_PRINT_ENABLE

///用户配置: DAG任务测试案例 DAG Threads Ctrl
#define CFG_DAG_THREADS_ENABLE
#define CFG_DAG_THREADS_PRINT_ENABLE

///用户配置: 时间确定性任务测试案例 Timed Threads Ctrl
#define CFG_TIMED_THREADS_ENABLE
#define CFG_TIMED_THREADS_PRINT_ENABLE


///用户配置：任务测试案例 OS Time Print Ctrl
#define CFG_OS_TICK_PRINT_ENABLE
#endif


///用户配置: 是否开启追踪线程切换信息
// #define CFG_TRACE_THREADS_SWITCH_ENABLE

#ifdef CFG_TRACE_THREADS_SWITCH_ENABLE
// #define CFG_TRACE_THREADS_SWITCH_WITH_SIM_ENABLE
#endif

#ifdef CFG_TRACE_THREADS_SWITCH_WITH_SIM_ENABLE

#ifdef CFG_PERIOD_THREADS_PRINT_ENABLE
#undef CFG_PERIOD_THREADS_PRINT_ENABLE
#endif

#ifdef CFG_DAG_THREADS_PRINT_ENABLE
#undef CFG_DAG_THREADS_PRINT_ENABLE
#endif

#ifdef CFG_TIMED_THREADS_PRINT_ENABLE
#undef CFG_TIMED_THREADS_PRINT_ENABLE
#endif

#ifdef CFG_OS_TICK_PRINT_ENABLE
#undef CFG_OS_TICK_PRINT_ENABLE
#endif

#endif

/// 开销测试相关 start
/* note: can only choose one */
#define MEASURE_CONSEXT_SWITCH      0   /* 上下文切换测试 */
#define MEASURE_MOVE_THREAD         0   /* 核间迁移测试 */
#define MEASURE_SCHEDULE            1   /* 调度测试 */

#if (MEASURE_SCHEDULE == 1)
    /* note: can only choose one */
    #define MEASURE_SCHED_PERIOD    1   /* 周期调度策略 */
    #define MEASURE_SCHED_DAG       0   /* dag调度策略 */
    #define MEASURE_SCHED_TIMED     0   /* 时间确定调度策略 */
        #if (MEASURE_SCHED_TIMED == 1)
            #define CFG_TIMED_THREADS_ENABLE
            #undef CFG_TIMED_THREADS_PRINT_ENABLE
            #undef CFG_OS_TICK_PRINT_ENABLE
        #endif /* #if (MEASURE_SCHED_TIMED == 1) */
#endif /* #if (MEASURE_SCHEDULE == 1) */

/* 保证配置选项不超过一项 */
#if \
((MEASURE_CONSEXT_SWITCH == 1) && (MEASURE_MOVE_THREAD == 1) && (MEASURE_SCHEDULE == 1)) || \
((MEASURE_CONSEXT_SWITCH == 0) && (MEASURE_MOVE_THREAD == 1) && (MEASURE_SCHEDULE == 1)) || \
((MEASURE_CONSEXT_SWITCH == 1) && (MEASURE_MOVE_THREAD == 0) && (MEASURE_SCHEDULE == 1)) || \
((MEASURE_CONSEXT_SWITCH == 1) && (MEASURE_MOVE_THREAD == 1) && (MEASURE_SCHEDULE == 0))
    #error "only one measure can be choosed"
#endif /* a lot */

#if (MEASURE_SCHEDULE == 1)
    #if \
    ((MEASURE_SCHED_PERIOD == 1) && (MEASURE_SCHED_DAG == 1) && (MEASURE_SCHED_TIMED == 1)) || \
    ((MEASURE_SCHED_PERIOD == 0) && (MEASURE_SCHED_DAG == 1) && (MEASURE_SCHED_TIMED == 1)) || \
    ((MEASURE_SCHED_PERIOD == 1) && (MEASURE_SCHED_DAG == 0) && (MEASURE_SCHED_TIMED == 1)) || \
    ((MEASURE_SCHED_PERIOD == 1) && (MEASURE_SCHED_DAG == 1) && (MEASURE_SCHED_TIMED == 0))
        #error "only one schedule measure can be choosed"
    #endif /* a lot */
#endif /* #if (MEASURE_SCHEDULE == 1) */
/* 开销测试相关 end */


//用户配置: 任务准入控制测试案例 Test Threads Ctrl
// #define CFG_TEST_THREADS_ENABLE
//#define CFG_TEST_THREADS_ADMIS_CTRL_PRINT_ENABLE


///用户配置: 准入控制功能 Thread Access Ctrl
//#define CFG_ADMIS_CTRL_ENABLE
//#define CFG_ADMIS_CTRL_PRINT_ENABLE

///用户配置：mpcp
#define CFG_MPCP
///用户配置：dpcp
#define CFG_DPCP
///用户配置：shell
// #define CFG_SHELL
//#undef CFG_SHELL
///用户配置：FIFO队列为0
#define CFG_FIFO_QUEUE 0
///用户配置：PRIO队列为1
#define CFG_PRIO_QUEUE 1
///用户配置：队列所使用的机制
#define CFG_IPC_QUEUE_MODE CFG_PRIO_QUEUE
/*
 * System hacking
 */
///标识C语言条件
#define __GNU_C__ 1

#endif
