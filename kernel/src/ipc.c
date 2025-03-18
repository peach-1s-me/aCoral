/**
 * @file ipc.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层线程通信相关源文件
 * @version 1.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-13 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 *         <tr><td>v1.2 <td>文佳源 <td>2025-02-26 <td>修改acoral_ipc_wait_queue_empty返回值类型bool->acoral_bool
 * 
 */
#include <acoral.h>

///ipc内存池控制结构体实例
acoral_pool_ctrl_t acoral_ipc_pool_ctrl;

/**
 * @brief 线程通信管理系统初始化
 * 
 */
void acoral_ipc_sys_init(void)
{
    acoral_ipc_pool_init();
}
/**
 * @brief 线程通信内存池初始化
 * 
 */
void acoral_ipc_pool_init(void)
{
    acoral_ipc_pool_ctrl.type=ACORAL_RES_IPC;
    acoral_ipc_pool_ctrl.size=sizeof(acoral_ipc_t);
    acoral_ipc_pool_ctrl.num_per_pool=8;
    acoral_ipc_pool_ctrl.num=0;
    acoral_ipc_pool_ctrl.max_pools=4;
    acoral_pool_ctrl_init(&acoral_ipc_pool_ctrl);
}
/**
 * @brief 分配线程通信结构体
 * 
 * @return acoral_ipc_t* 线程通信结构体指针
 */
acoral_ipc_t *acoral_ipc_alloc(void)
{
    return (acoral_ipc_t *)acoral_get_res(&acoral_ipc_pool_ctrl);
}
/**
 * @brief 线程通信结构体初始化
 * 
 * @param ipc 线程通信结构体指针
 */
void acoral_ipc_init(acoral_ipc_t *ipc)
{
    acoral_spin_init(&ipc->lock);
#if CFG_IPC_QUEUE_MODE == CFG_FIFO_QUEUE
    acoral_fifo_queue_init(&ipc->wait_queue);
#else
    acoral_prio_queue_init(&ipc->wait_queue);
#endif
}

/**
 * @brief 检测ipc等待队列是否为空
 * 
 * @param ipc 线程通信结构体指针
 * @return true 无节点
 * @return false 有节点
 */
acoral_bool acoral_ipc_wait_queue_empty(acoral_ipc_t *ipc)
{
    return acoral_list_empty(&ipc->wait_queue.head);
}
/**
 * @brief 获取ipc等待队列最优先线程
 * 
 * @param ipc 线程通信结构体指针
 * @return void* 线程结构体指针
 */
void *acoral_ipc_high_thread(acoral_ipc_t *ipc)
{
    acoral_list_t *head;
    acoral_thread_t *thread;
    head=&ipc->wait_queue.head;
    if(acoral_list_empty(head))
        return NULL;
    thread=list_entry(head->next,acoral_thread_t,pending);
    return thread;
}
/**
 * @brief ipc等待队列添加节点
 * 
 * @param ipc 线程通信结构体指针
 * @param new 要添加的线程结构体指针
 */
void acoral_ipc_wait_queue_add(acoral_ipc_t *ipc, void *new)
{
	acoral_thread_t *thread = (acoral_thread_t *)new;
	thread->ipc=ipc;
#if CFG_IPC_QUEUE_MODE == CFG_FIFO_QUEUE
    acoral_fifo_queue_add(&ipc->wait_queue, &thread->pending);
#else
    acoral_prio_queue_add(&ipc->wait_queue, &thread->pending);
#endif
}
/**
 * @brief ipc等待队列删除节点
 * 
 * @param ipc 线程通信结构体指针
 * @param old 要删除的线程结构体指针
 */
void acoral_ipc_wait_queue_del(acoral_ipc_t *ipc, void *old)
{
	acoral_thread_t *thread = (acoral_thread_t *)old;
#if CFG_IPC_QUEUE_MODE == CFG_FIFO_QUEUE
    acoral_fifo_queue_del(&ipc->wait_queue, &thread->pending);
#else
    acoral_prio_queue_del(&ipc->wait_queue, &thread->pending);
#endif
    thread->ipc=NULL;
}


/**********************************互斥量******************************************/
///信号量资源：可用
#define MUTEX_AVAI 0x00FF
///信号量资源：不可用
#define MUTEX_NOAVAI 0xFF00

/**
 * @brief 互斥量初始化
 * 
 * @param ipc 互斥量指针
 * @return acoral_err
 */
acoral_err acoral_mutex_init(acoral_ipc_t *ipc)
{
    if((acoral_ipc_t*)0 == ipc)
    {
        return KR_IPC_ERR_NULL;
    }
    ipc->count  = MUTEX_AVAI;
    ipc->type = ACORAL_IPC_MUTEX;
    ipc->data = NULL;
    acoral_ipc_init(ipc);
    return KR_OK;
}

