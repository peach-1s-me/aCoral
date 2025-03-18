/**
 * @file usr_vspace.c
 * @author 文佳源 饶洪江 (648137125@qq.com)
 * @brief mmu用户空间定义
 * @version 0.1
 * @date 2024-08-05
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 饶洪江<td>2024-08-05 <td>内容
 * </table>
 */
#include "usr_vspace.h"
#include "mmu.h"

#define MMU_USR_SPACE_ALLOC_ALIGN       12  /* 分配内存时对齐位数 */

#define MMU_L2TBL_SIZE                  (256*sizeof(u32)) /* 目前每个空间最多1MB->256项二级页表 */

usr_vspace_t uvs1, uvs2;

/* 两个二级页表, 因为每次最多分配1M空间, 所以二级页表只需要256项 */
// volatile static u32 L2table1[256] __attribute__((aligned(4 * 1024))) = {0};
// volatile static u32 L2table2[256] __attribute__((aligned(4 * 1024))) = {0};

/* 两个空间 */
// volatile static u8 us1[0x2000] __attribute__((aligned(4 * 1024))) = {0};
// volatile static u8 us2[0x3000] __attribute__((aligned(4 * 1024))) = {0};

// static u8 usr_space[1024*1024] __attribute__((aligned(4 * 1024)));         /* 临时用户空间 */

// u32 __usr_space_start = usr_space;
// u32 __usr_space_end = usr_space + 1024*1024;

#define SECTION_START(_sec_name) (u32)&_sec_name##_start
#define SECTION_END(_sec_name) (u32)&_sec_name##_end

extern volatile u32 _1st_page_table[4 * 1024];
extern u32 _usr_space_start; /* 用户空间起始地址 */
extern u32 _usr_space_end;   /* 用户空间结束地址(可用的最后一个空间+1) */
static u32 usr_space_alloc_index = 0;   /* 用户空间分配指针 */
extern u32 _L2tables_start;
extern u32 _L2tables_end;
static u32 L2tables_alloc_index = 0;   /* 用户空间分配指针 */

/**
 * @brief 分配二级页表空间
 * 
 * @param  L2table_addr_ptr 存储二级页表地址的变量的指针
 * @return i32 0：成功， else：失败
 */
i32 mmu_alloc_L2_table(u32 *L2table_addr_ptr)
{
    i32 ret = -1;
    if(((u32)&_L2tables_end - L2tables_alloc_index) >= MMU_L2TBL_SIZE)
    {
        printf("_L2tables_start=0x%lx\r\n", (u32)&_L2tables_start);
        *L2table_addr_ptr = (u32)&_L2tables_start + L2tables_alloc_index;
        L2tables_alloc_index += MMU_L2TBL_SIZE;

        ret = 0;
    }
    else
    {
        printf("ERROR: L2tables not enough\r\n");
    }

    return ret;
}

/**
 * @brief 分配对齐的用户物理内存空间
 * 
 * @param  length           需要的长度
 * @return u32              非0:分配的内存首地址, 0: 失败
 */
i32 mmu_alloc_usr_space_aligned(u32 length, u32 *L2table_addr, u32 *paddr)
{
    i32 ret = -1;

    /* 分配用户空间 */
    if((length & 0x00000fff) == 0)
    {
        if(((u32)&_usr_space_end - usr_space_alloc_index) >= length)
        {
            printf("_usr_space_start=0x%lx\r\n", (u32)&_usr_space_start);
            if(((u32)&_L2tables_end - L2tables_alloc_index) >= MMU_L2TBL_SIZE)
            {
                printf("_L2tables_start=0x%lx\r\n", (u32)&_L2tables_start);
                *L2table_addr = (u32)&_L2tables_start + L2tables_alloc_index;
                L2tables_alloc_index += MMU_L2TBL_SIZE;

                ret = 0;
            }
            else
            {
                printf("ERROR: L2tables not enough\r\n");
            }

            if(0 == ret)
            {
                *paddr = (u32)&_usr_space_start + usr_space_alloc_index;
                usr_space_alloc_index += length;
            }
        }
        else
        {
            printf("ERROR: usr_space not enough\r\n");
        }
    }
    else
    {
        printf("ERROR: length is not aligned\r\n");
    }

    return ret;
}

/**
 * @brief 映射用户空间虚拟地址到物理地址
 * 
 * @param  uvs              虚拟内存空间结构体
 * @param  L2table_addr     要写入的第一个二级页表项地址
 * @param  vaddr            要映射的虚拟地址起始
 * @param  paddr            要映射的物理地址起始
 * @param  length           空间长度（如果没对齐则自动向上对齐）
 * @return i32              0：成功， else：失败
 */
