/**
 * @file hal_thread.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief hal层线程相关源文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>增加注释
 */

#include <type.h>

/**
 * @brief 寄存器栈结构体
 * 
 */
typedef struct{
    acoral_u32 r0;///<r0寄存器
    acoral_u32 r1;///<r1寄存器
    acoral_u32 r2;///<r2寄存器
    acoral_u32 r3;///<r3寄存器
    acoral_u32 r4;///<r4寄存器
    acoral_u32 r5;///<r5寄存器
    acoral_u32 r6;///<r6寄存器
    acoral_u32 r7;///<r7寄存器
    acoral_u32 r8;///<r8寄存器
    acoral_u32 r9;///<r9寄存器
    acoral_u32 r10;///<r10寄存器
    acoral_u32 r11;///<r11寄存器
    acoral_u32 r12;///<r12寄存器
    acoral_u32 lr;///<lr寄存器
    acoral_u32 pc;///<pc寄存器
    acoral_u32 cpsr;///<cpsr寄存器
}hal_ctx_t;

/**
 * @brief 线程栈初始化
 * 
 * @param stk 线程栈地址
 * @param route 执行函数
 * @param exit 退出函数
 * @param args 传入参数
 */
void hal_stack_init(acoral_u32 **stk,void (*route)(),void (*exit)(),void *args)
{
    hal_ctx_t *ctx=(hal_ctx_t *)*stk;
    ctx--;
    ctx=(hal_ctx_t *)((acoral_u32 *)ctx+1);
    ctx->r0=(acoral_u32)args;
    ctx->r1=1;
    ctx->r2=2;
    ctx->r3=3;
    ctx->r4=4;
    ctx->r5=5;
    ctx->r6=6;
    ctx->r7=7;
    ctx->r8=8;
    ctx->r9=9;
    ctx->r10=10;
    ctx->r11=11;
    ctx->r12=12;
    ctx->lr=(acoral_u32)exit;
    ctx->pc=(acoral_u32)route;
    ctx->cpsr=0x0000001fL;
    *stk=(acoral_u32 *)ctx;
}
