/**
 * @file int.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层中断系统源文件
 * @version 1.2
 * @date 2025-03-28
 *
 * @copyright Copyright (c) 2022 EIC-UESTC
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 *         <tr><td>v1.1 <td>文佳源 <td>2024-09-25 <td>增加注册中断服务函数和开关中断的接口
 *         <tr><td>v1.2 <td>文佳源 <td>2025-03-28 <td>规范代码风格
 */
#include "type.h"
#include "hal.h"
#include "lsched.h"
#include "cpu.h"
#include "int.h"
#include "print.h"
#include "xscugic.h"

XScuGic        int_ctrl[CFG_MAX_CPU];       /* 中断系统控制结构体实例 */
XScuGic_Config int_gic_config[CFG_MAX_CPU]; /* 中断系统gic配置结构体实例 */

/**
 * @brief 中断系统服务函数入口
 *
 * @param ulICCIAR 中断号寄存器的值
 */
void acoral_intr_handler(acoral_u32 ulICCIAR)
{
    static const XScuGic_VectorTableEntry *vector_table = NULL; /* static 变量初始化只能为常量 */

    /* 获取当前cpu */
    acoral_u32 cpu = acoral_current_cpu;
    /* 获取当前cpu的中断向量表 */
    vector_table = int_ctrl[cpu].Config->HandlerTable;

    /* 获取中断id */
    acoral_u32 int_id = ulICCIAR & 0x3FFUL;

    const XScuGic_VectorTableEntry *vector_entry = NULL; /* 向量表成员指针 */
    if (int_id < XSCUGIC_MAX_NUM_INTR_INPUTS)
    {
        /* 获取中断结构体 */
        vector_entry = &(vector_table[int_id]);
        /* 执行中断服务函数 */
        vector_entry->Handler(vector_entry->CallBackRef);
    }
}

/**
 * @brief 中断配置初始化函数
 *
 * @param cpu 目标cpu
 */
void acoral_intr_init(acoral_u32 cpu)
{
    /* 获取官方gic配置，主要是一些寄存器地址 */
    XScuGic_Config *gic_config = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);

    /* 根据配置设置中断实例参数 */
    int_gic_config[cpu].CpuBaseAddress  = gic_config->CpuBaseAddress;
    int_gic_config[cpu].DeviceId        = gic_config->DeviceId;
    int_gic_config[cpu].DistBaseAddress = gic_config->DistBaseAddress;

    /* 初始化中断系统 */
    XScuGic_CfgInitialize(&int_ctrl[cpu], &int_gic_config[cpu], int_gic_config[cpu].CpuBaseAddress);
}

/**
 * @brief 注册中断回调函数
 *
 * @param  cpu              cpu号
 * @param  intr_id          中断号
 * @param  callback         中断回调函数
 * @param  callback_arg     回调函数的参数
 */
void acoral_intr_callback_register(acoral_u32 cpu, acoral_u32 intr_id, acoral_intr_callback_t callback, void *callback_arg)
{
    if (NULL == callback)
    {
        acoral_print("ERROR: null callback ptr\r\n");
        while (1);
    }
    int_gic_config[cpu].HandlerTable[intr_id].Handler     = callback;
    int_gic_config[cpu].HandlerTable[intr_id].CallBackRef = callback_arg;
}

/**
 * @brief 根据中断号开启中断
 *
 * @param  cpu              cpu号
 * @param  intr_id          中断号
 */
void acoral_intr_enable_by_id(acoral_u32 cpu, acoral_u32 intr_id)
{
    XScuGic_EnableIntr(int_gic_config[cpu].DistBaseAddress, (u32)intr_id);
}

/**
 * @brief 根据中断号关闭中断
 *
 * @param  cpu              cpu号
 * @param  intr_id          中断号
 */
void acoral_intr_disable_by_id(acoral_u32 cpu, acoral_u32 intr_id)
{
    XScuGic_DisableIntr(int_gic_config[cpu].DistBaseAddress, (u32)intr_id);
}

/**
 * @brief 中断系统初始化函数
 *
 */
void acoral_intr_sys_init(void)
{
    /* 关中断 */
    acoral_intr_disable();

    /* 中断嵌套标志初始化 */
    HAL_INTR_NESTING_INIT();

    /* 中断底层初始化函数 */
    acoral_intr_init(acoral_current_cpu);
}

/**
 * @brief 中断入口函数
 *
 * @param ulICCIAR 中断号寄存器的值
 */
void acoral_intr_entry(acoral_u32 ulICCIAR)
{
    acoral_intr_nesting_inc();     /* 增加中断嵌套计数 */
    acoral_intr_enable();          /* 开启中断，允许重入 */

    acoral_intr_handler(ulICCIAR); /* 执行中断服务函数 */

    acoral_intr_disable();         /* 关闭中断 */
    acoral_intr_nesting_dec();     /* 减少中断嵌套计数 */

    acoral_intr_exit();            /* 查看是否需要调度 */
}

/**
 * @brief 中断退出函数
 *
 */
void acoral_intr_exit()
{
    if (acoral_sched_enable  && acoral_need_sched && !acoral_intr_nesting  &&
       !acoral_sched_is_lock)
    {
        hal_irq_switch[acoral_current_cpu] = 1;
    }
}
