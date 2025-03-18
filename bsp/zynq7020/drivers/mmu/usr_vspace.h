/**
 * @file usr_vspace.h
 * @author 文佳源 饶洪江 (648137125@qq.com)
 * @brief 
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
#ifndef _USR_VSPACE_H
#define _USR_VSPACE_H

#include <stdio.h>
#include "xil_types.h"
#include "mmu.h"

#define USR_SPACE_TEST        0

#if (USR_SPACE_TEST == 1)
    typedef unsigned int u32;
    typedef unsigned char u8;
#endif

typedef struct _usr_vspace_t
{
    u32 asid;

    u32 vaddr_start;
    // u32 paddr_start;
    u32 length;

    u32 L2table_addr;
} usr_vspace_t;

i32 mmu_alloc_L2_table(u32 *L2table_addr_ptr);
i32 mmu_alloc_usr_space_aligned(u32 length, u32 *L2table_addr, u32 *paddr);
i32 mmu_map_usr_space(usr_vspace_t *uvs, u32 L2table_addr, u32 vaddr, u32 paddr, u32 length);
i32 mmu_activate_usr_space(usr_vspace_t *uvs);

void test_usr_space(void);

#endif