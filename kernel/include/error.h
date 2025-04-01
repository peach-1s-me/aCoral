/**
 * @file error.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层错误相关头文件
 * @version 2.0
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-26 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-03-31 <td>规范代码风格
 */
#ifndef KERNEL_ERROR_H
#define KERNEL_ERROR_H

/**
 * @brief 内核层错误枚举
 * 
 */
typedef enum
{
    KR_MEM_ERR_LESS = -255, /* 内存错误：内存不足 */
    KR_MEM_ERR_UNDEF,       /* 内存错误：未定义 */
    KR_MEM_ERR_MALLOC,      /* 内存错误：分配错误 */
    KR_POLICY_ERR_NULL,     /* 线程策略错误：空指针 */
    KR_POLICY_ERR_THREAD,   /* 线程策略错误：无线程 */
    KR_RES_ERR_NO_POOL,     /* 资源池错误：没有资源池 */
    KR_RES_ERR_NO_MEM,      /* 资源池错误：没有内存 */
    KR_RES_ERR_MAX_POOL,    /* 资源池错误：超过最大资源池数 */
    KR_THREAD_ERR_NO_STACK, /* 线程错误：无堆栈 */
    KR_THREAD_ERR_STATE,    /* 线程错误：线程状态不符 */
    KR_THREAD_ERR_CPU,      /* 线程错误：cpu错误 */
    KR_THREAD_ERR_PRIO,     /* 线程错误：prio错误 */
    KR_THREAD_ERR_UNDEF,    /* 线程错误：未定义 */
    KR_IPC_ERR_NULL,        /* 线程通信错误：空指针 */
    KR_IPC_ERR_TYPE,        /* 线程通信错误：类型错误 */
    KR_IPC_ERR_TASK_EXIST,  /* 线程通信错误：存在等待线程 */
    KR_IPC_ERR_INTR,        /* 线程通信错误：中断重入错误 */
    KR_IPC_ERR_UNDEF,       /* 线程通信错误：未定义 */
    KR_IPC_ERR_TIMEOUT,     /* 线程通信错误：超时 */
    KR_IPC_ERR_THREAD,      /* 线程通信错误：无线程 */
    KR_IPC_ERR_CPU,         /* 线程通信错误：cpu错误 */
    KR_OK = 0               /* OK */
} acoral_kernel_error_t;
#endif
