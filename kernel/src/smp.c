/**
 * @file smp.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层对称多处理器源文件
 * @version 1.0
 * @date 2022-07-11
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-11 <td>增加注释
 */
#include <acoral.h>

///idle线程栈大小
#define IDLE_STACK_SIZE 128
///资源回收线程栈大小
#define DAEM_STACK_SIZE 256
///init线程栈大小
#define INIT_STACK_SIZE 2048

///init线程优先级为最高
#define INIT_PRIO  0

///调度使能标志位
volatile acoral_u32 acoral_sched_enable=false;

///idle线程id
acoral_id idle_id;
///资源回收线程id
acoral_id daemon_id;
///init线程id
acoral_id init_id;

///orig线程，所有线程的父线程
acoral_thread_t orig_thread;

extern acoral_queue_t acoral_release_queue;
#ifdef CFG_SMP
/**
 * @brief 启动次核准备工作
 *
 */
static void acoral_prepare_cpus(void)
{
    HAL_PREPARE_CPUS();
}

/**
 * @brief 启动次核
 *
 */
static void acoral_start_cpus(void)
{
    acoral_32 i;
    for(i=0;i<CFG_MAX_CPU;i++)
        if(!acoral_cpu_is_active(i))
            HAL_START_CPU(i);
}

/**
 * @brief 主核有关多核的初始化
 *
 */
static void acoral_smp_init(void)
{
    //为其他核心启动作准备，比如其他核心的启动代码
    acoral_prepare_cpus();
    //启动其他核心，zynq7020为发送sev
    acoral_start_cpus();
    //核间通信初始化
    acoral_core_ipi_init();
}
#endif

/**
 * @brief idle线程
 * 
 * @param args 回调参数
 */
__weak void idle(void *args)
{
    while(1)
    {

    }
}

/**
 * @brief 资源回收线程
 * 
 * @param args 回调参数
 */
void daem(void *args)
{
    acoral_thread_t * thread;
    acoral_list_t *head,*tmp,*tmp1;
    head=&acoral_release_queue.head;//寻找需要释放的线程队列头
    while(1)
    {
        //依次进行释放
        for(tmp=head->next;tmp!=head;)
        {
            tmp1=tmp->next;
            acoral_enter_critical();
            thread=list_entry(tmp,acoral_thread_t,pending);
            //如果线程资源已经不在使用，即release状态则释放
            acoral_fifo_queue_del(&acoral_release_queue, tmp);
            acoral_exit_critical();
            tmp=tmp1;
            if(thread->state==ACORAL_THREAD_STATE_RELEASE)//线程状态为可释放状态
            {
                acoral_release_thread((acoral_res_t *)thread);
            }
            else//还不是可释放状态，应该还处于退出状态，重新添加到队列尾
            {
                acoral_enter_critical();
                tmp1=head->prev;
                acoral_fifo_queue_add(&acoral_release_queue, &thread->pending);
                acoral_exit_critical();
            }
        }
        acoral_suspend_self();//运行完一轮后挂起自己，等待被唤醒
    }
}


/**
 * @brief 主核初始化线程（弱定义）
 * 
 * @param args 回调参数
 */
__weak void init(void *args)
{
    acoral_ticks_init();//ticks中断初始化函数
    acoral_sched_enable=true;//使能调度
}

#ifdef CFG_SMP
/**
 * @brief 次核idle线程
 *
 * @param args 回调参数
 */
void idle_follow(void *args)
{
    HAL_CMP_ACK();//解锁，响应主核，标识次核启动成功
    while(1)
    {
    }
}
#endif

#ifdef CFG_SMP
/**
 * @brief 主核SMP初始化
 *
 */
static void acoral_core_cpu_init(void)
{
    acoral_cpu_set_active(acoral_current_cpu);
}

/**
 * @brief 次核SMP初始化
 *
 */
static void acoral_follow_cpu_init(void)
{
    acoral_cpu_set_active(acoral_current_cpu);
    acoral_intr_init(acoral_current_cpu);
    acoral_follow_ipi_init();
}
#endif

