/**
 * @file mmu.c
 * @author 文佳源 饶洪江 (648137125@qq.com)
 * @brief mmu操作相关
 * @version 0.1
 * @date 2024-08-01
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 饶洪江 <td>2024-08-01 <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */

#include <stdio.h>
#include "platform.h"
#include "xil_types.h"
#include "cp15.h"
#include "mmu.h"

int rt_hw_cpu_id(void);

void rt_cpu_dcache_clean_flush(void);
void rt_cpu_icache_flush(void);

void rt_cpu_vector_set_base(unsigned int addr);

/* Instruction Synchronization Barrier */
#define _ISB() __asm__ __volatile__ ("isb" : : : "memory")

/* Data Synchronization Barrier */
#define _DSB() __asm__ __volatile__ ("dsb" : : : "memory")

/* Data Memory Barrier */
#define _DMB() __asm__ __volatile__ ("dmb" : : : "memory")



/* 一级页表项-section类型 */
typedef struct _l_one_pt_entry_section_t
{
    u32 type:        2; /* 一级页表项类型-section=0b10 */
    u32 CB:          2; /*  */
    u32 XN:          1;
    u32 Domain:      4;
    u32 P:           1;
    u32 AP:          2;
    u32 TEX:         3;
    u32 APX:         1;
    u32 S:           1;
    u32 nG:          1;
    u32 isSuperSec:  1;
    u32 SBZ:         1;
    u32 SecBaseAddr: 12;
} __attribute__((packed)) l_one_pt_entry_section_t;
/* 一级页表项-二级页表指针类型 */
typedef struct _l_one_pt_entry_2ndptr_t
{
    u32 type:                2; /* 一级页表项类型-2ndpagetable=0b01 */
    u32 SBZ:                 3; /*  */
    u32 Domain:              4;
    u32 P:                   1;
    u32 L2DesciptorBaseAddr: 22;
} __attribute__((packed)) l_one_pt_entry_2ndptr_t;

volatile u32 _1st_page_table[4 * 1024] __attribute__((aligned(16 * 1024),section(".my_mmu_tbl"))) = {0};
volatile static u32 _2nd_page_table[4 * 1024][256] __attribute__((aligned(16 * 1024),section(".my_mmu_tbl"))) = {0};

void mmu_pre_init(void)
{
    /* 关闭cache和mmu */
    cache_disable();
    mmu_disable();

    /* 将所有地址空间映射到一级页表 */
    mmu_set_L1table(0x0, 0xffffffff, 0x0, 0x15de6);

    /* 设置ttbr0为一级页表地址 */
    mmu_set_L1_table_addr((u32)_1st_page_table);
    mmu_enable();
}

/* 如果用这个就不用 mmu_pre_init() */
/* 初始化mmu和页表 */
void mmu_init(void)
{
    /* 关闭cache和mmu */
    cache_disable();
    mmu_disable();

    /* 将所有地址空间映射到一级页表 */
    mmu_set_L1table(0x0, 0xffffffff, 0x0, 0x15de6);

    /* 按照zynq原本的页表设置 */
    mmu_set_L1table(0x00000000, 0x000fffff, 0x00000000, 0x14de6);
    mmu_set_L1table(0x00100000, 0x3fffffff, 0x00100000, 0x15de6);
    mmu_set_L1table(0x40000000, 0x7fffffff, 0x40000000, 0x00c02);
    mmu_set_L1table(0x80000000, 0xbfffffff, 0x80000000, 0x00c02);
    mmu_set_L1table(0xc0000000, 0xdfffffff, 0xc0000000, 0x00000);
    mmu_set_L1table(0xe0000000, 0xe02fffff, 0xe0000000, 0x00c06);
    mmu_set_L1table(0xe0300000, 0xe0ffffff, 0xe0300000, 0x00000);
    mmu_set_L1table(0xe1000000, 0xe1ffffff, 0xe1000000, 0x00c06);
    mmu_set_L1table(0xe2000000, 0xe3ffffff, 0xe2000000, 0x00c06);
    mmu_set_L1table(0xe4000000, 0xe5ffffff, 0xe4000000, 0x00c0e);
    mmu_set_L1table(0xe6000000, 0xf7ffffff, 0xe6000000, 0x00000);
    /* 这个不加就会产生地址转换错误 */
    mmu_set_L1table(0xf8000000, 0xf8ffffff, 0xf8000000, 0x00c06);
    mmu_set_L1table(0xf9000000, 0xfbffffff, 0xf9000000, 0x00000);
    mmu_set_L1table(0xfc000000, 0xfdffffff, 0xfc000000, 0x00c0a);
    mmu_set_L1table(0xfe000000, 0xffefffff, 0xfe000000, 0x00000);
    mmu_set_L1table(0xfff00000, 0xffffffff, 0xfff00000, 0x04c0e);

    /* 设置ttbr0为一级页表地址 */
    mmu_set_L1_table_addr((u32)_1st_page_table);
    mmu_enable();
}

