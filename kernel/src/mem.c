/**
 * @file mem.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层内存相关源文件
 * @version 2.0
 * @date 2025-04-02
 *
 * @copyright Copyright (c) 2022 EIC-UESTC
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-09-26 <td>错误头文件相关改动
 *         <tr><td>v2.0 <td>文佳源 <td>2025-04-02 <td>规范代码风格
 */

#include "config.h"
#include "type.h"
#include "hal.h"
#include "mem.h"
#include "ipc.h"
#include "lsched.h"
#include "spinlock.h"
#include "hal.h"
#include "bitops.h"
#include "print.h"
#include "error.h"

/**
 * @brief 内存管理系统初始化
 *
 */
void acoral_mem_sys_init(void)
{
    /* 硬件相关的内存初始化，比如内存控制器等 */
    HAL_MEM_INIT();

    /* 堆初始化,这个可以选择不同管理系统，比如buddy内存管理等 */
    acoral_mem_init((acoral_u32)HAL_HEAP_START, (acoral_u32)HAL_HEAP_END);

#ifdef CFG_MEM_VOL
    acoral_vol_mem_init();
#endif
}

/**************************************伙伴系统**********************************************/
#define LEVEL 14 /* 最大层数 */

/* bitmap的index换算，因为除去最大内存块的剩余层中64块用一个32位图表示，所以要除以2 */
#define BLOCK_INDEX(index) ((index) >> 1)
#define BLOCK_SHIFT 7                 /* 基本内存块偏移量 */
#define BLOCK_SIZE (1 << BLOCK_SHIFT) /* 基本内存块大小 128b */

#define MEM_NO_ALLOC 0 /* 内存系统状态定义：容量太小不可分配 */
#define MEM_OK 1       /* 内存系统状态定义：容量足够可以分配 */

/**
 * @brief 内存块层数结构体
 *
 */
typedef struct
{
    acoral_8 level_0; /* 标志某块从哪层分配的 */
} acoral_block_level_t;

/**
 * @brief 内存控制块结构体
 *
 */
typedef struct
{
    acoral_32        *free_list[LEVEL]; /* 各层空闲位图链表 */
    acoral_u32       *bitmap[LEVEL];    /* 各层内存状态位图块，两种情况：一. 最大内存块层，为一块内存空闲与否；二.其余层，1 标识两块相邻内存块有一块空闲，0 标识没有空闲 */
    acoral_32         free_cur[LEVEL];  /* 各层空闲位图链表头 */
    acoral_u32        num_l[LEVEL];     /* 各层内存块个数 */
    acoral_8          level;            /* 层数 */
    acoral_u8         state;            /* 状态 */
    acoral_u32        start_adr;        /* 内存起始地址 */
    acoral_u32        end_adr;          /* 内存终止地址 */
    acoral_u32        block_num;        /* 基本内存块数 */
    acoral_u32        free_num;         /* 空闲内存块数 */
    acoral_u32        block_size;       /* 基本内存块大小 */
    acoral_spinlock_t lock;             /* 自旋锁 */
} acoral_block_ctr_t;

acoral_block_ctr_t   *acoral_mem_ctrl;   /* 内存控制块实例指针 */
acoral_block_level_t *acoral_mem_blocks; /* 内存块层数实例指针 */

/**
 * @brief 伙伴系统扫描，查看是否有空闲块
 *
 */
void buddy_scan(void)
{
    acoral_u32 num_perlevel;
    acoral_u32 max_level_1 = acoral_mem_ctrl->level;
    for (acoral_u32 i = 0; i < max_level_1; i++)
    {
        acoral_print("Level%d\r\n", i);
        acoral_prints("bitmap:");
        num_perlevel = acoral_mem_ctrl->num_l[i];
        for (acoral_u32 k = 0; k < num_perlevel;)
        {
            for (acoral_u32 n = 0; n < 8 && k < num_perlevel; n++, k++)
            {
                acoral_print("%x ", acoral_mem_ctrl->bitmap[i][k]);
            }
            acoral_prints("\r\n");
        }
        acoral_print("Free Block head:%d\r\n", acoral_mem_ctrl->free_cur[i]);
        acoral_prints("\r\n");
    }
    acoral_print("Free Mem Block Number:%d\r\n", acoral_mem_ctrl->free_num);
    acoral_print("\r\n");
}