/**
 * @brief 创建互斥量
 * 
 * @return acoral_ipc_t* 传递指针
 */
acoral_ipc_t *acoral_mutex_create(void)
{
    acoral_ipc_t *ipc;

    ipc = acoral_ipc_alloc();//获取内存块
    if(NULL == ipc)
    {
        return NULL;
    }
    ipc->count  = MUTEX_AVAI;
    ipc->type = ACORAL_IPC_MUTEX;
    ipc->data = NULL;
    acoral_ipc_init(ipc);
    return ipc;
}

/**
 * @brief 删除互斥量
 * 
 * @param ipc 互斥量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mutex_del(acoral_ipc_t *ipc)
{


    if(acoral_intr_nesting)//只能在中断最外层运行
    {
        return KR_IPC_ERR_INTR;
    }
    //参数检测
    if(NULL == ipc)
        return KR_IPC_ERR_NULL;//error
    if(ipc->type != ACORAL_IPC_MUTEX)
        return KR_IPC_ERR_TYPE;//error

    
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if(acoral_ipc_wait_queue_empty(ipc))//是否有线程等待
    {
        //无等待线程删除
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        ipc = NULL;
        return KR_OK;
    }
    else
    {
        //有等待线程
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_IPC_ERR_TASK_EXIST;
    }
}

/**
 * @brief 申请互斥量（无阻塞）
 * 
 * @param ipc 信号量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mutex_trypend(acoral_ipc_t *ipc)
{

    acoral_thread_t *cur;

    if(acoral_intr_nesting>0)
        return KR_IPC_ERR_INTR;

    cur=acoral_cur_thread;

    //参数检测
    if(NULL == ipc)
        return KR_IPC_ERR_NULL;//error
    if(ipc->type != ACORAL_IPC_MUTEX)
        return KR_IPC_ERR_TYPE;//error
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if (ipc->count == MUTEX_AVAI)
    {
        //申请成功
        ipc->count = MUTEX_NOAVAI;
        ipc->data = (void*)cur;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }

#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_IPC_ERR_TIMEOUT;
}


/**
 * @brief 申请互斥量（有阻塞）
 * 
 * @param ipc 互斥量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mutex_pend(acoral_ipc_t *ipc)
{

    acoral_thread_t *cur;

    if(acoral_intr_nesting>0)
        return KR_IPC_ERR_INTR;

    cur=acoral_cur_thread;

    //参数检测
    if(NULL == ipc)
        return KR_IPC_ERR_NULL;//error
    if(ipc->type != ACORAL_IPC_MUTEX)
        return KR_IPC_ERR_TYPE;//error
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if (ipc->count == MUTEX_AVAI)//是否可用
    {
        //申请成功
        ipc->count = MUTEX_NOAVAI;
        ipc->data = (void*)cur;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }
    acoral_suspend_self();
    acoral_ipc_wait_queue_add(ipc, (void *)cur);//添加到等待队列
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    //给个调度间隙
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if(ipc->data == NULL)
    {
        acoral_ipc_wait_queue_del(ipc, (void *)cur);//从等待队列中删除
        ipc->count = MUTEX_NOAVAI;
        ipc->data = (void*)cur;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }
    else
    {
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_IPC_ERR_UNDEF;
    }
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_OK;
}

/**
 * @brief 释放信号量
 * 
 * @param ipc 互斥量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mutex_post(acoral_ipc_t *ipc)
{

    acoral_thread_t *thread;
    acoral_thread_t *cur;

    cur=acoral_cur_thread;

    //参数检测
    if(NULL == ipc)
        return KR_IPC_ERR_NULL;//error
    if(ipc->type != ACORAL_IPC_MUTEX)
        return KR_IPC_ERR_TYPE;//error
    if(ipc->data != cur)
        return KR_IPC_ERR_THREAD;//error
    cur->ipc=NULL;
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    ipc->count = MUTEX_AVAI;
    ipc->data = NULL;
    thread = (acoral_thread_t *)acoral_ipc_high_thread(ipc);
    if (thread==NULL)//无等待线程
    {
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    acoral_rdy_thread(thread);
    return KR_OK;
}
/**********************************互斥量******************************************/

/**********************************信号量******************************************/
///信号量资源：可用
#define SEM_AVAI 0
///信号量资源：不可用
#define SEM_NOAVAI 1
/**
 * @brief 信号量初始化
 * 
 * @param ipc 信号量结构体
 * @param semNum 资源数
 * @return acoral_err 错误检测
 */
