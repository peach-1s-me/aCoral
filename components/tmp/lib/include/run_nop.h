#ifndef LIB_RUN_NOP_H
#define LIB_RUN_NOP_H
#include <type.h>

#define CLOCK_CICLES_PER_MS                             3450        // 通过系统Tick测试出来的 使用该函数 每一秒会比系统tick实际少26ms 比如 __TIMES__为1000时, 系统Tick实际为1026ms
#define _nop() __asm__ __volatile__ ("nop"::)

#define run_nop_ms(__TIMES__) \
    do{                                                                         \
                acoral_u32 cicles = CLOCK_CICLES_PER_MS * (__TIMES__);          \
                while(cicles > 0) {                                             \
                    _nop();                                                     \
                    cicles --;                                                  \
                }                                                               \
    } while(0)                                                                  \

#endif