/**
 * @brief 伙伴系统初始化
 *
 * @param start_adr
 * @param end_adr
 * @return acoral_err
 */
acoral_err buddy_init(acoral_u32 start_adr, acoral_u32 end_adr)
{
    acoral_32 i; /* 用于循环控制 */

    start_adr += 3;
    start_adr &= ~(4 - 1); /* 首地址四字节对齐 */
    end_adr &= ~(4 - 1);   /* 尾地址四字节对齐 */

    end_adr  = end_adr - sizeof(acoral_block_ctr_t);  /* 减去内存控制块的大小，剩下的才是可分配内存 */
    end_adr &= ~(4 - 1);                             /* 尾地址再进行一次四字节对齐 */

    acoral_mem_ctrl = (acoral_block_ctr_t *)end_adr; /* 内存控制块的地址 */

    /* 如果内存太少，不适合分配 */
    if (start_adr > end_adr || end_adr - start_adr < BLOCK_SIZE)
    {
        acoral_mem_ctrl->state = MEM_NO_ALLOC;

        return KR_MEM_ERR_LESS;
    }
    acoral_mem_ctrl->state = MEM_OK;

    acoral_u32 resize_size    = BLOCK_SIZE;
    acoral_u32 num            = 1;
    acoral_u32 adjust_level_1 = 1;
    while (1) /* 计算分配层数 */
    {
        if (end_adr <= start_adr + resize_size)
        {
            break;
        }

        resize_size = resize_size << 1;
        num         = num << 1; /* 全分成基本内存块的数量 */
        adjust_level_1++;
    }

    acoral_mem_blocks = (acoral_block_level_t *)end_adr - num; /* 减去基本内存块位图大小 */
    acoral_32 level_1 = adjust_level_1; /* 实际层数 */
    /* 如果层数较小，则最大层用一块构成，如果层数较多，限制层数范围，最大层由多块构成 */
    if (adjust_level_1 > LEVEL)
    {
        level_1 = LEVEL;
    }
    num = num / 32; /* 每32个基本内存块由一个32位位图表示 */

    acoral_u32 save_adr = (acoral_u32)acoral_mem_blocks;
    for (i = 0; i < level_1 - 1; i++)
    {
        num = num >> 1; /* 除去最大层，其他每层的32位图都是64个块构成，所以要除以2 */
        if (num == 0)   /* 不足一个位图的，用一个位图表示 */
        {
            num = 1;
        }
        save_adr -= num * 4;                                   /* 每一个32位位图为4个字节 */
        save_adr &= ~(4 - 1);                                  /* 四字节对齐 */
        acoral_mem_ctrl->bitmap[i] = (acoral_u32 *)save_adr;   /* 当层bitmap地址 */
        acoral_mem_ctrl->num_l[i]  = num;
        save_adr -= num * 4;                                   /* 每一个32位位图为4个字节 */
        save_adr &= ~(4 - 1);                                  /* 四字节对齐 */
        acoral_mem_ctrl->free_list[i] = (acoral_32 *)save_adr; /* 当层free_list地址 */
        for (acoral_32 k = 0; k < num; k++)
        {
            acoral_mem_ctrl->bitmap[i][k]    = 0;     /* 初始化当层bitmap */
            acoral_mem_ctrl->free_list[i][k] = -1; /* 初始化当层free_list */
        }
        acoral_mem_ctrl->free_cur[i] = -1; /* 初始化当层free_cur */
    }

    /* 最大内存块层如果不足一个位图的，用一个位图表示 */
    if (num == 0)
    {
        num = 1;
    }

    /* 继续完成最大内存块层的初始化，和前面一样 */
    save_adr -= num * 4;
    save_adr &= ~(4 - 1);
    acoral_mem_ctrl->bitmap[i] = (acoral_u32 *)save_adr;
    acoral_mem_ctrl->num_l[i]  = num;
    save_adr -= num * 4;
    save_adr &= ~(4 - 1);
    acoral_mem_ctrl->free_list[i] = (acoral_32 *)save_adr;
    for (acoral_32 k = 0; k < num; k++)
    {
        acoral_mem_ctrl->bitmap[i][k]    = 0;
        acoral_mem_ctrl->free_list[i][k] = -1;
    }
    acoral_mem_ctrl->free_cur[i] = -1;

    if (save_adr <= (start_adr + (resize_size >> 1)))
    {
        /* 如果剩余内存大小不够形成现在的level */
        adjust_level_1--;
    }
    if (adjust_level_1 > LEVEL)
    {
        level_1 = LEVEL;
    }

    /* 初始化内存控制块 */
    acoral_mem_ctrl->level      = level_1;
    acoral_mem_ctrl->start_adr  = start_adr;
    num                         = (save_adr - start_adr) >> BLOCK_SHIFT;
    acoral_mem_ctrl->end_adr    = start_adr + (num << BLOCK_SHIFT);
    acoral_mem_ctrl->block_num  = num;
    acoral_mem_ctrl->free_num   = num;
    acoral_mem_ctrl->block_size = BLOCK_SIZE;

    i = 0;
    acoral_u32 max_num = 1 << (level_1 - 1); /* 最大内存块层的每块内存大小 */
    acoral_u32 o_num   = 0;

    /* 有内存块，则最大内存块层的free_cur为0 */
    if (num > 0) 
    {
        acoral_mem_ctrl->free_cur[level_1 - 1] = 0;
    }
    else
    {
        /* 无内存块，则最大内存块层的free_cur为-1 */
        acoral_mem_ctrl->free_cur[level_1 - 1] = -1;
    }

    /* 整块内存优先分给最大内存块层 */
    while (num >= max_num * 32) /* 计算当前可分配内存容量是否能直接形成一个最大内存块层的32位图 */
    {
        acoral_mem_ctrl->bitmap[level_1 - 1][i]    = -1;
        acoral_mem_ctrl->free_list[level_1 - 1][i] = i + 1;
        num -= max_num * 32;
        o_num += max_num * 32;
        i++;
    }

    /* 所有块正好分配到最大内存块层的32位图 */
    if (num == 0) 
    {
        acoral_mem_ctrl->free_list[level_1 - 1][i - 1] = -1;
    }

    acoral_u32 index;
    while (num >= max_num) /* 计算当前可分配内存是否还能形成最大内存块层的一块 */
    {
        index = o_num >> (level_1 - 1);
        acoral_set_bit(index, acoral_mem_ctrl->bitmap[level_1 - 1]);
        num   -= max_num;
        o_num += max_num;
    }
    acoral_mem_ctrl->free_list[level_1 - 1][i] = -1;

    while (--level_1 > 0) /* 接下来的每层初始化 */
    {
        index = o_num >> level_1;
        if (num == 0)
        {
            break;
        }

        acoral_u32 cur = index / 32;
        max_num = 1 << (level_1 - 1); /* 每层的内存块大小 */
        if (num >= max_num)
        {
            acoral_mem_blocks[BLOCK_INDEX(o_num)].level_0 = -1;
            acoral_set_bit(index, acoral_mem_ctrl->bitmap[level_1 - 1]);
            acoral_mem_ctrl->free_list[level_1 - 1][cur]  = -1;
            acoral_mem_ctrl->free_cur[level_1 - 1]        = cur;
            o_num += max_num;
            num   -= max_num;
        }
    }
    acoral_spin_init(&acoral_mem_ctrl->lock); /* 自旋锁初始化 */
    return KR_OK;
}
/**
 * @brief 迭代获取空闲块的首num
 *
 * @param level_0 要获取的层数，起始为0
 * @return acoral_32 空闲块的首num
 */