void mmu_set_L1_table_addr(u32 addr)
{
    addr |= 0x5b;
    SET_TTBR0(addr);
}

/**
 * @brief 设置CONTEXTIDR(目前主要是ASID)
 * 
 * @param  procid           进程ID
 * @param  asid             ASID(AddressSpecificID)
 * @return i32      0:成功, else:失败
 */
i32 mmu_set_CONTEXTIDR(u32 procid, u32 asid)
{
    if(
        ((procid & ~0x00ffffff) != 0) ||
        ((asid &   ~0x000000ff) != 0)
    )
    {
        /* 超出范围 */
        printf("ERROR: invalid paras\r\n");
        return -1;
    }

    u32 contextidr = (procid << 8) | (asid << 0);

    SET_CONTEXTIDR(contextidr);
    _DSB();
    _ISB();

    return 0;
}
    

/**
 * @brief 使能mmu
 * 
 */
void mmu_enable(void)
{
    volatile u32 reg = GET_SCTLR();
    reg |= SCTLR_M;
    SET_SCTLR(reg);
    _DSB();
    _ISB();
}

/**
 * @brief 关闭mmu
 * 
 */
void mmu_disable(void)
{
    volatile u32 reg = GET_SCTLR();
    reg &= ~SCTLR_M;
    SET_SCTLR(reg);
    // _DSB();
}

void mmu_tlb_invalidate(void)
{
    SET_ITLBIALL();
}

void mmu_branch_predictor_invalidate(void)
{
    SET_BPIALL();
}

void cache_enable(void)
{
    volatile u32 reg = GET_SCTLR();
    reg |= (SCTLR_C | SCTLR_I);
    SET_SCTLR(reg);
    _DSB();
}

void cache_disable(void)
{
    volatile u32 reg = GET_SCTLR();
    reg &= ~(SCTLR_C | SCTLR_I);
    SET_SCTLR(reg);
    _DSB();
}

void dump_l_one_table(void)
{
    unsigned int i=0;
    printf("***level 1 page table dump ***\r\n");
    for(i=0; i<4*1024; i++)
    {
        l_one_pt_entry_section_t *pte = (l_one_pt_entry_section_t*)&_1st_page_table[i];
        if(0x0 == pte->type)
        {
            printf("nope ");
            continue;
        }

        printf("-%d-: type(%x) CB(%x) XN(%x) Domain(%x) P(%x) AP(%x) TEX(%x) APX(%x) S(%x) nG(%x) isSuperSec(%x) SBZ(%x) SecBaseAddr(%x)\r\n",
            i,
            pte->type       ,
            pte->CB         ,
            pte->XN         ,
            pte->Domain     ,
            pte->P          ,
            pte->AP         ,
            pte->TEX        ,
            pte->APX        ,
            pte->S          ,
            pte->nG         ,
            pte->isSuperSec ,
            pte->SBZ        ,
            pte->SecBaseAddr
        );
    }
}