acoral_err acoral_sem_init(acoral_ipc_t *ipc,acoral_u32 semNum)
{
	if (NULL == ipc)
	{
		return 0;
	}
    ipc->count = 1 - semNum;//拥有多个资源，0 一个，-1 两个，-2 三个...
    ipc->number = semNum;
	ipc->type  = ACORAL_IPC_SEM;
	ipc->data  = NULL;
	acoral_ipc_init(ipc);
	return KR_OK;
}

/**
 * @brief 创建信号量
 * 
 * @param semNum 资源数
 * @return acoral_ipc_t* 传递指针
 */
acoral_ipc_t *acoral_sem_create(acoral_u32 semNum)
{
    acoral_ipc_t *ipc;
    ipc = acoral_ipc_alloc();//获取内存块
    //参数检测
    if (NULL == ipc)
    {
        return NULL;
    }
    ipc->count = 1 - semNum;//拥有多个资源，0 一个，-1 两个，-2 三个...
    ipc->number = semNum;
    ipc->type  = ACORAL_IPC_SEM;//类型：信号量
    ipc->data  = NULL;
    acoral_ipc_init(ipc);//初始化资源
    return ipc;
}

/**
 * @brief 删除信号量
 * 
 * @param ipc 信号量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_sem_del(acoral_ipc_t *ipc)
{

    if(acoral_intr_nesting)//只能在中断最外层运行
    {
        return KR_IPC_ERR_INTR;
    }
    //参数检测
    if(NULL == ipc)
        return KR_IPC_ERR_NULL;//error
    if(ipc->type != ACORAL_IPC_SEM)
        return KR_IPC_ERR_TYPE;//error

    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if(acoral_ipc_wait_queue_empty(ipc))
    {
        //队列上无等待线程
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        ipc = NULL;
        return KR_IPC_ERR_UNDEF;
    }
    else
    {
        //有等待线程
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_IPC_ERR_TASK_EXIST;//error
    }
}

/**
 * @brief 申请信号量（无阻塞）
 * 
 * @param ipc 信号量指针
 * @return acoral_err 错误检测
 */
acoral_u32 acoral_sem_trypend(acoral_ipc_t *ipc)
{


    if (acoral_intr_nesting)
    {
        return KR_IPC_ERR_INTR;
    }

    //参数检测
    if (NULL == ipc)
    {
        return KR_IPC_ERR_NULL;//error
    }
    if (ACORAL_IPC_SEM != ipc->type)
    {
        return KR_IPC_ERR_TYPE;//error
    }

    //计算信号量处理
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if ((acoral_8)ipc->count <= SEM_AVAI)//有资源可使用
    {
        ipc->count++;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_IPC_ERR_TIMEOUT;
}

/**
 * @brief 申请信号量（有阻塞，不获取资源）
 *
 * @param ipc 信号量指针
 * @return acoral_err
 */
acoral_err acoral_sem_pends(acoral_ipc_t *ipc)
{
    acoral_thread_t *cur = acoral_cur_thread;


    if(acoral_intr_nesting)
    {
        return KR_IPC_ERR_INTR;
    }

    //参数检测
    if(NULL == ipc)
    {
        return KR_IPC_ERR_NULL;//error
    }
    if(ACORAL_IPC_SEM != ipc->type)
    {
        return KR_IPC_ERR_TYPE;//error
    }

    //计算信号量处理
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if ((acoral_8)ipc->count <= SEM_AVAI)//有资源可使用
    {
//        ipc->count++;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }

//    ipc->count++;
    acoral_suspend_self();
    acoral_ipc_wait_queue_add(ipc, (void *)cur);//添加线程到信号量等待队列
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    //给个调度间隙
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);//自旋锁检测
#endif
    acoral_ipc_wait_queue_del(ipc, (void *)cur);//删除信号量等待队列中的此线程
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_OK;
}

/**
 * @brief 申请信号量（有阻塞，不获取资源）
 * 
 * @param ipc 信号量指针
 * @return acoral_err
 */
acoral_err acoral_sem_pend(acoral_ipc_t *ipc)
{
    acoral_thread_t *cur = acoral_cur_thread;


    if(acoral_intr_nesting)
    {
        return KR_IPC_ERR_INTR;
    }

    //参数检测
    if(NULL == ipc)
    {
        return KR_IPC_ERR_NULL;//error
    }
    if(ACORAL_IPC_SEM != ipc->type)
    {
        return KR_IPC_ERR_TYPE;//error
    }

    //计算信号量处理
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    if ((acoral_8)ipc->count <= SEM_AVAI)//有资源可使用
    {
        ipc->count++;
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_OK;
    }

    ipc->count++;
    acoral_suspend_self();
    acoral_ipc_wait_queue_add(ipc, (void *)cur);//添加线程到信号量等待队列
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    //给个调度间隙
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);//自旋锁检测
#endif
    acoral_ipc_wait_queue_del(ipc, (void *)cur);//删除信号量等待队列中的此线程
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_OK;
}

