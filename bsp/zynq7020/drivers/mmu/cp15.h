/**
 * @file cp15.h
 * @author 文佳源 饶洪江 (648137125@qq.com)
 * @brief cp15寄存器(mmu相关)操作
 * @version 0.1
 * @date 2024-07-26
 *
 * Copyright (c) 2024
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 饶洪江 <td>2024-07-26 <td>内容
 * </table>
 */
#ifndef _CP15_H
#define _CP15_H

#include <stdio.h>
#include "xil_types.h"

/**
 * set cp15
 */
#define _MCR_P15(op1, crn, crm, op2, clobbers, val)                        \
    ({                                                                     \
        register u32 __val = val;                                     \
        __asm__ __volatile__("mcr p15," #op1 ",%0," #crn "," #crm "," #op2 \
                             :                                             \
                             : "r"(__val)                                  \
                             : clobbers);                                  \
    })

/**
 * get cp15
 */
#define _MRC_P15(op1, crn, crm, op2, clobbers)                              \
    ({                                                                     \
        register u32 __val;                                           \
        __asm__ __volatile__("mrc p15," #op1 ",%0," #crn "," #crm "," #op2 \
                             : "=r"(__val)                                 \
                             :                                             \
                             : clobbers);                                  \
        __val;                                                             \
    })

/** System Control Register */
#define GET_SCTLR()         _MRC_P15(0, c1, c0, 0, "cc") /* "cc"表示使用的汇编代码可能会改变符号标志位寄存器, 告知编译器可能有副作用 */
#define SET_SCTLR(val)      _MCR_P15(0, c1, c0, 0, "cc", val)
#define SCTLR_M             (1U << 0)  /* MMU enabled */
#define SCTLR_C             (1U << 2)  /* Data caching enabled */
#define SCTLR_I             (1U << 12) /* instruction caching enabled */

/** Translation Table Base Register */
#define GET_TTBR0()         _MRC_P15(0, c2, c0, 0, )
#define SET_TTBR0(val)      _MCR_P15(0, c2, c0, 0, , val)

#define GET_TTBR1()         _MRC_P15(0, c2, c0, 1, )
#define SET_TTBR1(val)      _MCR_P15(0, c2, c0, 1, , val)

#define SET_ITLBIALL()       _MCR_P15(0, c8, c7, 0, , 0)
// #define SET_ITLBIASID(val)     _MCR_P15(0, c8, c7, 0, , val)

#define SET_BPIALL()        _MCR_P15(0, c7, c5, 6, , 0)

#define GET_CONTEXTIDR()    _MRC_P15(0, c13, c0, 1)
#define SET_CONTEXTIDR(val)    _MCR_P15(0, c13, c0, 1, , val)

#endif
