//hal层对称多处理器相关汇编文件

//链接文件中的栈地址
.set Undef_stack,   sec__undef_stack
.set FIQ_stack,     sec__fiq_stack
.set Abort_stack,   sec__abort_stack
.set SPV_stack,     sec__supervisor_stack
.set IRQ_stack,     sec__irq_stack
.set SYS_stack,     sec__stack

//写入次核启动地址的地址
.set follow_cpu_entry,  0xfffffff0
//次核中断向量表基址
.set vector_base,       _vector_table_1
//引出的函数以及变量
.global HAL_SET_FOLLOW_CPU_ENTRY
.global sec__irq_stack
.global sec__supervisor_stack
.global sec__abort_stack
.global sec__fiq_stack
.global sec__undef_stack

invalidate_dcache:
    mrc p15, 1, r0, c0, c0, 1  /* read CLIDR */
    ands r3, r0, #0x7000000
    mov r3, r3, lsr #23   /* cache level value (naturally aligned) */
    beq finished
    mov r10, #0    /* start with level 0 */
loop1:
    add r2, r10, r10, lsr #1  /* work out 3xcachelevel */
    mov r1, r0, lsr r2   /* bottom 3 bits are the Cache type for this level */
    and r1, r1, #7   /* get those 3 bits alone */
    cmp r1, #2
    blt skip    /* no cache or only instruction cache at this level */
    mcr p15, 2, r10, c0, c0, 0  /* write the Cache Size selection register */
    isb     /* isb to sync the change to the CacheSizeID reg */
    mrc p15, 1, r1, c0, c0, 0  /* reads current Cache Size ID register */
    and r2, r1, #7   /* extract the line length field */
    add r2, r2, #4   /* add 4 for the line length offset (log2 16 bytes) */
    ldr r4, =0x3ff
    ands r4, r4, r1, lsr #3  /* r4 is the max number on the way size (right aligned) */
    clz r5, r4    /* r5 is the bit position of the way size increment */
    ldr r7, =0x7fff
    ands r7, r7, r1, lsr #13  /* r7 is the max number of the index size (right aligned) */
loop2:
    mov r9, r4    /* r9 working copy of the max way size (right aligned) */
loop3:
    orr r11, r10, r9, lsl r5  /* factor in the way number and cache number into r11 */
    orr r11, r11, r7, lsl r2  /* factor in the index number */
    mcr p15, 0, r11, c7, c6, 2  /* invalidate by set/way */
    subs r9, r9, #1   /* decrement the way number */
    bge loop3
    subs r7, r7, #1   /* decrement the index */
    bge loop2
skip:
    add r10, r10, #2   /* increment the cache number */
    cmp r3, r10
    bgt loop1

finished:
    mov r10, #0    /* swith back to cache level 0 */
    mcr p15, 2, r10, c0, c0, 0  /* select current cache level in cssr */
    dsb
    isb
    bx lr






HAL_FOLLOW_CPU_START:
    /* set VBAR to the _vector_table address in linker script */
    ldr r0, =vector_base
    mcr p15, 0, r0, c12, c0, 0

    /* Invalidate caches and TLBs */
    mov r0, #0    /* r0 = 0  */
    mcr p15, 0, r0, c8, c7, 0  /* invalidate TLBs */
    mcr p15, 0, r0, c7, c5, 0  /* invalidate icache */
    mcr p15, 0, r0, c7, c5, 6  /* Invalidate branch predictor array */
    bl  invalidate_dcache  /* invalidate dcache */

    /* Disable MMU, if enabled */
    mrc p15, 0, r0, c1, c0, 0  /* read CP15 register 1 */
    bic r0, r0, #0x1   /* clear bit 0 */
    mcr p15, 0, r0, c1, c0, 0  /* write value back */

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the irq stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x12   /* IRQ mode */
    msr cpsr, r2
    ldr r13, =IRQ_stack   /* IRQ stack pointer */
    bic r2, r2, #(0x1 << 9)       /* Set EE bit to little-endian */
    msr spsr_fsxc, r2

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the supervisor stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x13   /* supervisor mode */
    msr cpsr, r2
    ldr r13, =SPV_stack   /* Supervisor stack pointer */
    bic r2, r2, #(0x1 << 9)       /* Set EE bit to little-endian */
    msr spsr_fsxc, r2

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the Abort  stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x17   /* Abort mode */
    msr cpsr, r2
    ldr r13, =Abort_stack  /* Abort stack pointer */
    bic r2, r2, #(0x1 << 9)       /* Set EE bit to little-endian */
    msr spsr_fsxc, r2

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the FIQ stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x11   /* FIQ mode */
    msr cpsr, r2
    ldr r13, =FIQ_stack   /* FIQ stack pointer */
    bic r2, r2, #(0x1 << 9)      /* Set EE bit to little-endian */
    msr spsr_fsxc, r2

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the Undefine stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x1b   /* Undefine mode */
    msr cpsr, r2
    ldr r13, =Undef_stack  /* Undefine stack pointer */
    bic r2, r2, #(0x1 << 9)       /* Set EE bit to little-endian */
    msr spsr_fsxc, r2

    mrs r0, cpsr   /* get the current PSR */
    mvn r1, #0x1f   /* set up the system stack pointer */
    and r2, r1, r0
    orr r2, r2, #0x1F   /* SYS mode */
    msr cpsr, r2
    ldr r13, =SYS_stack   /* SYS stack pointer */

    /*set scu enable bit in scu*/
    ldr r7, =0xf8f00000
    ldr r0, [r7]
    orr r0, r0, #0x1
    str r0, [r7]

    mov r0, r0
    mrc p15, 0, r1, c1, c0, 2       /* read cp access control register (CACR) into r1 */
    orr r1, r1, #(0xf << 20)        /* enable full access for p10 & p11 */
    mcr p15, 0, r1, c1, c0, 2       /* write back into CACR */

    mrc p15,0,r0,c1,c0,0        /* flow prediction enable */
    orr r0, r0, #(0x01 << 11)       /* #0x8000 */
    mcr p15,0,r0,c1,c0,0

    mrc p15,0,r0,c1,c0,1        /* read Auxiliary Control Register */
    orr r0, r0, #(0x1 << 2)     /* enable Dside prefetch */
    orr r0, r0, #(0x1 << 1)     /* enable L2 Prefetch hint */
    mcr p15,0,r0,c1,c0,1        /* write Auxiliary Control Register */

    mrs r0, cpsr   /* get the current PSR */
    bic r0, r0, #0x100   /* enable asynchronous abort exception */
    msr cpsr_xsf, r0

    b   acoral_start
    b   .
HAL_SET_FOLLOW_CPU_ENTRY:
    ldr r1, =follow_cpu_entry
    ldr r2, =HAL_FOLLOW_CPU_START
    str r2, [r1]
    mov pc, lr