static acoral_32 recus_malloc(acoral_32 level_0)
{
    /* 层数超出范围 */
    if (level_0 >= acoral_mem_ctrl->level)
    {
        return KR_MEM_ERR_UNDEF;
    }

    acoral_32 num;
    acoral_u32 index;
    acoral_32 cur = acoral_mem_ctrl->free_cur[level_0]; /* 获取首个空闲位图 */
    if (cur < 0)
    {
        /* 无空闲 */
        num = recus_malloc(level_0 + 1); /* 迭代向上寻找 */
        if (num < 0)
        {
            return KR_MEM_ERR_UNDEF;
        }
        index = num >> (level_0 + 1);                            /* 计算在位图中位置 */
        cur   = index / 32;                                      /* 计算在位图链表中位置 */
        acoral_set_bit(index, acoral_mem_ctrl->bitmap[level_0]); /* 在位图中置1，把偶数块分出去 */

        /* 当前层无空闲，两个空闲块是从上层分配的，所以空闲位图链表更改，然后分配完一块还有一块，所以移动空闲位图链表头 */
        acoral_mem_ctrl->free_list[level_0][cur] = -1;
        acoral_mem_ctrl->free_cur[level_0]       = cur;

        return num;
    }

    /* 有空闲内存 */
    index = acoral_find_first_set(acoral_mem_ctrl->bitmap[level_0][cur]);              /* 获取空闲块在其32位图中的位置 */
    index = cur * 32 + index;                                                          /* 计算空闲块实际位置 */
    acoral_clear_bit(index, acoral_mem_ctrl->bitmap[level_0]);                         /* 从只有一块空闲变成两块都不空闲了，所以清0 */
    if (acoral_mem_ctrl->bitmap[level_0][cur] == 0)                                    /* 如果此位图无空闲块了 */
    {
        acoral_mem_ctrl->free_cur[level_0] = acoral_mem_ctrl->free_list[level_0][cur]; /* 移动空闲位图链表头 */
    }
    num = index << (level_0 + 1);                                                      /* 计算空闲块首个num */

    /* 最大内存块层 */
    if (level_0 == acoral_mem_ctrl->level - 1)
    {
        /* 容量不够 */
        if ((num >> 1) + (1 << level_0) > acoral_mem_ctrl->block_num)
        {
            return KR_MEM_ERR_UNDEF;
        }
        return num >> 1;
    }
    /* 其余层 */
    if (acoral_mem_blocks[BLOCK_INDEX(num)].level_0 >= 0) /* 检查这块内存有没有被分配 */
    {
        return num + (1 << level_0);                      /* 如果此块已经被分配，那就是后面一块为空闲块 */
    }

    return num;
}
/**
 * @brief 伙伴系统实际内存分配
 *
 * @param level_0 要分配的层数，起始为0
 * @return void* 返回分配的地址
 */
