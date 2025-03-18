
/**
 * @file monitor.c
 * @author 高久强
 * @brief 线程切换追踪信息
 * @version 0.1
 * @date 2025-02-24
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>高久强 <td>2025-02-24 <td>内容
 * </table>
 */
#include "monitor.h"
#include "lib.h"
//#include "acosim.pb.h"
//#include "pb_encode.h"

acoral_spinlock_t tinfos_lock;
acoral_spinlock_t tswinfos_lock;
acoral_spinlock_t send_lock;

thread_info_set tinfos_real = {
        .total_size = CFG_MAX_THREAD,
        .st_idx = 0,
        .ed_idx = 0,
        .info_type = MONITOR_CMD_CREATE,
        .size = 0,
        .item_bsize = sizeof(thread_info),
        .overflow = 0,
};
thread_switch_info_set tswinfos_real = {
        .total_size = CFG_MAX_THREAD,
        .threshold = CFG_MAX_THREAD/2,
        .st_idx = 0,
        .ed_idx = 0,
        .info_type = MONITOR_CMD_SWITCH,
        .size = 0,
        .item_bsize = sizeof(thread_info),
        .overflow = 0
};

thread_info_set *tinfos = &tinfos_real;
thread_switch_info_set *tswinfos = &tswinfos_real;

#ifdef CFG_TRACE_THREADS_SWITCH_WITH_SIM_ENABLE
typedef struct {
    char title[4];
    acoral_u32 type;
    acoral_u32 length;
} header_info;
static header_info header;
#endif

void acoral_monitor_init(void) {
#ifdef CFG_TRACE_THREADS_SWITCH_WITH_SIM_ENABLE
    for (int i = 0; i < 4; i++) {
        header.title[i] = '+';
    }
    header.length = 0;
    header.type = 0;
#endif
    // 初始化自旋锁
    acoral_spin_init(&tinfos_lock);
    acoral_spin_init(&tswinfos_lock);
    acoral_spin_init(&send_lock);
//     初始化串口输出线程
    acoral_period_policy_data_t *period_policy_data = (acoral_period_policy_data_t *)acoral_vol_malloc(sizeof(acoral_period_policy_data_t));
    period_policy_data -> prio = 8;
    period_policy_data -> cpu = 0;
    period_policy_data -> time = 10000;
    acoral_create_thread(acoral_infos_send,
                            1024,
                            NULL,
                            "acoral_infos_send",
                            NULL,
                            ACORAL_SCHED_POLICY_PERIOD,
                            period_policy_data,
                            NULL,
                            NULL);
    acoral_vol_free(period_policy_data);
    // 暂时先不写
}

/**
 * @brief 添加线程信息
 * 
 * @param  thread           线程指针
 */
void acoral_tinfos_add(acoral_thread_t* thread) {
//    acoral_enter_critical();
//    acoral_spin_lock(&tinfos_lock);
    // 插入数据操作
    tinfos -> threads[tinfos -> ed_idx].tcb_ptr = thread;
    tinfos -> threads[tinfos -> ed_idx].thread_policy = thread -> policy;
    tinfos -> threads[tinfos -> ed_idx].prio = thread -> prio;
    switch(thread -> policy) {
        case ACORAL_SCHED_POLICY_COMM:
            tinfos -> threads[tinfos -> ed_idx].period = 0;
            tinfos -> threads[tinfos -> ed_idx].supercycle = 0;
            break;
        case ACORAL_SCHED_POLICY_PERIOD:
            tinfos -> threads[tinfos -> ed_idx].period = ((acoral_period_policy_data_t *)(thread -> private_data)) -> time;
            tinfos -> threads[tinfos -> ed_idx].supercycle = 40000;   // ms
        case ACORAL_SCHED_POLICY_TIMED:
            tinfos -> threads[tinfos -> ed_idx].period = 40000;
            tinfos -> threads[tinfos -> ed_idx].supercycle = 40000;
        default:
            break;
    }
    tinfos -> threads[tinfos -> ed_idx].cpu = thread -> cpu;
    tinfos -> threads[tinfos -> ed_idx].create_tick = acoral_ticks;
    acoral_str_cpy(&((tinfos -> threads[tinfos -> ed_idx]).thread_name[0]), thread -> name);

    if(tinfos -> ed_idx < tinfos-> total_size - 1 ) {
        tinfos -> ed_idx ++;
        tinfos -> size ++;
    } else {
        tinfos -> ed_idx = 0;
        tinfos -> overflow ++;
        if(tinfos -> size < tinfos -> total_size) {
            tinfos -> size ++;
        }
    }
//    acoral_spin_unlock(&tinfos_lock);
//    acoral_exit_critical();

}

/**
 * @brief 线程切换信息添加
 * 
 * @param  from             被切换的线程指针
 * @param  to               要切换到的线程指针
 */