/**
 * @brief 信号量释放（调度，配合pends使用）
 *
 * @param ipc 信号量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_sem_posts(acoral_ipc_t *ipc)
{
    acoral_list_t *tmp, *head;
    acoral_thread_t *thread;
    //参数检测
    if ( NULL == ipc)
    {
        return KR_IPC_ERR_NULL;//error
    }
    if (ACORAL_IPC_SEM != ipc->type)
    {
        return KR_IPC_ERR_TYPE;
    }

    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    //计算信号量的释放
    if(ipc->count >= -32768)
    {
        if(ipc->count <= SEM_NOAVAI)//释放资源
        {
            ipc->count--;
        }
        head=&ipc->wait_queue.head;
        if(acoral_list_empty(head))
        {
#ifdef CFG_SMP
            acoral_spin_unlock(&ipc->lock);
#endif
            acoral_exit_critical();
            return KR_OK;
        }
        //有等待线程，依次就绪
        for(tmp=head->next;tmp!=head;tmp=tmp->next)
        {
            thread = list_entry(tmp,acoral_thread_t,pending);
            if(thread==NULL)//应该有等待线程却没有找到
            {
                acoral_printerr("Err Sem post\n");
#ifdef CFG_SMP
                acoral_spin_unlock(&ipc->lock);
#endif
                acoral_exit_critical();
                return KR_IPC_ERR_UNDEF;
            }
            acoral_rdy_thread(thread);
        }
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
    }
    return KR_OK;
}

/**
 * @brief 信号量释放（调度）
 * 
 * @param ipc 信号量指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_sem_post(acoral_ipc_t *ipc)
{
    acoral_thread_t     *thread;


    //参数检测
    if ( NULL == ipc)
    {
        return KR_IPC_ERR_NULL;//error
    }
    if (ACORAL_IPC_SEM != ipc->type)
    {
        return KR_IPC_ERR_TYPE;
    }

    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    //计算信号量的释放
    if(ipc->count >= -32768)
    {
        if (ipc->count <= SEM_NOAVAI)//没有线程等待获取该信号量
        {
            ipc->count--;
#ifdef CFG_SMP
            acoral_spin_unlock(&ipc->lock);
#endif
            acoral_exit_critical();
            return KR_OK;
        }
    }
    else
    {
#ifdef CFG_SMP
            acoral_spin_unlock(&ipc->lock);
#endif
        return KR_IPC_ERR_UNDEF;
    }
    //有等待线程
    ipc->count--;
    thread = (acoral_thread_t *)acoral_ipc_high_thread(ipc);
    if (thread==NULL)//应该有等待线程却没有找到
    {
        acoral_printerr("Err Sem post\n");
#ifdef CFG_SMP
        acoral_spin_unlock(&ipc->lock);
#endif
        acoral_exit_critical();
        return KR_IPC_ERR_UNDEF;
    }
    //就绪等待线程
    acoral_rdy_thread(thread);
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return KR_OK;
}

/**
 * @brief 获取信号量当前资源数
 * 
 * @param ipc 信号量指针
 * @return acoral_32 当前资源数
 */
acoral_32 acoral_sem_getnum(acoral_ipc_t* ipc)
{
    acoral_32 t;

    //参数检测
    if (NULL == ipc)
        return KR_IPC_ERR_NULL;

    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&ipc->lock);
#endif
    t = 1 - (acoral_32)ipc->count;//获取当前资源数
#ifdef CFG_SMP
    acoral_spin_unlock(&ipc->lock);
#endif
    acoral_exit_critical();
    return t;
}
/**********************************信号量******************************************/

/**********************************mpcp******************************************/
///mpcp局部资源系统结构体实例
acoral_mpcp_system_t acoral_mpcp_system[CFG_MAX_CPU];
/**
 * @brief mpcp初始化
 *
 */
void acoral_mpcp_system_init(void)
{
    acoral_32 i = 0;
    for(i=0;i<CFG_MAX_CPU;i++)
    {
        acoral_mpcp_system[i].prio = ACORAL_MAX_PRIO_NUM;
        acoral_lifo_queue_v_init(&acoral_mpcp_system[i].stack, ACORAL_MAX_PRIO_NUM);
        acoral_prio_queue_init(&acoral_mpcp_system[i].wait);
    }
}
/**
 * @brief 创建mpcp
 *
 * @param type mpcp类型
 * @param prio_ceiling 优先级天花板
 * @return acoral_mpcp_t* mpcp指针
 */