static void *buddy_r_malloc(acoral_u8 level_0)
{

    acoral_32 num;
    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&acoral_mem_ctrl->lock);
#endif
    acoral_mem_ctrl->free_num -= 1 << level_0; /* 提前减去即将分配的基本内存块数 */
    num = recus_malloc(level_0);               /* 这个函数会一直迭代向上寻找 */
    if ((num & 0x1) == 0)                      /* 偶数块才需要标识自己由哪一层分配而来 */
    {
        acoral_mem_blocks[BLOCK_INDEX(num)].level_0 = level_0;
    }
#ifdef CFG_SMP
    acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
    acoral_exit_critical();

    return (void *)(acoral_mem_ctrl->start_adr + (num << BLOCK_SHIFT));
}
/**
 * @brief 伙伴系统内存计算（获取想使用一定数量内存时实际分配的内存大小）
 *
 * @param size 想使用内存的大小
 * @return acoral_u32 实际分配的内存大小
 */
acoral_u32 buddy_malloc_size(acoral_u32 size)
{
    if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
    {
        return 0;
    }

    acoral_u32 resize_size = BLOCK_SIZE;
    acoral_u8  level_0     = 0;
    acoral_u32 num         = 1;
    while (resize_size < size && level_0 < acoral_mem_ctrl->level)
    {
        num = num << 1;
        level_0++;
        resize_size = resize_size << 1;
    }

    return resize_size;
}
/**
 * @brief 伙伴系统分配内存
 *
 * @param size 要分配的大小
 * @return void* 返回分配的地址
 */
void *buddy_malloc(acoral_u32 size)
{
    if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
    {
        return NULL;
    }

    acoral_u32 resize_size = BLOCK_SIZE;
    acoral_u8  level_0     = 0;
    acoral_u32 num         = 1;
    while (resize_size < size) /* 本层块大小不满足申请内存 */
    {
        num = num << 1;
        level_0++;
        resize_size = resize_size << 1;
    }

    if (num > acoral_mem_ctrl->free_num) /* 剩余内存不足 */
    {
        return NULL;
    }
    if (level_0 >= acoral_mem_ctrl->level) /* 申请内存块大小超过顶层内存块大小 */
    {
        return NULL;
    }

    return buddy_r_malloc(level_0); /* 实际的分配函数 */
}
/**
 * @brief 伙伴系统回收内存
 *
 * @param ptr 要回收的地址
 */
