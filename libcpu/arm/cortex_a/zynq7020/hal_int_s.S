//hal层中断相关汇编文件
.set SYS_MODE,  0x1F
.set SVC_MODE,	0x13
.set IRQ_MODE,	0x12
.set ICCIAR,	0xF8F0010C
.set ICCEOIR,	0xF8F00110

.global HAL_START_OS
.global HAL_CORE_SWI_ENTRY
.global HAL_FOLLOW_SWI_ENTRY
.global HAL_CORE_IRQ_ENTRY
.global HAL_FOLLOW_IRQ_ENTRY
.global HAL_INTR_DISABLE
.global HAL_INTR_ENABLE
.global HAL_INTR_DISABLE_SAVE
.global HAL_INTR_RESTORE
.extern acoral_running_thread
.extern acoral_switch_context
.extern hal_irq_switch

.macro portSAVE_CONTEXT
    /* Save the LR and SPSR onto the system mode stack before switching to
    system mode to save the remaining system mode registers. */
    srsdb   sp!, #SYS_MODE
    cps     #SYS_MODE
    push    {r0-r12, r14}

    bl      HAL_GET_CURRENT_CPU
    mov     r1, #4
    mul     r0, r0, r1
    ldr     r1, =acoral_running_thread
    add     r1, r1, r0
    ldr     r0, [r1]
    add     r0, r0, #4
    str     sp, [r0]
    .endm


.macro portRESTORE_CONTEXT
    bl      HAL_GET_CURRENT_CPU
    mov     r1, #4
    mul     r0, r0, r1
    ldr     r1, =acoral_running_thread
    add     r1, r1, r0
    ldr     r0, [r1]
    add     r0, r0, #4
    ldr     sp, [r0]

    pop     {r0-r12, r14}
    /* Return to the task code, loading CPSR on the way. */
    rfeia   sp!
    .endm

HAL_START_OS:
    /* Switch to system mode. */
    CPS     #SYS_MODE
    portRESTORE_CONTEXT

HAL_CORE_SWI_ENTRY:
    portSAVE_CONTEXT

    bl      acoral_switch_context

    portRESTORE_CONTEXT

HAL_FOLLOW_SWI_ENTRY:
    portSAVE_CONTEXT

    bl      acoral_switch_context

    portRESTORE_CONTEXT

HAL_CORE_IRQ_ENTRY:
    /* Return to the interrupted instruction. */
    sub     lr, lr, #4

    /* Push the return address and SPSR. */
    push    {lr}
    mrs     lr, spsr
    push    {lr}

    /* Change to supervisor mode to allow reentry. */
    cps     #SVC_MODE

    /* Push used registers. */
    push    {r0-r4, r12}

    /* Read value from the interrupt acknowledge register, which is stored in r0
    for future parameter and interrupt clearing use. */
    ldr     r2, =ICCIAR
    ldr     r0, [r2]

    push    {r0-r4, lr}
    bl      hal_all_entry           /* IRQ vector */
    pop     {r0-r4, lr}

    /* Write the value read from ICCIAR to ICCEOIR. */
    ldr     r4, =ICCEOIR
    str     r0, [r4]

    bl      HAL_GET_CURRENT_CPU
    mov     r1, #4
    mul     r0, r0, r1
    ldr     r1, =hal_irq_switch
    add     r1, r1, r0
    ldr     r0, [r1]
    cmp     r0, #0
    bne     switch_before_exit0

exit_without_switch0:
    /* No context switch.  Restore used registers, LR_irq and SPSR before
    returning. */
    pop     {r0-r4, r12}
    cps     #IRQ_MODE
    pop     {lr}
    msr     spsr_cxsf, lr
    pop     {lr}
    movs    pc, lr

switch_before_exit0:
    pop     {r0-r4, r12}
    cps     #IRQ_MODE
    pop     {lr}
    msr     spsr_cxsf, lr
    pop     {lr}

    portSAVE_CONTEXT

    bl      acoral_switch_context

    portRESTORE_CONTEXT

HAL_FOLLOW_IRQ_ENTRY:
    /* Return to the interrupted instruction. */
    sub     lr, lr, #4

    /* Push the return address and SPSR. */
    push    {lr}
    mrs     lr, spsr
    push    {lr}

    /* Change to supervisor mode to allow reentry. */
    cps     #SVC_MODE

    /* Push used registers. */
    push    {r0-r4, r12}

    /* Read value from the interrupt acknowledge register, which is stored in r0
    for future parameter and interrupt clearing use. */
    ldr     r2, =ICCIAR
    ldr     r0, [r2]

    push    {r0-r4, lr}
    bl      hal_all_entry           /* IRQ vector */
    pop     {r0-r4, lr}

    /* Write the value read from ICCIAR to ICCEOIR. */
    ldr     r4, =ICCEOIR
    str     r0, [r4]

    bl      HAL_GET_CURRENT_CPU
    mov     r1, #4
    mul     r0, r0, r1
    ldr     r1, =hal_irq_switch
    add     r1, r1, r0
    ldr     r0, [r1]
    cmp     r0, #0
    bne     switch_before_exit0

exit_without_switch1:
    /* No context switch.  Restore used registers, LR_irq and SPSR before
    returning. */
    pop     {r0-r4, r12}
    cps     #IRQ_MODE
    pop     {lr}
    msr     spsr_cxsf, lr
    pop     {lr}
    movs    pc, lr

switch_before_exit1:
    pop     {r0-r4, r12}
    cps     #IRQ_MODE
    pop     {lr}
    msr     spsr_cxsf, lr
    pop     {lr}

    portSAVE_CONTEXT

    bl      acoral_switch_context

    portRESTORE_CONTEXT

HAL_INTR_ENABLE:
    cpsie   i
    dsb
    isb
    mov pc, lr

HAL_INTR_DISABLE:
    cpsid   i
    dsb
    isb
    mov pc, lr