acoral_mpcp_t *acoral_mpcp_create(acoral_u8 type, acoral_u8 prio_ceiling)
{
    acoral_mpcp_t *mpcp;
    mpcp = acoral_malloc(sizeof(acoral_mpcp_t));
    mpcp->type = type;
    if(mpcp->type == ACORAL_MPCP_GLOBAL)
    {
        mpcp->cpu = 0xff;
    }
    else if(mpcp->type == ACORAL_MPCP_LOCAL)
    {
        mpcp->cpu = acoral_current_cpu;
    }
    else
    {
        return NULL;
    }

    mpcp->prio_ceiling = prio_ceiling;
    mpcp->prio_switch = 0;
    acoral_vlist_init(&mpcp->stacking, mpcp->prio_ceiling);//初始化mpcp栈链表
    acoral_spin_init(&mpcp->lock);
    mpcp->sem = acoral_sem_create(1);//初始化二值信号量
    return mpcp;
}
/**
 * @brief 删除mpcp
 *
 * @param mpcp mpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mpcp_del(acoral_mpcp_t *mpcp)
{
    acoral_u32 r_temp;
    r_temp = acoral_sem_del(mpcp->sem);//删除信号量
    acoral_free(mpcp);//回收mpcp内存
    mpcp = NULL;
    return r_temp;
}
/**
 * @brief 获取mpcp
 *
 * @param mpcp mpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mpcp_pend(acoral_mpcp_t *mpcp)
{
    acoral_u32 r_temp;
    acoral_thread_t *cur = acoral_cur_thread;
    acoral_u32 cur_cpu = acoral_current_cpu;
    acoral_mpcp_t *last_mpcp;
    if(mpcp->type == ACORAL_MPCP_LOCAL)//局部资源
    {
        if(mpcp->cpu != cur_cpu)
            return KR_IPC_ERR_CPU;
        if(cur->prio >= acoral_mpcp_system[cur_cpu].prio)//当前线程优先级低于或等于当前核系统优先级
        {
            //找出天花板为系统优先级的局部资源last_mpcp
            last_mpcp = list_entry(acoral_mpcp_system[cur_cpu].stack.head.next, acoral_mpcp_t, stacking);
            //正在使用last_mpcp的线程优先级继承当前线程优先级
            acoral_thread_change_prio((acoral_thread_t *)last_mpcp->sem->data, cur->prio);
            //把当前线程添加到系统等待队列
            acoral_prio_queue_add(&acoral_mpcp_system[cur_cpu].wait, &cur->pending);
            acoral_suspend_self();//阻塞当前线程，并引发调度
        }
        if(cur->prio < acoral_mpcp_system[cur_cpu].prio)//当前线程优先级高于当前核系统优先级
        {
            //把系统优先级改成当前mpcp的天花板
            acoral_mpcp_system[cur_cpu].prio = mpcp->prio_ceiling;
            //当前mpcp入mpcp局部资源系统栈，即将被使用
            acoral_lifo_queue_add(&acoral_mpcp_system[cur_cpu].stack, &mpcp->stacking);
            mpcp->prio_switch = cur->prio;//暂存线程优先级
        }
    }
    else if(mpcp->type == ACORAL_MPCP_GLOBAL)//全局资源
    {
#ifdef CFG_SMP
        acoral_spin_lock(&mpcp->lock);
#endif
        if(cur->preempt_type == ACORAL_PREEMPT_LOCAL)
            cur->preempt_type = ACORAL_PREEMPT_GLOBAL;//设置线程抢占类型为全局
        if(mpcp->sem->data == NULL)//该mpcp没有被获取
        {
            mpcp->prio_switch = cur->prio;//暂存线程优先级
            acoral_thread_change_prio(cur, mpcp->prio_ceiling);//改变线程优先级为天花板
        }
    }

    if(mpcp->sem->data == NULL)//如果资源未被锁住
    {
        mpcp->sem->data = (void *)cur;
    }

    r_temp = acoral_sem_pend(mpcp->sem);//获取信号量
#ifdef CFG_SMP
    if(mpcp->type == ACORAL_MPCP_GLOBAL)//只有全局资源需要用到自旋锁
        acoral_spin_unlock(&mpcp->lock);
#endif
    return r_temp;
}
/**
 * @brief 释放mpcp
 *
 * @param mpcp mpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_mpcp_post(acoral_mpcp_t *mpcp)
{
    acoral_u32 r_temp;
    acoral_thread_t *cur = acoral_cur_thread;
    acoral_thread_t *mpcp_thread = (acoral_thread_t *)mpcp->sem->data;
    acoral_u32 cur_cpu = acoral_current_cpu;
    acoral_thread_t *thread;
    if(mpcp_thread != cur)
        return KR_IPC_ERR_THREAD;
    if(mpcp->type == ACORAL_MPCP_LOCAL)//局部资源
    {
        acoral_lifo_queue_del(&acoral_mpcp_system[cur_cpu].stack, &mpcp->stacking);
        //系统优先级也要出栈恢复
        acoral_mpcp_system[cur_cpu].prio = acoral_mpcp_system[cur_cpu].stack.head.next->value;
        if(!acoral_list_empty(&acoral_mpcp_system[cur_cpu].wait.head))//mpcp局部资源等待队列中有线程
        {
            thread = list_entry(acoral_mpcp_system[cur_cpu].wait.head.next, acoral_thread_t, pending);
            acoral_rdy_thread(thread);//就绪线程
        }
    }
    else if(mpcp->type == ACORAL_MPCP_GLOBAL)//全局资源
    {
#ifdef CFG_SMP
        acoral_spin_lock(&mpcp->lock);
#endif
        if(cur->preempt_type == ACORAL_PREEMPT_GLOBAL)
            cur->preempt_type = ACORAL_PREEMPT_LOCAL;//设置线程抢占类型为局部
    }

    if(mpcp_thread != NULL)//如果资源已被锁住
    {
        if(mpcp_thread->prio != mpcp->prio_switch)
            acoral_thread_change_prio(mpcp_thread, mpcp->prio_switch);//恢复线程优先级
        mpcp->prio_switch = 0;//清除暂存优先级
        mpcp->sem->data = NULL;
    }

    r_temp = acoral_sem_post(mpcp->sem);//释放信号量
#ifdef CFG_SMP
    if(mpcp->type == ACORAL_MPCP_GLOBAL)//只有全局资源需要用到自旋锁
        acoral_spin_unlock(&mpcp->lock);
#endif
    return r_temp;
}
/**********************************mpcp******************************************/
/**********************************dpcp******************************************/
///暂存优先级标志移位
#define PRIO_SWITCH_FLAG_SHIFT 31
///暂存优先级标志掩码
#define PRIO_SWITCH_FLAG_MASK 0x8000
///暂存优先级掩码
#define PRIO_SWITCH_MASK 0x7fff
///dpcp资源系统结构体实例
acoral_dpcp_system_t acoral_dpcp_system[2][CFG_MAX_CPU];
///dpcp总队列
acoral_queue_t acoral_dpcp_queue[2][CFG_MAX_CPU];