void buddy_free(void *ptr)
{
    if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
    {
        return;
    }

    acoral_u32 num;
    acoral_u32 adr = (acoral_u32)ptr;
    /* 无效地址 */
    if (ptr == NULL || adr < acoral_mem_ctrl->start_adr || adr + BLOCK_SIZE > acoral_mem_ctrl->end_adr)
    {
        acoral_printerr("Invalid Free Address:0x%x\n", ptr);
        return;
    }

    acoral_u32 max_level_1 = acoral_mem_ctrl->level;         /* 记下层数 */
    num = (adr - acoral_mem_ctrl->start_adr) >> BLOCK_SHIFT; /* 地址与基本块数换算 */
    /* 如果不是block整数倍，肯定是非法地址 */
    if (adr != acoral_mem_ctrl->start_adr + (num << BLOCK_SHIFT))
    {
        acoral_printerr("Invalid Free Address:0x%x\n", ptr);
        return;
    }

    acoral_enter_critical();
#ifdef CFG_SMP
    acoral_spin_lock(&acoral_mem_ctrl->lock);
#endif

    acoral_8 buddy_level_0;
    acoral_u32 index;
    acoral_8 level_0;
    if (num & 0x1) /* 奇数基本内存块 */
    {
        level_0 = 0; /* 奇数基本内存块一定是从0层分配 */
        /* 下面是地址检查 */
        index         = BLOCK_INDEX(num);
        buddy_level_0 = acoral_mem_blocks[index].level_0; /* 查看该块是从哪一层分配的 */
        /* 不是从0层，直接返回错误 */
        if (buddy_level_0 > 0)
        {
            acoral_printerr("Invalid Free Address:0x%x\n", ptr);
#ifdef CFG_SMP
            acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
            acoral_exit_critical();

            return;
        }
        /* 伙伴分配出去，如果对应的位为1,肯定是回收过伙伴 */
        if (buddy_level_0 == 0 && acoral_get_bit(index, acoral_mem_ctrl->bitmap[level_0]))
        {
            acoral_printerr("Address:0x%x have been freed\n", ptr);
#ifdef CFG_SMP
            acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
            acoral_exit_critical();

            return;
        }
        /* 伙伴没有分配出去，如果对应的位为0,肯定是已经向上回收过 */
        if (buddy_level_0 < 0 && !acoral_get_bit(index, acoral_mem_ctrl->bitmap[level_0]))
        {
            acoral_printerr("Address:0x%x have been freed\n", ptr);
#ifdef CFG_SMP
            acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
            acoral_exit_critical();

            return;
        }
    }
    else
    {
        /* 查看该块是从哪一层分配的 */
        level_0 = acoral_mem_blocks[BLOCK_INDEX(num)].level_0;
        /* 已经释放 */
        if (level_0 < 0)
        {
            acoral_printerr("Address:0x%x have been freed\n", ptr);
#ifdef CFG_SMP
            acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
            acoral_exit_critical();

            return;
        }
        acoral_mem_ctrl->free_num += 1 << level_0;        /* 空闲基本块数增加 */
        acoral_mem_blocks[BLOCK_INDEX(num)].level_0 = -1; /* 标志此基本块未被分配 */
    }
    if (level_0 == max_level_1 - 1) /* 最大内存块直接回收 */
    {
        index = num >> level_0;                                  /* 最大内存块层，一块一位 */
        acoral_set_bit(index, acoral_mem_ctrl->bitmap[level_0]); /* 设置空闲标志位 */
#ifdef CFG_SMP
        acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
        acoral_exit_critical();

        return;
    }
    index = num >> (1 + level_0); /* 其余层，两块一位 */


    acoral_32 cur, temp_cur;
    while (level_0 < max_level_1) /* 其余层回收，有可能回收到最大层 */
    {
        cur = index / 32;

        /* 两块都没空闲 */
        if (!acoral_get_bit(index, acoral_mem_ctrl->bitmap[level_0]))
        {
            /* 设置成有一块空闲 */
            acoral_set_bit(index, acoral_mem_ctrl->bitmap[level_0]);

            /* 无空闲块或者释放的位比空闲位图链表头小 */
            if (acoral_mem_ctrl->free_cur[level_0] < 0 || cur < acoral_mem_ctrl->free_cur[level_0])
            {
                acoral_mem_ctrl->free_list[level_0][cur] = acoral_mem_ctrl->free_cur[level_0];
                acoral_mem_ctrl->free_cur[level_0]       = cur;
            }
            else if (cur > acoral_mem_ctrl->free_cur[level_0])
            {
                /* 释放的位比空闲位图链表头大 */

                /* 遍历链表到添加的位置 */
                temp_cur = acoral_mem_ctrl->free_cur[level_0];
                while ((acoral_mem_ctrl->free_list[level_0][temp_cur] != -1) &&
                       (acoral_mem_ctrl->free_list[level_0][temp_cur] < cur))
                {
                    temp_cur = acoral_mem_ctrl->free_list[level_0][temp_cur];
                }

                /* 添加位图，此时空闲位图链表头不用改变 */
                acoral_mem_ctrl->free_list[level_0][cur]      = acoral_mem_ctrl->free_list[level_0][temp_cur];
                acoral_mem_ctrl->free_list[level_0][temp_cur] = cur;
            }

            break;
        }
        /* 有个伙伴是空闲的，向上回收 */
        acoral_clear_bit(index, acoral_mem_ctrl->bitmap[level_0]);
        temp_cur = acoral_mem_ctrl->free_cur[level_0];

        do {
            if (cur == temp_cur) /* 空闲位图链表头就是当前位图 */
            {
                break;
            }
            temp_cur = acoral_mem_ctrl->free_list[level_0][temp_cur]; /* 不是的话，遍历寻找 */
        } while (acoral_mem_ctrl->free_list[level_0][temp_cur] != cur);

        if (acoral_mem_ctrl->bitmap[level_0][temp_cur] == 0) /* 如果回收后整个位图无空闲块，空闲位图链表头移到下一个空闲块 */
        {
            acoral_mem_ctrl->free_cur[level_0] = acoral_mem_ctrl->free_list[level_0][cur];
        }

        /* 迭代操作 */
        level_0++;
        if (level_0 < max_level_1 - 1) /* 除了最大内存块层，都要除以2 */
        {
            index = index >> 1;
        }
    }
    acoral_exit_critical();
#ifdef CFG_SMP
    acoral_spin_unlock(&acoral_mem_ctrl->lock);
#endif
}
/**************************************伙伴系统**********************************************/