// extern unsigned long _1st_page_table[1023];
void mmu_set_L1table(u32 vaddr_start,
                      u32 vaddr_end,
                      u32 paddr_start,
                      u32 attr)
{
    volatile u32 *pTT;
    volatile int i, nSec;

    if(((vaddr_end - vaddr_start + 1) & 0xfffff) != 0)
    {
        /* the space is not aligned with 1MB */
        while(1);
    }

    pTT = (u32 *)_1st_page_table + (vaddr_start >> 20);
    nSec = (vaddr_end >> 20) - (vaddr_start >> 20);
    for (i = 0; i <= nSec; i++)
    {
        *pTT = attr | (((paddr_start >> 20) + i) << 20);
        pTT++;
    }
}

// void init_mmu_only_L1(void)
// {
//     cache_disable();
//     mmu_disable();

//      unsigned int attr = 0x15de6;
//     mmu_set_L1table(0x0, 0xffffffff, 0x0, attr);
//     mmu_set_L1table(0x40100000, 0x80000000, 0x100000, attr);

//     rt_cpu_tlb_set(_1st_page_table);

//     mmu_enable();
// }

void mmu_set_L2table(u32 vaddr_start,
                      u32 vaddr_end,
                      u32 paddr_start,
                      u32 attr)
{
    (void)attr;
    volatile u32 ptr_1st_index_start = (vaddr_start >> 20);
    volatile u32 *ptr_1st = (u32*)_1st_page_table + ptr_1st_index_start;
    volatile u32 ptr_1st_num = (vaddr_end >> 20) - (vaddr_start >> 20);
    volatile int i, j;

    for(i=0; i<ptr_1st_num; i++)
    {
        volatile u32 *pte_2nd = &(_2nd_page_table[ptr_1st_index_start+i][0]);
        if(0 != ((u32)pte_2nd & 0x3ff))
        {
            printf("ERROR: pte_2nd_addr 0x%lx not aligned\r\n", (u32)&(_2nd_page_table[ptr_1st_index_start+i][0]));
            while(1);
        }
        *ptr_1st = ((u32)pte_2nd)|0x1e1;
        // printf("-pte *(0x%lx)=0x%lx addr<0x%x> attr<0x%x>\r\n", ptr_1st, *ptr_1st, (u32)pte_2nd, 0b0111100001);

        volatile u32 pte_2nd_num = 256;
        for(j=0; j<pte_2nd_num; j++)
        {
            volatile u32 target_paddr = paddr_start + (i << 20) + (j << 12);
            if(0 != ((u32)target_paddr & ((1<<12) - 1)))
            {
                printf("ERROR: target_paddr 0x%lx not aligned\r\n", target_paddr);
                while(1);
            }

            *pte_2nd = target_paddr | 0x576;
            pte_2nd++;
        }

        ptr_1st++;
    }

    printf("set mtt success\r\n");
}

void mmu_set_attribute(u32 addr, u32 attr)
{
    volatile u32 ptr_1st_index_start = (addr >> 20);
    volatile u32 *ptr_1st = (u32*)_1st_page_table + ptr_1st_index_start;

	if(ptr_1st != NULL) {
		*ptr_1st = (addr & 0xFFF00000U) | attr;
	}

	// Xil_DCacheFlush();

	mmu_tlb_invalidate();
	/* Invalidate all branch predictors */
	mmu_branch_predictor_invalidate();

	_DSB(); /* ensure completion of the BP and TLB invalidation */
    _ISB(); /* synchronize context on this processor */
}

// void init_mmu_only_L2(void)
// {
//     cache_disable();
//     mmu_disable();

//     /* set page table */
//     /* 4G 1:1 memory */
//      unsigned int attr = 0x15de6; /* 0x0001 0101 1101 1110 0110 */
//     mmu_set_L2table(0x0, 0xffffffff, 0x0, attr);
//     mmu_set_L2table(0x40100000, 0x80000000, 0x100000, attr);

//     rt_cpu_tlb_set(_1st_page_table);

//     mmu_enable();
// }