/**
 * @brief dpcp系统优先级初始化
 *
 */
void acoral_dpcp_system_init(void)
{
    acoral_32 i = 0;
    for(i=0;i<CFG_MAX_CPU;i++)
    {
        acoral_dpcp_system[0][i].prio = ACORAL_MAX_PRIO_NUM;
        acoral_dpcp_system[1][i].prio = ACORAL_MAX_PRIO_NUM;
        acoral_lifo_queue_v_init(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][i].stack, ACORAL_MAX_PRIO_NUM);
        acoral_prio_queue_init(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][i].wait);
        acoral_lifo_queue_v_init(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][i].stack, ACORAL_MAX_PRIO_NUM);
        acoral_prio_queue_init(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][i].wait);
        acoral_lifo_queue_init(&acoral_dpcp_queue[ACORAL_DPCP_LOCAL][i]);
        acoral_lifo_queue_init(&acoral_dpcp_queue[ACORAL_DPCP_GLOBAL][i]);
    }
}

/**
 * @brief 更新dpcp资源状态接口
 *
 * @param dpcp dpcp指针
 * @param type 资源类型
 * @param prio_ceiling 优先级天花板
 */
void acoral_dpcp_set(acoral_dpcp_t *dpcp, acoral_u8 type, acoral_u8 prio_ceiling)
{
    if(acoral_dpcp_system[dpcp->type][dpcp->cpu].stack.head.next == &dpcp->stacking)
    {
        if(dpcp->type != type)
        {
            //出dpcp资源系统栈
            acoral_lifo_queue_del(&acoral_dpcp_system[dpcp->type][dpcp->cpu].stack, &dpcp->stacking);
            //系统优先级也要出栈恢复
            acoral_dpcp_system[dpcp->type][dpcp->cpu].prio = acoral_dpcp_system[dpcp->type][dpcp->cpu].stack.head.next->value;
            //把系统优先级改成当前dpcp的天花板
            acoral_dpcp_system[type][dpcp->cpu].prio = prio_ceiling;
            //出dpcp资源系统栈
            acoral_lifo_queue_add(&acoral_dpcp_system[type][dpcp->cpu].stack, &dpcp->stacking);

            acoral_lifo_queue_del(&acoral_dpcp_queue[dpcp->type][dpcp->cpu], &dpcp->list);
            acoral_lifo_queue_add(&acoral_dpcp_queue[type][dpcp->cpu], &dpcp->list);
        }
        else
        {
            acoral_dpcp_system[dpcp->type][dpcp->cpu].prio = prio_ceiling;
        }
    }
    dpcp->type = type;
    dpcp->prio_ceiling = prio_ceiling;
    dpcp->stacking.value = prio_ceiling;
}