/**********************************任意大小内存分配******************************************/
/* 魔术数 */
#define MAGIC 0xcc
/* 魔术数掩码 */
#define MAGIC_MASK 0xfe
/* 块被使用的标识 */
#define USED 1
/* 块空闲的标识 */
#define FREE 0
/* 状态标识掩码 */
#define USETAG_MASK 0x1
/* 块大小掩码 */
#define SIZE_MASK 0xffffff00
/* 块大小移动位数 */
#define SIZE_SHIFT 8
/* 使用魔术数进行校验 */
#define BLOCK_CHECK(value)        (((value & MAGIC_MASK) == MAGIC))
/* 得到块的大小 */
#define BLOCK_GET_SIZE(value)     ((value & SIZE_MASK) >> SIZE_SHIFT)
/* 得到块的状态 */
#define BLOCK_TAG(value)          (value & USETAG_MASK)
/* 检测块是否空闲 */
#define BLOCK_IS_FREE(value)      (BLOCK_TAG(value) == FREE)
/* 检测块是是否被使用 */
#define BLOCK_IS_USED(value)      (BLOCK_TAG(value) == USED)
/* 设置块被使用 */
#define BLOCK_SET_USED(ptr, size) (*ptr = ((size) << SIZE_SHIFT) | 0x1 | MAGIC)
/* 设置块空闲 */
#define BLOCK_SET_FREE(ptr, size) (*ptr = ((size) << SIZE_SHIFT) | MAGIC)
/* 清除块指针 */
#define BLOCK_CLEAR(ptr)          (*ptr = 0)

/**
 * @brief 任意大小内存分配控制块
 *
 */
struct vol_mem_ctrl_t
{
    acoral_ipc_t mutex;     /* 互斥量 */
    acoral_8    *top_p;     /* 内存顶指针 */
    acoral_8    *down_p;    /* 内存底指针 */
    acoral_u32  *freep_p;   /* 空闲内存指针 */
    acoral_u8    mem_state; /* 内存状态 */
};