/**
 * @brief 操作系统启动函数
 *
 */
static void acoral_start_os(void)
{
    acoral_sched_init();//调度初始化
    acoral_select_thread();//就绪优先级最高的线程
    acoral_set_running_thread((void *)acoral_ready_thread);//设置就绪线程运行
    HAL_START_OS();//开始线程运行
}

/**
 * @brief 主核开始函数
 * 
 */
static void acoral_core_cpu_start(void)
{
    acoral_comm_policy_data_t p_data;
    acoral_sched_enable=false;
    //创建idle线程
    p_data.cpu=acoral_current_cpu;
    p_data.prio=ACORAL_IDLE_PRIO;
    idle_id=acoral_create_thread(idle,IDLE_STACK_SIZE,NULL,"idle",NULL,ACORAL_SCHED_POLICY_COMM,&p_data,NULL,NULL);
    if(idle_id==-1)
        while(1);
    //创建初始化线程
    p_data.prio=INIT_PRIO;
    init_id=acoral_create_thread(init,INIT_STACK_SIZE,"in init","init",NULL,ACORAL_SCHED_POLICY_COMM,&p_data,NULL,NULL);
    if(init_id==-1)
        while(1);
    acoral_fifo_queue_init(&acoral_release_queue);
    //创建资源回收线程
    p_data.cpu=acoral_current_cpu;
    p_data.prio=ACORAL_DAEMON_PRIO;
    daemon_id=acoral_create_thread(daem,DAEM_STACK_SIZE,NULL,"daemon",NULL,ACORAL_SCHED_POLICY_COMM,&p_data,NULL,NULL);
    if(daemon_id==-1)
        while(1);
    acoral_start_os();
}
#ifdef CFG_SMP
/**
 * @brief 次核开始函数
 *
 */
static void acoral_follow_cpu_start(void)
{
    acoral_comm_policy_data_t p_data;
    acoral_id idle_follow_id;
    //创建次核idle线程
    p_data.cpu=acoral_current_cpu;
    p_data.prio=ACORAL_IDLE_PRIO;
    idle_follow_id=acoral_create_thread(idle_follow,128,NULL,"idle_follow",NULL,ACORAL_SCHED_POLICY_COMM,&p_data,NULL,NULL);
    if(idle_follow_id==-1)
        while(1);
    acoral_start_os();
}
#endif
/**
 * @brief 内核各个模块初始化
 * 
 */
static void acoral_module_init(void)
{
    //中断系统初始化
    acoral_intr_sys_init();
    //内存管理系统初始化
    acoral_mem_sys_init();
    //资源管理系统初始化
    acoral_res_pool_sys_init();
    //线程管理系统初始化
    acoral_thread_sys_init();
    //时钟管理系统初始化
    acoral_time_sys_init();
    //线程通信管理系统初始化
    acoral_ipc_sys_init();
}


/**
 * @brief C语言入口函数
 * 
 */
void acoral_start(void)
{
#ifdef CFG_SMP
    static acoral_32 core_cpu=1;//主次核判断参数
    if(!core_cpu)//次核运行到这时，core_cpu为0
    {
        acoral_run_orig_thread((void *)&orig_thread);
        acoral_follow_cpu_init();//次核初始化
        acoral_follow_cpu_start();//其他次cpu core的开始函数,不会返回的
    }
    core_cpu=0;
    acoral_core_cpu_init();//主核初始化
#endif
    orig_thread.console_id=1;
    acoral_run_orig_thread((void *)&orig_thread);
    acoral_module_init();//内核模块初始化
#ifdef CFG_SMP
    acoral_smp_init();//cmp初始化，包含启动次核
#endif
#ifdef CFG_MPCP
    acoral_mpcp_system_init();//mpcp初始化
#endif
#ifdef CFG_DPCP
    acoral_dpcp_system_init();
#endif
    acoral_core_cpu_start();//主核开始
}
