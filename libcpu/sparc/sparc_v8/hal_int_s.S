/**
 * @file hal_int_s.c
 * @author 文佳源，绕洪江
 * @brief hal层中断相关汇编文件
 * @version 1.0
 * @date 2025-05-10
 * 
 * @copyright Copyright (c) 2025
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>文佳源，绕洪江 <td>2025-05-10 <td>创建文件
 */

/* #define SPARC_PSR_PIL_MASK 0x00000F00 不知道为啥用不了，只能用set*/
.set SPARC_PSR_PIL_MASK, 0x00000F00
#define SPARC_PSR_ET_MASK 0x00000020
.set NWINDOWS, 8

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


.global _3883_HAL_START_OS
_3883_HAL_START_OS:
    /* Switch to system mode. */
    mov %o0, %o1
	ta 3
	retl
	nop

HAL_INTR_ENABLE:
    rd %psr, %o0
    /* or %o0, SPARC_PSR_PIL_MASK, %o1 */
    or %o0, 0xF00, %o1
    wr %o1, %psr
    nop
    nop
    nop
    retl
    nop

HAL_INTR_RESTORE:
    wr %o0, %psr
    nop
    nop
    nop
    retl
    nop

HAL_INTR_DISABLE:
    /* wr %o0, %psr */
    rd %psr, %o0           ! 读取当前 PSR
    andn %o0, 0xF00, %o1   ! 清除 PIL 位（PIL=0）
    wr %o1, %psr           ! 写回 PSR
    nop
    nop
    nop
    retl
    nop

/*
l0 = psr
l1 = pc
l2 = npc
l3 = tbr
*/
.global _context_switch
_context_switch:
/*
保存当前PC到l1，设置npc为PC+4
保存全局寄存器g4/g5到局部寄存器l4/l5
将窗口无效掩码(WIM)保存到g4
 */
	mov %l2, %l1
	add %l2, 4, %l2
	
	mov %g4, %l4
	mov %g5, %l5
	mov %wim, %g4
	srl %g4, %l0, %g5
	cmp %g5, 1
	bne good_window
	nop

	srl %g4, 1, %g5
	sll %g4, NWINDOWS - 1, %g4
	or %g4, %g5, %g4
	save
	mov %g4, %wim
	nop
	nop
	nop
	
	std %l0, [%sp + 0x00]
	std %l2, [%sp + 0x08]
	std %l4, [%sp + 0x10]
	std %l6, [%sp + 0x18]

	std %i0, [%sp + 0x20]
	std %i2, [%sp + 0x28]
	std %i4, [%sp + 0x30]
	std %i6, [%sp + 0x38]
	
	restore
	nop

good_window:
	and %l3, 0x0FF0, %l3
	srl %l3, 4, %l4
	cmp %l4, 0x82
	bne switch_to
	nop
	
	sub %fp, 0x20, %sp
	std %g0, [%sp + 0x00]
	std %g2, [%sp + 0x08]
	std %g4, [%sp + 0x10]
	std %g6, [%sp + 0x18]
	
	mov %sp, %g3
	mov %l1, %g4
	mov %l2, %g5
	mov %l0, %g6
	mov %wim, %g7
	
	mov %g0, %wim
	nop
	nop
	nop
	set 0xFFFFFFF8, %g1
	and %g1, %g6, %g1
	mov %g1, %psr
	nop
	nop
	nop
	
	mov %g0, %g1
save_window:
	save
	sub %g3, 0x40, %g3
	std %l0, [%g3 + 0x00]
	std %l2, [%g3 + 0x08]
	std %l4, [%g3 + 0x10]
	std %l6, [%g3 + 0x18]
	std %i0, [%g3 + 0x20]
	std %i2, [%g3 + 0x28]
	std %i4, [%g3 + 0x30]
	std %i6, [%g3 + 0x38]
	inc %g1
	cmp %g1, NWINDOWS
	bne save_window
	nop

	sub %g3, 0x108, %g3
/*
	std %f0, [%g3 + 0x00]
	std %f2, [%g3 + 0x08]
	std %f4, [%g3 + 0x10]
	std %f6, [%g3 + 0x18]
	std %f8, [%g3 + 0x20]
	std %f10, [%g3 + 0x28]
	std %f12, [%g3 + 0x30]
	std %f14, [%g3 + 0x38]
	std %f16, [%g3 + 0x40]
	std %f18, [%g3 + 0x48]
	std %f20, [%g3 + 0x50]
	std %f22, [%g3 + 0x58]
	std %f24, [%g3 + 0x60]
	std %f26, [%g3 + 0x68]
	std %f28, [%g3 + 0x70]
	std %f30, [%g3 + 0x78]
*/
    std  %f0,  [%g3 + 0x00]
    st   %f1,  [%g3 + 0x08]
    std  %f2,  [%g3 + 0x10]
    st   %f3,  [%g3 + 0x18]
    std  %f4,  [%g3 + 0x20]
    st   %f5,  [%g3 + 0x28]
    std  %f6,  [%g3 + 0x30]
    st   %f7,  [%g3 + 0x38]
    std  %f8,  [%g3 + 0x40]
    st   %f9,  [%g3 + 0x48]
    std  %f10, [%g3 + 0x50]
    st   %f11, [%g3 + 0x58]
    std  %f12, [%g3 + 0x60]
    st   %f13, [%g3 + 0x68]
    std  %f14, [%g3 + 0x70]
    st   %f15, [%g3 + 0x78]
    std  %f16, [%g3 + 0x80]
    st   %f17, [%g3 + 0x88]
    std  %f18, [%g3 + 0x90]
    st   %f19, [%g3 + 0x98]
    std  %f20, [%g3 + 0xa0]
    st   %f21, [%g3 + 0xa8]
    std  %f22, [%g3 + 0xb0]
    st   %f23, [%g3 + 0xb8]
    std  %f24, [%g3 + 0xc0]
    st   %f25, [%g3 + 0xc8]
    std  %f26, [%g3 + 0xd0]
    st   %f27, [%g3 + 0xd8]
    std  %f28, [%g3 + 0xe0]
    st   %f29, [%g3 + 0xe8]
    std  %f30, [%g3 + 0xf0]
    st   %f31, [%g3 + 0xf8]

	mov %y, %g1
	st %g1, [%g3 + 0x100]
	st %fsr, [%g3 + 0x104]
	
	sub %g3, 0x10, %g3
	std %g4, [%g3 + 0x00]
	std %g6, [%g3 + 0x08]
	
	mov %g6, %psr
	nop
	nop
	nop
	
	st %g3, [%i0]