/* 任意大小内存分配控制块实例 */
struct vol_mem_ctrl_t vol_mem_ctrl;

/**
 * @brief 任意大小内存系统初始化
 *
 */
void vol_mem_init()
{
    acoral_size size    = acoral_malloc_size(CFG_MEM_VOL_SIZE);           /* 计算任意大小内存可分配容量 */
    vol_mem_ctrl.down_p = (acoral_8 *)acoral_malloc(size); /* 分配任意大小内存可分配容量 */
    if (vol_mem_ctrl.down_p == NULL)
    {
        vol_mem_ctrl.mem_state = 0;

        return;
    }
    else
    {
        vol_mem_ctrl.mem_state = 1;
    }

    acoral_mutex_init(&vol_mem_ctrl.mutex);                   /* 互斥量初始化 */
    vol_mem_ctrl.top_p   = vol_mem_ctrl.down_p + size;          /* 可任意分配内存顶 */
    vol_mem_ctrl.freep_p = (acoral_u32 *)vol_mem_ctrl.down_p; /* 可任意分配内存空闲块指针 */
    BLOCK_SET_FREE(vol_mem_ctrl.freep_p, size);               /* 设置一整块空闲 */
}
/**
 * @brief 任意大小内存实际分配
 *
 * @param size 分配的大小
 * @return void* 地址
 */
static void *vol_r_malloc(acoral_32 size)
{
    size = size + 4;
    acoral_mutex_pend(&vol_mem_ctrl.mutex);

    acoral_u32  b_size;
    acoral_u32 *tp  = vol_mem_ctrl.freep_p;
    acoral_8   *ctp = (acoral_8 *)tp;
    while (ctp < vol_mem_ctrl.top_p)
    {
        b_size = BLOCK_GET_SIZE(*tp); /* 该块size */
        if (0 == b_size)              /* 该块size为0 */
        {
            acoral_printerr("Err address is 0x%x,size should not be 0", tp);
            acoral_mutex_post(&vol_mem_ctrl.mutex); /* 释放互斥量 */

            return NULL;
        }
        if (BLOCK_IS_USED(*tp) || b_size < size) /* 该块被使用或者容量小于size */
        {
            /* 找到下一个空闲块指针 */
            ctp = ctp + b_size;
            tp  = (acoral_u32 *)ctp;
        }
        else /* 该块容量足够且空闲 */
        {
            BLOCK_SET_USED(tp, size); /* 标志使用 */
            ctp = ctp + size;
            tp  = (acoral_u32 *)ctp;                 /* 移动到所需的size末尾 */
            if (b_size - size > 0)                  /* 此前分配的容量大于即将分配的size */
            {
                BLOCK_SET_FREE(tp, b_size - size);  /* 设置后面的内存空闲，造成碎片 */
            }
            vol_mem_ctrl.freep_p = tp;              /* 移动空闲块指针 */
            acoral_mutex_post(&vol_mem_ctrl.mutex); /* 释放互斥量 */

            return (void *)(ctp - size + 4);
        }
    }
    ctp = vol_mem_ctrl.down_p;
    tp  = (acoral_u32 *)ctp;
    while (tp < vol_mem_ctrl.freep_p)
    {
        b_size = BLOCK_GET_SIZE(*tp);
        if (b_size == 0)
        {
            acoral_printerr("Err address is 0x%x,size should not be 0", tp);
            acoral_mutex_post(&vol_mem_ctrl.mutex);
            
            return NULL;
        }
        if (BLOCK_IS_USED(*tp) || b_size < size)
        {
            ctp = ctp + b_size;
            tp  = (acoral_u32 *)ctp;
        }
        else
        {
            BLOCK_SET_USED(tp, size);
            ctp = ctp + size;
            tp  = (acoral_u32 *)ctp;
            if (b_size - size > 0)
            {
                BLOCK_SET_FREE(tp, b_size - size);
            }
            vol_mem_ctrl.freep_p = tp;
            acoral_mutex_post(&vol_mem_ctrl.mutex);

            return (void *)(ctp - size + 4);
        }
    }

    acoral_mutex_post(&vol_mem_ctrl.mutex);

    return NULL;
}
/**
 * @brief 任意大小内存分配
 *
 * @param size 分配的大小
 * @return void* 地址
 */