void acoral_tswinfos_add(acoral_thread_t* from, acoral_thread_t* to) {
//    acoral_enter_critical();
//    acoral_spin_lock(&tswinfos_lock);
    /* 填充任务切换信息 */
    tswinfos -> switch_infos[tswinfos -> ed_idx].cpu = acoral_current_cpu;
    tswinfos -> switch_infos[tswinfos -> ed_idx].tick = acoral_ticks;
    tswinfos -> switch_infos[tswinfos -> ed_idx].from_tcb = from;
    tswinfos -> switch_infos[tswinfos -> ed_idx].to_tcb = to;
//    acoral_str_cpy(&(tswinfos -> switch_infos[tswinfos -> ed_idx].from_name[0]), from -> name);
//    acoral_str_cpy(&(tswinfos -> switch_infos[tswinfos -> ed_idx].to_name[0]), to -> name);
    if(tswinfos -> ed_idx < tswinfos -> total_size - 1 ) {
        tswinfos -> ed_idx ++;
        tswinfos -> size ++;
    } else {
        tswinfos -> ed_idx = 0;
        tswinfos -> overflow ++;
        tswinfos -> size ++;
    }

//    acoral_spin_unlock(&tswinfos_lock);
//    acoral_exit_critical();
}


#ifdef CFG_TRACE_THREADS_SWITCH_WITH_SIM_ENABLE

/**
 * @brief 发送字节数据
 * 
 * @param  data             数据源指针
 * @param  size             字节数
 */
static void acoral_bytes_send(char * data, acoral_u32 size) {
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&send_lock);
#endif
    /* 发送数据头 */
    for(int i = 0;i < sizeof(header); i++){
        outbyte(*((char *)&header + i));
    }
    /* 发送数据 */
    while(size > 0) {
        outbyte(*data);
        data++;
        size--;
    }
//    outbyte('\n');

#ifdef CFG_SMP
    acoral_spin_unlock(&send_lock);
#endif
   acoral_exit_critical();
}

/**
 * @brief 发送任务和切换信息
 * 
 */
void acoral_infos_send() {
    static acoral_u32 symbel = 0;
    acoral_u32 print_num;
    thread_info_set *tinfos_tmp;
    thread_switch_info_set *tswinfos_tmp;
    tinfos_tmp = (thread_info_set *)acoral_vol_malloc(sizeof(thread_info_set));
    tswinfos_tmp = (thread_switch_info_set *)acoral_vol_malloc(sizeof(thread_switch_info_set));


    acoral_enter_critical();
//    acoral_spin_lock(&tinfos_lock);
//    acoral_spin_lock(&tswinfos_lock);
    *tinfos_tmp = *tinfos;
    *tswinfos_tmp = *tswinfos;
    tinfos -> size = 0;
    tinfos -> st_idx = 0;
    tinfos -> ed_idx = 0;
    tswinfos -> size = 0;
    tswinfos -> st_idx = 0;
    tswinfos -> ed_idx = 0;
//    acoral_spin_unlock(&tswinfos_lock);
//    acoral_spin_unlock(&tinfos_lock);
    acoral_exit_critical();


    // 发送核心的相关信息
    if (symbel == 0) {
        symbel = 1;
        header.type = 1;
        header.length = sizeof(CPUcore_info);
        CPUcore_info * CPUcore_info_tmp = (CPUcore_info *)acoral_vol_malloc(sizeof(CPUcore_info));
        CPUcore_info_tmp -> id = 0;
        CPUcore_info_tmp -> cpuFrequency = 666000000;
        acoral_str_cpy(&(CPUcore_info_tmp -> name[0]), "CPU 0");

        acoral_bytes_send((char *)CPUcore_info_tmp, sizeof(CPUcore_info));
        CPUcore_info_tmp -> id = 1;
        acoral_str_cpy(&(CPUcore_info_tmp -> name[0]), "CPU 1");
        acoral_bytes_send((char *)CPUcore_info_tmp, sizeof(CPUcore_info));
        acoral_vol_free(CPUcore_info_tmp);

    }
    // 发送线程创建相关信息
    if(tinfos_tmp -> size > 0) {
        // 编码数据
        header.type = 2;
        header.length = sizeof(thread_info);
        print_num = tinfos_tmp -> overflow > 0?  tinfos_tmp -> total_size:tinfos_tmp -> size;
        for(int i = 0; i< print_num; i++) {
            acoral_bytes_send((char *)(&(tinfos_tmp -> threads[i])) , sizeof(thread_info));
        }
        // 按字节发送数据
    }
    // 发送线程切换信息
    if(tswinfos_tmp -> size > 0) {
        // 编码数据
        header.type = 3;
        header.length = sizeof(thread_switch_info);
        print_num = tswinfos_tmp -> overflow > 0?  tswinfos_tmp -> total_size:tswinfos_tmp -> size;
        for(int i = 0; i< print_num; i++) {
            acoral_bytes_send((char *)(&(tswinfos_tmp -> switch_infos[i])) , sizeof(thread_switch_info));
        }
        // 按字节发送数据
    }
    acoral_vol_free(tinfos_tmp);
    acoral_vol_free(tswinfos_tmp);
}