i32 mmu_map_usr_space(usr_vspace_t *uvs, u32 L2table_addr, u32 vaddr, u32 paddr, u32 length)
{
    i32 ret = -1;
    volatile int i, j;

    uvs->vaddr_start = vaddr;
    uvs->L2table_addr = L2table_addr;
    uvs->length = length;
    
    volatile u32 ptr_1st_num = uvs->length >> 20;
    if(0 == ptr_1st_num)
    {
        ptr_1st_num = 1;
    }

    volatile u32 pte_2nd_num = 256;
    volatile u32 *pte_2nd = (u32 *)uvs->L2table_addr;
    for(i=0; i<ptr_1st_num; i++)
    {
        if(0 != ((u32)pte_2nd & 0x3ff))
        {
            printf("ERROR: pte_2nd_addr 0x%lx not aligned\r\n", (u32)(uvs->L2table_addr + i));
            while(1);
        }
        /* 这里只填充二级页表, 等到激活时才将二级页表地址写入一级页表 */
        // *ptr_1st = ((u32)pte_2nd)|0x1e1;
        for(j=0; j<pte_2nd_num; j++)
        {
            volatile u32 target_paddr = paddr + (i << 20) + (j << 12);
            if(0 != ((u32)target_paddr & ((1<<12) - 1)))
            {
                printf("ERROR: target_paddr 0x%lx not aligned\r\n", target_paddr);
                while(1);
            }
            
            // *pte_2nd = target_paddr | 0x576;
            *pte_2nd = target_paddr | 0xd76;
            pte_2nd++;
        }
    }

    return ret;
}

/**
 * @brief 激活用户虚拟内存空间, 即切换到使用该虚拟内存对应的物理地址
 * 
 * @param  uvs              虚拟内存结构体
 * @return i32              0: 成功, else:失败
 */
i32 mmu_activate_usr_space(usr_vspace_t *uvs)
{
    i32 ret = -1;

    volatile u32 ptr_1st_index_start = (uvs->vaddr_start >> 20);
    volatile u32 *ptr_1st = (u32*)_1st_page_table + ptr_1st_index_start;
    volatile u32 ptr_1st_num = uvs->length >> 20;

    if(0 == ptr_1st_num)
    {
        ptr_1st_num = 1;
    }

    volatile u32 *pte_2nd = (u32 *)uvs->L2table_addr;
    volatile int i;
    for(i=0; i<ptr_1st_num; i++)
    {
        if(0 != ((u32)pte_2nd & 0x3ff))
        {
            printf("ERROR: pte_2nd_addr 0x%lx not aligned\r\n", (u32)(uvs->L2table_addr + i));
            while(1);
        }
        *ptr_1st = ((u32)pte_2nd)|0x1e1;

        ptr_1st++;
        pte_2nd += 256;
    }

    return ret;
}

#define VADDR       0xc0000000
void test_usr_space(void)
{
    /* 关闭cache和mmu */
    cache_disable();
    mmu_disable();

    /* 将所有地址空间映射到一级页表 */
    mmu_set_L1table(0x0, 0xffffffff, 0x0, 0x15de6);

#if 0
    /* 分配用户内存空间 */
    u32 L2table1_addr, L2table2_addr;
    u32 us1_addr, us2_addr;
    mmu_alloc_usr_space_aligned(0x2000, &L2table1_addr, &us1_addr);
    mmu_alloc_usr_space_aligned(0x3000, &L2table2_addr, &us2_addr);

    /* 将用户空间用二级页表方式映射到对应地址 */
    mmu_map_usr_space(&uvs1, L2table1_addr, VADDR, us1_addr, 0x2000);
    mmu_map_usr_space(&uvs2, L2table2_addr, VADDR, us2_addr, 0x3000);

    /* 设置ttbr0为一级页表地址 */
    mmu_set_L1_table_addr((u32)_1st_page_table);
    mmu_enable();

    /* 测试切换和读写 */
    u32 *p = (u32 *)VADDR;

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x11);
    mmu_activate_usr_space(&uvs1);
    mmu_enable();
    *p = 0xaaaaaaaa;

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x22);
    mmu_activate_usr_space(&uvs2);
    mmu_enable();
    *p = 0xbbbbbbbb;

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x11);
    mmu_activate_usr_space(&uvs1);
    mmu_enable();
    printf("uvs1: *(0x%lx) = 0x%lx\r\n", (u32)p, (u32)*p);

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x22);
    mmu_activate_usr_space(&uvs2);
    mmu_enable();
    printf("uvs2: *(0x%lx) = 0x%lx\r\n", (u32)p, (u32)*p);
#else
    /* 分配用户内存空间 */
    u32 L2table1_addr, L2table2_addr;
    u32 test1_addr = 0x30000000, test2_addr = 0x30100000;
    // mmu_alloc_usr_space_aligned(0x2000, &L2table1_addr, &us1_addr);
    // mmu_alloc_usr_space_aligned(0x3000, &L2table2_addr, &us2_addr);

    mmu_alloc_L2_table(&L2table1_addr);
    mmu_alloc_L2_table(&L2table2_addr);

    /* 将用户空间用二级页表方式映射到对应地址 */
    mmu_map_usr_space(&uvs1, L2table1_addr, VADDR, test1_addr, 0x100000);
    mmu_map_usr_space(&uvs2, L2table2_addr, VADDR, test2_addr, 0x100000);

    /* 设置ttbr0为一级页表地址 */
    mmu_set_L1_table_addr((u32)_1st_page_table);
    mmu_enable();

    /* 测试切换和读写 */
    void (*f)(void) = VADDR;

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x11);
    mmu_activate_usr_space(&uvs1);
    mmu_enable();
    f();

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x22);
    mmu_activate_usr_space(&uvs2);
    mmu_enable();
    f();

    mmu_disable();
    //mmu_tlb_invalidate();
    mmu_set_CONTEXTIDR(0, 0x11);
    mmu_activate_usr_space(&uvs1);
    mmu_enable();
    f();

#endif
}
