/**
 * @file ipi.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层核间中断源文件
 * @version 2.0
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-03-31 <td>规范代码风格
 */

#include "type.h"
#include "hal.h"
#include "ipi.h"
#include "cpu.h"
#include "int.h"
#include "thread.h"
#include "admis_ctl.h"
#include "xscugic.h"

#ifdef CFG_SMP
/* 核间中断命令实例 */
acoral_ipi_cmd_t acoral_ipi_cmd[CFG_MAX_CPU];
/* 核间中断状态实例 */
acoral_u8 acoral_ipi_status[CFG_MAX_CPU] = {0};

/* 核间中断的中断号 */
#define ACORAL_IPI_INT_ID 0

/* 发送SGI用到的目标cpu0mask */
#define IPI_CPU0_MASK XSCUGIC_SPI_CPU0_MASK
/* 发送SGI用到的目标cpu1mask */
#define IPI_CPU1_MASK XSCUGIC_SPI_CPU1_MASK

/**
 * @brief 核间中断发送
 * 
 * @param cpu 目标cpu
 */
void acoral_ipi_send(acoral_u32 cpu)
{
    if(cpu == 0)
    {
        /* 发送软中断 */
        XScuGic_SoftwareIntr(&int_ctrl[acoral_current_cpu], ACORAL_IPI_INT_ID, IPI_CPU0_MASK);
    }
    else if(cpu == 1)
    {
        /* 发送软中断 */
        XScuGic_SoftwareIntr(&int_ctrl[acoral_current_cpu], ACORAL_IPI_INT_ID, IPI_CPU1_MASK);
    }
}

/**
 * @brief 主核核间中断初始化
 * 
 */
void acoral_core_ipi_init()
{
    XScuGic_Connect(&int_ctrl[0], ACORAL_IPI_INT_ID, (Xil_ExceptionHandler)acoral_ipi_cmd_handler,
                    (void *)&int_ctrl[0]);
    XScuGic_Enable(&int_ctrl[0], ACORAL_IPI_INT_ID);

    acoral_ipi_status[0] = ACORAL_IPI_READY; /* 标识状态为ready */
    acoral_intr_enable();                    /* 开中断 */
    acoral_ipi_cmd_init();                   /* 核间命令初始化 */
}

/**
 * @brief 次核核间中断初始化
 * 
 */
void acoral_follow_ipi_init()
{
    XScuGic_Connect(&int_ctrl[1], ACORAL_IPI_INT_ID, (Xil_ExceptionHandler)acoral_ipi_cmd_handler,
                    (void *)&int_ctrl[1]);
    XScuGic_Enable(&int_ctrl[1], ACORAL_IPI_INT_ID);
    
    acoral_ipi_status[1] = ACORAL_IPI_READY; /* 标识状态为ready */
    acoral_intr_enable();                    /* 开中断 */
}

/**
 * @brief 核间命令初始化
 * 
 */
void acoral_ipi_cmd_init()
{
    acoral_u32 i;
    for (i = 0; i < CFG_MAX_CPU; i++)
    {
        acoral_spin_init(&acoral_ipi_cmd[i].lock);
    }
}

/**
 * @brief 核间命令发送
 * 
 * @param cpu 目标cpu
 * @param cmd 命令
 * @param thread_id 相关线程id
 * @param data 数据
 */
void acoral_ipi_cmd_send(
    acoral_u32 cpu,
    acoral_u32 cmd,
    acoral_id  thread_id,
    void      *data
)
{
    acoral_ipi_cmd_t *cmd_data;

    /* 设置命令数据 */
    if(cpu >= CFG_MAX_CPU)
    {
        cpu = CFG_MAX_CPU - 1;
    }
    cmd_data = acoral_ipi_cmd + cpu;
    acoral_spin_lock(&cmd_data->lock); /* 占用cmd数据结构锁 */
    cmd_data->cmd       = cmd;
    cmd_data->thread_id = thread_id;
    cmd_data->data      = data;
    
    acoral_ipi_send(cpu); /* 发送核间中断 */
}

/**
 * @brief 核间中断处理函数
 * 
 * @param CallBackRef 回调参数
 */
void acoral_ipi_cmd_handler(void *CallBackRef)
{
    acoral_ipi_cmd_t *cmd_data;
    acoral_u32 cmd;
    acoral_id thread_id;
    void *data;

    cmd_data  = acoral_ipi_cmd + acoral_current_cpu;
    cmd       = cmd_data->cmd;
    thread_id = cmd_data->thread_id;
    data      = cmd_data->data;
    
    acoral_spin_unlock(&cmd_data->lock); /* 释放cmd数据结构锁 */

    switch(cmd)
    {
        case ACORAL_IPI_THREAD_KILL:
            acoral_kill_thread_by_id(thread_id);
            break;
        case ACORAL_IPI_THREAD_READY:
            acoral_rdy_thread_by_id(thread_id);
            break;
        case ACORAL_IPI_THREAD_SUSPEND:
            acoral_suspend_thread_by_id(thread_id);
            break;
        case ACORAL_IPI_THREAD_CHG_PRIO:
            acoral_thread_change_prio_by_id(thread_id, (acoral_u32)data);
            break;
        case ACORAL_IPI_THREAD_MOVETO:
            acoral_moveto_thread_by_id(thread_id, (acoral_u32)data);
            break;
        case ACORAL_IPI_THREAD_MOVEIN:
            acoral_movein_thread_by_id(thread_id);
            break;
        case ACORAL_IPI_ADMIS_CHG_PRIO:
            acoral_admis_ctl_change_prio((acoral_admis_ctl_data *)data);
            break;
        case ACORAL_IPI_ADMIS_CHG_RES:
            acoral_admis_ctl_change_res((acoral_admis_res_data *)data);
            break;
        default:
            break;
    }
}

/**
 * @brief 获取核间命令结构体
 * 
 * @param cpu 目标cpu
 * @return acoral_ipi_cmd_t* 核间命令结构体
 */
acoral_ipi_cmd_t *acoral_get_ipi_cmd(acoral_u32 cpu)
{
    return acoral_ipi_cmd + cpu;
}

#endif /* ifdef CFG_SMP */