#else
/**
 * @brief 发送任务和切换信息
 * 
 */
void acoral_infos_send() {

//    static acoral_u32 symbel = 0;
//    if (symbel == 0) {
//        symbel = 1;
//        return;
//    }
      acoral_print("monitor start");

    acoral_u32 print_num;
    thread_info_set *tinfos_tmp;
    thread_switch_info_set *tswinfos_tmp;
    tinfos_tmp = (thread_info_set *)acoral_vol_malloc(sizeof(thread_info_set));
    tswinfos_tmp = (thread_switch_info_set *)acoral_vol_malloc(sizeof(thread_switch_info_set));


    acoral_enter_critical();
//    acoral_spin_lock(&tinfos_lock);
//    acoral_spin_lock(&tswinfos_lock);
    *tinfos_tmp = *tinfos;
    *tswinfos_tmp = *tswinfos;
    tinfos -> size = 0;
    tinfos -> st_idx = 0;
    tinfos -> ed_idx = 0;
    tswinfos -> size = 0;
    tswinfos -> st_idx = 0;
    tswinfos -> ed_idx = 0;

//    acoral_spin_unlock(&tswinfos_lock);
//    acoral_spin_unlock(&tinfos_lock);
    acoral_exit_critical();
    acoral_print("指针大小: %d \r\n", sizeof(thread_switch_info_set));
    if(tinfos_tmp -> size > 0) {
        acoral_print("打印线程创建信息 \r\n");
        acoral_print("创建线程: %d 条目 \r\n", tinfos_tmp -> size);
        print_num = tinfos_tmp -> overflow > 0?  tinfos_tmp -> total_size:tinfos_tmp -> size;
        for(int i = 0; i< print_num; i++) {
//            acoral_print(
//                    "线程策略: %d, 优先级: %d, cpu: %d, 创建时间:%d 名称: %s \r\n",
//                    tinfos_tmp -> threads[i].thread_policy,
//                    tinfos_tmp -> threads[i].prio,
//                    tinfos_tmp -> threads[i].cpu,
//                    tinfos_tmp -> threads[i].create_tick,
//                    &(tinfos_tmp -> threads[i].thread_name[0])
//            );
            acoral_print(
                    "TCB: %x, 线程策略: %d, 优先级: %d, cpu: %d, 创建时间:%d 名称: %s \r\n",
                    tinfos_tmp -> threads[i].tcb_ptr,
                    tinfos_tmp -> threads[i].thread_policy,
                    tinfos_tmp -> threads[i].prio,
                    tinfos_tmp -> threads[i].cpu,
                    tinfos_tmp -> threads[i].create_tick,
                    &(tinfos_tmp -> threads[i].thread_name[0])
            );
        }

    } else {
        acoral_print("没有线程创建信息 \r\n");
    }
    if(tswinfos_tmp -> size > 0) {
            acoral_print("打印线程切换信息 \r\n");
            acoral_print("切换线程: %d 条目 \r\n", tswinfos_tmp -> size);
            print_num = tswinfos_tmp -> overflow > 0?  tswinfos_tmp -> total_size:tswinfos_tmp -> size;
            for(int i = 0; i< print_num; i++) {
//                acoral_print(
//                        "cpu: %d, 发生时间:%d, from: %s, to: %s \r\n",
//                        tswinfos_tmp -> switch_infos[i].cpu,
//                        tswinfos_tmp -> switch_infos[i].tick,
//                        &(tswinfos_tmp -> switch_infos[i].from_name[0]),
//                        &(tswinfos_tmp -> switch_infos[i].to_name[0])
//                );
                acoral_print(
                            "cpu: %d, 发生时间:%d, from: %x, to: %x \r\n",
                            tswinfos_tmp -> switch_infos[i].cpu,
                            tswinfos_tmp -> switch_infos[i].tick,
                            tswinfos_tmp -> switch_infos[i].from_tcb,
                            tswinfos_tmp -> switch_infos[i].to_tcb
                    );
//                acoral_print(
//                            "cpu: %d, 发生时间:%d, from: %x : %s, to: %x : %s \r\n",
//                            tswinfos_tmp -> switch_infos[i].cpu,
//                            tswinfos_tmp -> switch_infos[i].tick,
//                            tswinfos_tmp -> switch_infos[i].from_tcb,
//                            &(tswinfos_tmp -> switch_infos[i].from_name[0]),
//                            tswinfos_tmp -> switch_infos[i].to_tcb,
//                            &(tswinfos_tmp -> switch_infos[i].to_name[0])
//                    );
            }


    } else {
        acoral_print("没有打印线程切换信息 \r\n");
    }
    acoral_vol_free(tinfos_tmp);
    acoral_vol_free(tswinfos_tmp);
}

#endif