void *vol_malloc(acoral_32 size)
{
    if (vol_mem_ctrl.mem_state == 0)
    {
        return NULL;
    }

    size = (size + 3) & ~3; /* 四字节对齐 */
    
    return vol_r_malloc(size);
}
/**
 * @brief 任意大小内存回收
 *
 * @param p 回收的地址
 */
void vol_free(void *p)
{
    if (vol_mem_ctrl.mem_state == 0)
    {
        return;
    }

    p = (acoral_8 *)p - 4;
    acoral_u32 *tp = (acoral_u32 *)p;
    acoral_mutex_pend(&vol_mem_ctrl.mutex);
    /* 无效地址检测 */
    if (p == NULL || (acoral_8 *)p < vol_mem_ctrl.down_p || (acoral_8 *)p >= vol_mem_ctrl.top_p || !BLOCK_CHECK(*tp))
    {
        acoral_printerr("Invalide Free address:0x%x\n", tp);
        return;
    }
    /* 已经空闲了 */
    if (BLOCK_IS_FREE(*tp))
    {
        acoral_printerr("Address:0x%x have been freed\n", tp);
        return;
    }
    acoral_u32 *prev_tp = tp;
    acoral_8 *ctp       = (acoral_8 *)tp;
    acoral_u32 b_size   = BLOCK_GET_SIZE(*tp); /* 该块size */

    ctp = ctp + b_size;
    tp  = (acoral_u32 *)ctp;
    acoral_u32 size; 
    if (BLOCK_IS_FREE(*tp)) /* 检测后面的块是否空闲 */
    {
        size = BLOCK_GET_SIZE(*tp); /* 获取后面块的size */
        if (size == 0)              /* 检测size */
        {
            acoral_printerr("Err address is 0x%x,size should not be 0", tp);
            acoral_mutex_post(&vol_mem_ctrl.mutex);
            return;
        }
        b_size += size;  /* 合并大小 */
        BLOCK_CLEAR(tp); /* 清除后面块的控制块 */
    }

    BLOCK_SET_FREE(prev_tp, b_size); /* 设置合并后的块空闲 */
    vol_mem_ctrl.freep_p = prev_tp;

    if (p == vol_mem_ctrl.down_p) /* 回收地址就是首地址 */
    {
        acoral_mutex_post(&vol_mem_ctrl.mutex);

        return;
    }

    ctp = vol_mem_ctrl.down_p;
    tp  = (acoral_u32 *)ctp;
    while (ctp < (acoral_8 *)p) /* 找到前一块指针 */
    {
        size = BLOCK_GET_SIZE(*tp); /* 获取块size */
        if (size == 0)              /* 检测size */
        {
            acoral_printerr("err address is 0x%x,size should not be 0", tp);
            acoral_mutex_post(&vol_mem_ctrl.mutex);

            return;
        }

        ctp     = ctp + size;
        prev_tp = tp;
        tp      = (acoral_u32 *)ctp;
    }

    if (BLOCK_IS_FREE(*prev_tp)) /* 若前一块空闲，可合并 */
    {
        tp = (acoral_u32 *)p;
        BLOCK_CLEAR(tp);
        BLOCK_SET_FREE(prev_tp, b_size + size);
        vol_mem_ctrl.freep_p = prev_tp;
    }

    acoral_mutex_post(&vol_mem_ctrl.mutex);
}

/**
 * @brief 任意大小内存系统扫描
 *
 */
void vol_mem_scan(void)
{
    if (vol_mem_ctrl.mem_state == 0)
    {
        acoral_print("Mem Init Err ,so no mem space to malloc\r\n");

        return;
    }
    acoral_8 *ctp = vol_mem_ctrl.down_p;
    do
    {
        acoral_u32 *tp  = (acoral_u32 *)ctp;
        acoral_u32 size = BLOCK_GET_SIZE(*tp);
        if (size == 0)
        {
            acoral_print("Err address is 0x%x,size should not be 0\r\n", tp);

            break;
        }

        if (BLOCK_IS_USED(*tp))
        {
            acoral_print("The address is 0x%x,the block is used and it's size is %d\r\n", tp, size);
        }
        else
        {

            acoral_print("The address is 0x%x,the block is unused and it's size is %d\r\n", tp, size);
        }

        ctp = ctp + size;

    } while (ctp < vol_mem_ctrl.top_p);
}
/**********************************任意大小内存分配******************************************/