switch_to:
	mov %g0, %wim
	nop
	nop
	nop
	ld [%i1], %g3

	ldd [%g3 + 0x00], %g4
	ldd [%g3 + 0x08], %g6
	add %g3, 0x10, %g3
	
/*
	ldd [%g3 + 0x00], %f0
	ldd [%g3 + 0x08], %f2
	ldd [%g3 + 0x10], %f4
	ldd [%g3 + 0x18], %f6
	ldd [%g3 + 0x20], %f8
	ldd [%g3 + 0x28], %f10
	ldd [%g3 + 0x30], %f12
	ldd [%g3 + 0x38], %f14
	ldd [%g3 + 0x40], %f16
	ldd [%g3 + 0x48], %f18
	ldd [%g3 + 0x50], %f20
	ldd [%g3 + 0x58], %f22
	ldd [%g3 + 0x60], %f24
	ldd [%g3 + 0x68], %f26
	ldd [%g3 + 0x70], %f28
	ldd [%g3 + 0x78], %f30
*/
    ldd  [%g3 + 0x00], %f0
    ld   [%g3 + 0x08], %f1
    ldd  [%g3 + 0x10], %f2
    ld   [%g3 + 0x18], %f3
    ldd  [%g3 + 0x20], %f4
    ld   [%g3 + 0x28], %f5
    ldd  [%g3 + 0x30], %f6
    ld   [%g3 + 0x38], %f7
    ldd  [%g3 + 0x40], %f8
    ld   [%g3 + 0x48], %f9
    ldd  [%g3 + 0x50], %f10
    ld   [%g3 + 0x58], %f11
    ldd  [%g3 + 0x60], %f12
    ld   [%g3 + 0x68], %f13
    ldd  [%g3 + 0x70], %f14
    ld   [%g3 + 0x78], %f15
    ldd  [%g3 + 0x80], %f16
    ld   [%g3 + 0x88], %f17
    ldd  [%g3 + 0x90], %f18
    ld   [%g3 + 0x98], %f19
    ldd  [%g3 + 0xa0], %f20
    ld   [%g3 + 0xa8], %f21
    ldd  [%g3 + 0xb0], %f22
    ld   [%g3 + 0xb8], %f23
    ldd  [%g3 + 0xc0], %f24
    ld   [%g3 + 0xc8], %f25
    ldd  [%g3 + 0xd0], %f26
    ld   [%g3 + 0xd8], %f27
    ldd  [%g3 + 0xe0], %f28
    ld   [%g3 + 0xe8], %f29
    ldd  [%g3 + 0xf0], %f30
    ld   [%g3 + 0xf8], %f31

	ld [%g3 + 0x100], %g1
	mov %g1, %y
	ld [%g3 + 0x104], %fsr
	add %g3, 0x108, %g3
	
	set NWINDOWS - 1, %g1
	or %g1, %g6, %g1
	mov %g1, %psr
	nop
	nop
	nop
	
	mov %g0, %g1
restore_window:
	restore
	ldd [%g3 + 0x00], %l0
	ldd [%g3 + 0x08], %l2
	ldd [%g3 + 0x10], %l4
	ldd [%g3 + 0x18], %l6 
	ldd [%g3 + 0x20], %i0
	ldd [%g3 + 0x28], %i2
	ldd [%g3 + 0x30], %i4
	ldd [%g3 + 0x38], %i6
	add %g3, 0x40, %g3
	inc %g1
	cmp %g1, NWINDOWS
	bne restore_window
	nop
	
	mov %g6, %psr
	nop
	nop
	nop
	mov %g7, %wim
	nop
	nop
	nop
	
	mov %g4, %l1
	mov %g5, %l2
	mov %g3, %sp
	
	ldd [%sp + 0x00], %g0
	ldd [%sp + 0x08], %g2
	ldd [%sp + 0x10], %g4
	ldd [%sp + 0x18], %g6	
	add %sp, 0x20, %fp
	jmp %l1
	rett %l2

/* 调用软中断2到_context_switch */
.global soc3883_context_switch
soc3883_context_switch:
    ta 2
    retl
    nop