/**
 * @brief 创建dpcp
 *
 * @param type 资源类型
 * @param prio_ceiling 优先级天花板
 * @return acoral_dpcp_t* dpcp指针
 */
acoral_dpcp_t *acoral_dpcp_create(void)
{
    acoral_dpcp_t *dpcp;
    dpcp = acoral_malloc(sizeof(acoral_dpcp_t));
    dpcp->type = ACORAL_DPCP_LOCAL;
    dpcp->cpu = UNBIND_CPU;
    dpcp->prio_ceiling = ACORAL_MAX_PRIO_NUM;
    dpcp->prio_switch = 0;
    acoral_vlist_init(&dpcp->stacking, dpcp->prio_ceiling);//初始化dpcp栈链表
    acoral_list_init(&dpcp->list);//初始化dpcp链表
    dpcp->sem = acoral_sem_create(1);//初始化二值信号量
    return dpcp;
}
/**
 * @brief 删除dpcp
 *
 * @param dpcp dpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_dpcp_del(acoral_dpcp_t *dpcp)
{
    acoral_u32 r_temp;
    r_temp = acoral_sem_del(dpcp->sem);//删除信号量
    if(!acoral_list_empty(&dpcp->stacking))
        acoral_lifo_queue_del(&acoral_dpcp_system[dpcp->type][dpcp->cpu].stack, &dpcp->stacking);
    if(dpcp->data!=NULL)
    {
        acoral_vol_free(dpcp->data);
        dpcp->data = NULL;
    }
    acoral_free(dpcp);//回收mpcp内存
    dpcp = NULL;
    return r_temp;
}
/**
 * @brief 获取dpcp
 *
 * @param dpcp dpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_dpcp_pend(acoral_dpcp_t *dpcp)
{
    acoral_u32 r_temp;
    acoral_thread_t *cur = acoral_cur_thread;
    acoral_u32 cur_cpu = acoral_current_cpu;
#ifdef CFG_SMP
    acoral_u32 *origin_cpu_p = NULL;
#endif
    acoral_dpcp_t *last_dpcp;
    acoral_enter_critical();
    if(dpcp->type == ACORAL_DPCP_LOCAL)
    {
        if(dpcp->cpu != cur_cpu)
            return KR_IPC_ERR_CPU;
        while(1)
        {
            if(cur->prio >= acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].prio)//当前线程优先级低于或等于当前核系统优先级
            {
                //找出天花板为系统优先级的局部资源last_dpcp
                last_dpcp = list_entry(acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].stack.head.next, acoral_dpcp_t, stacking);
                //正在使用last_dpcp的线程优先级继承当前线程优先级
                acoral_thread_change_prio((acoral_thread_t *)last_dpcp->sem->data, cur->prio);
                //把当前线程添加到系统等待队列
                acoral_prio_queue_add(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].wait, &cur->pending);
                acoral_exit_critical();
                acoral_suspend_self();//阻塞当前线程，并引发调度
                acoral_enter_critical();
            }
            else if(cur->prio < acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].prio)//当前线程优先级高于当前核系统优先级
            {
                //把系统优先级改成当前dpcp的天花板
                acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].prio = dpcp->prio_ceiling;
                //当前dpcp入dpcp局部资源系统栈，即将被使用
                acoral_lifo_queue_add(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].stack, &dpcp->stacking);
                dpcp->prio_switch = (1<<PRIO_SWITCH_FLAG_SHIFT)|cur->prio;//暂存线程优先级
                break;
            }
        }
    }
    else if(dpcp->type == ACORAL_DPCP_GLOBAL)
    {
#ifdef CFG_SMP
        if(cur->cpu != dpcp->cpu)
        {
            origin_cpu_p = acoral_vol_malloc(4);
            *origin_cpu_p = cur->cpu;
            cur->data = (void *)origin_cpu_p;
            acoral_exit_critical();
            acoral_jump_cpu(dpcp->cpu);
            acoral_enter_critical();
        }
#endif
        if(cur->preempt_type == ACORAL_PREEMPT_LOCAL)
            cur->preempt_type = ACORAL_PREEMPT_GLOBAL;//设置线程抢占类型为全局
        while(1)
        {
            if(cur->prio >= acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].prio)//当前线程优先级低于或等于资源所在核系统优先级
            {
                //把当前线程添加到系统等待队列
                acoral_prio_queue_add(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].wait, &cur->pending);
                acoral_exit_critical();
                acoral_suspend_self();//阻塞当前线程，并引发调度
                acoral_enter_critical();
            }
            if(cur->prio < acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].prio)//当前线程优先级高于资源所在核系统优先级
            {
                //把系统优先级改成当前dpcp的天花板
                acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].prio = dpcp->prio_ceiling;
                //当前dpcp入dpcp局部资源系统栈，即将被使用
                acoral_lifo_queue_add(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].stack, &dpcp->stacking);
                break;
            }
        }

    }
    if(dpcp->sem->data == NULL)//如果资源未被锁住
    {
        dpcp->sem->data = (void *)cur;
    }
    r_temp = acoral_sem_pend(dpcp->sem);//获取信号量
    acoral_exit_critical();
    return r_temp;
}

/**
 * @brief 释放dpcp
 *
 * @param dpcp dpcp指针
 * @return acoral_err 错误检测
 */
