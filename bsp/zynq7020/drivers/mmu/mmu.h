/**
 * @file mmu.h
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
 * </table>
 */

#ifndef _MMU_DEMO_H
#define _MMU_DEMO_H

// #define _1st_page_table MMUTable

typedef int i32;

void mmu_pre_init(void);
void mmu_init(void);

void mmu_set_L1_table_addr(u32 addr);
i32 mmu_set_CONTEXTIDR(u32 procid, u32 asid);

void mmu_enable(void);
void mmu_disable(void);

void mmu_tlb_invalidate(void);
void mmu_branch_predictor_invalidate(void);

void cache_enable(void);
void cache_disable(void);

void mmu_set_L1table(u32 vaddr_start,
                      u32 vaddr_end,
                      u32 paddr_start,
                      u32 attr);

void mmu_set_L2table(u32 vaddr_start,
                      u32 vaddr_end,
                      u32 paddr_start,
                      u32 attr);

void mmu_set_attribute(u32 addr, u32 attr);

void test_mmu(void);

#endif
