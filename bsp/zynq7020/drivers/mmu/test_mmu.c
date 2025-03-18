/**
 * @file test_mmu.c
 * @author 文佳源 (648137125@qq.com)
 * @brief mmu测试代码
 * @version 0.1
 * @date 2025-02-24
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-02-24 <td>内容
 * </table>
 */
#if 0
#include <stdio.h>
#include "platform.h"
// #include "xil_printf.h"
#include "xil_types.h"
#include "cp15.h"

#include "mmu.h"
#include "usr_vspace.h"

#define CMP(ptr1, ptr2) \
    do{\
        if(*ptr1 == *ptr2)\
        {\
            printf("*(0x%lx) == *(0x%lx) == 0x%lx\r\n", (u32)(ptr1), (u32)(ptr2), (u32)*ptr1);\
        }\
        else\
        {\
            printf("ERROR: *(0x%lx) = 0x%lx but *(0x%lx) = 0x%lx\r\n", (u32)ptr1, (u32)*ptr1, (u32)ptr2, (u32)*ptr2);\
        }\
    }while(0)

volatile unsigned int testmem[4];
void test_mmu(void)
{
	unsigned int *p = testmem, *tp = NULL;
    *p = 0xdeadbeaf;
    tp = (unsigned int*)((unsigned long)p | (unsigned long)0x40000000);

    printf("*(0x%lx)=%x\r\n", (unsigned long)p, *p);

    // init_mmu_only_L1();
    init_mmu_only_L2();

    printf("mem map success\r\n");
    CMP(p, tp);

    *tp = 0xbeafbeaf;
    printf("change tp to 0xbeafbeaf\r\n");
    CMP(p, tp);

    *p = 0xdeaddead;
    printf("change p to 0xdeaddead\r\n");
    CMP(p, tp);

    // dump_l_one_table();

}
#endif
