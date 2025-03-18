//hal层通用汇编文件，通用即与硬件细节无关

.global HAL_GET_CURRENT_CPU

HAL_GET_CURRENT_CPU:
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #0xf
    mov pc, lr
