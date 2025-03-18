/* asm_vectors.S modified by BowenHu */
#include "xil_errata.h"
#include "bspconfig.h"

.org 0
.text

.globl _vector_table
.globl _vector_table_1
.section .vectors
_vector_table:
    B   _boot
    B   Undefined
    B   HAL_CORE_SWI_ENTRY
    B   PrefetchAbortHandler
    B   DataAbortHandler
    NOP /* Placeholder for address exception vector*/
    B   HAL_CORE_IRQ_ENTRY
    B   FIQHandler
_vector_table_1:
    B   .
    B   .
    B   HAL_FOLLOW_SWI_ENTRY
    B   .
    B   .
    NOP /* Placeholder for address exception vector*/
    B   HAL_FOLLOW_IRQ_ENTRY
    B   .

IRQHandler:                 /* IRQ vector handler */

    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code*/
#if FPU_HARD_FLOAT_ABI_ENABLED
    vpush {d0-d7}
    vpush {d16-d31}
    vmrs r1, FPSCR
    push {r1}
    vmrs r1, FPEXC
    push {r1}
#endif

#ifdef PROFILING
    ldr r2, =prof_pc
    subs    r3, lr, #0
    str r3, [r2]
#endif

    bl  IRQInterrupt            /* IRQ vector */

#if FPU_HARD_FLOAT_ABI_ENABLED
    pop     {r1}
    vmsr    FPEXC, r1
    pop     {r1}
    vmsr    FPSCR, r1
    vpop    {d16-d31}
    vpop    {d0-d7}
#endif
    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */


    subs    pc, lr, #4          /* adjust return */


FIQHandler:                 /* FIQ vector handler */
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */
#if FPU_HARD_FLOAT_ABI_ENABLED
    vpush {d0-d7}
    vpush {d16-d31}
    vmrs r1, FPSCR
    push {r1}
    vmrs r1, FPEXC
    push {r1}
#endif

FIQLoop:
    bl  FIQInterrupt            /* FIQ vector */

#if FPU_HARD_FLOAT_ABI_ENABLED
    pop     {r1}
    vmsr    FPEXC, r1
    pop     {r1}
    vmsr    FPSCR, r1
    vpop    {d16-d31}
    vpop    {d0-d7}
#endif
    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */
    subs    pc, lr, #4          /* adjust return */


Undefined:                  /* Undefined handler */
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */
    ldr     r0, =UndefinedExceptionAddr
    sub     r1, lr, #4
    str     r1, [r0]                    /* Store address of instruction causing undefined exception */

    bl  UndefinedException      /* UndefinedException: call C function here */
    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */
    movs    pc, lr

SVCHandler:                 /* SWI handler */
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */

    tst r0, #0x20           /* check the T bit */
    ldrneh  r0, [lr,#-2]            /* Thumb mode */
    bicne   r0, r0, #0xff00         /* Thumb mode */
    ldreq   r0, [lr,#-4]            /* ARM mode */
    biceq   r0, r0, #0xff000000     /* ARM mode */

    bl  SWInterrupt         /* SWInterrupt: call C function here */

    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */

    movs    pc, lr      /*return to the next instruction after the SWI instruction */


DataAbortHandler:               /* Data Abort handler */
#ifdef CONFIG_ARM_ERRATA_775420
    dsb
#endif
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */
    ldr     r0, =DataAbortAddr
    sub     r1, lr, #8
    str     r1, [r0]                    /* Stores instruction causing data abort */

    bl  DataAbortInterrupt      /*DataAbortInterrupt :call C function here */

    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */

    subs    pc, lr, #8          /* points to the instruction that caused the Data Abort exception */

PrefetchAbortHandler:               /* Prefetch Abort handler */
#ifdef CONFIG_ARM_ERRATA_775420
    dsb
#endif
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */
    ldr     r0, =PrefetchAbortAddr
    sub     r1, lr, #4
    str     r1, [r0]                    /* Stores instruction causing prefetch abort */

    bl  PrefetchAbortInterrupt      /* PrefetchAbortInterrupt: call C function here */

    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */

    subs    pc, lr, #4          /* points to the instruction that caused the Prefetch Abort exception */

.end