acoral_err acoral_dpcp_post(acoral_dpcp_t *dpcp)
{
    acoral_u32 r_temp;
    acoral_thread_t *cur = acoral_cur_thread;
    acoral_thread_t *dpcp_thread = (acoral_thread_t *)dpcp->sem->data;
    acoral_u32 cur_cpu = acoral_current_cpu;
#ifdef CFG_SMP
    acoral_u32 *origin_cpu_p = (acoral_u32 *)cur->data;
    acoral_u32 origin_cpu = 0;
#endif
    acoral_thread_t *thread;
    acoral_enter_critical();
    if(dpcp_thread != cur)
        return KR_IPC_ERR_THREAD;
    if(dpcp->type == ACORAL_DPCP_LOCAL)//局部资源
    {
        if(dpcp->cpu != cur_cpu)
            return KR_IPC_ERR_CPU;
        //出dpcp局部资源系统栈
        acoral_lifo_queue_del(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].stack, &dpcp->stacking);
        //系统优先级也要出栈恢复
        acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].prio = acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].stack.head.next->value;
        if(!acoral_list_empty(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].wait.head))//dpcp局部资源等待队列中有线程
        {
            thread = list_entry(acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].wait.head.next, acoral_thread_t, pending);
            acoral_prio_queue_del(&acoral_dpcp_system[ACORAL_DPCP_LOCAL][cur_cpu].wait, &thread->pending);
            acoral_rdy_thread(thread);//就绪线程
        }
    }
    else if(dpcp->type == ACORAL_DPCP_GLOBAL)
    {
        if(cur->preempt_type == ACORAL_PREEMPT_GLOBAL)
            cur->preempt_type = ACORAL_PREEMPT_LOCAL;//设置线程抢占类型为局部
        //出dpcp全局资源系统栈
        acoral_lifo_queue_del(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].stack, &dpcp->stacking);
        //系统优先级也要出栈恢复
        acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].prio = acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].stack.head.next->value;
        if(!acoral_list_empty(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].wait.head))//dpcp局部资源等待队列中有线程
        {
            thread = list_entry(acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].wait.head.next, acoral_thread_t, pending);
            acoral_prio_queue_del(&acoral_dpcp_system[ACORAL_DPCP_GLOBAL][dpcp->cpu].wait, &thread->pending);
            acoral_rdy_thread(thread);//就绪线程
        }
    }

    if(dpcp_thread != NULL)//如果资源已被锁住
    {
        if(dpcp->prio_switch&PRIO_SWITCH_FLAG_MASK)
        {
            if(dpcp_thread->prio != (dpcp->prio_switch&PRIO_SWITCH_MASK))
                acoral_thread_change_prio(dpcp_thread, dpcp->prio_switch);//恢复线程优先级
            dpcp->prio_switch = 0;//清除暂存优先级
        }
            dpcp->sem->data = NULL;
    }

    r_temp = acoral_sem_post(dpcp->sem);//释放信号量
#ifdef CFG_SMP
    if(dpcp->type == ACORAL_DPCP_GLOBAL)//只有全局资源需要迁移
    {
        if((origin_cpu_p != NULL)&&(*origin_cpu_p != cur_cpu))
        {
            origin_cpu = *origin_cpu_p;
            acoral_vol_free(cur->data);
            cur->data = NULL;
            acoral_exit_critical();
            acoral_jump_cpu(origin_cpu);
            acoral_enter_critical();
        }
    }
#endif
    acoral_exit_critical();
    return r_temp;
}
