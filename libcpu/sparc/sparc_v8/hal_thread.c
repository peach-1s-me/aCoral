/**
 * @file hal_thread.c
 * @author 文佳源，绕洪江
 * @brief hal层通用源文件，通用即与硬件细节无关
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

#include <type.h>

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
    acoral_u32 *stack_addr;
    int window_index;
    int register_index;

    *stk += sizeof(acoral_u32);
    *stk  = (acoral_u8 *)ACORAL_ALIGN_DOWN((acoral_u32)*stk, 8);
    stack_addr  = (acoral_u32 *)*stk;

    stack_addr -= 24;
    stack_addr -= 8;

    for (register_index = 0; register_index != 8; register_index++)
        stack_addr[register_index] = 0xdeadbeef;

    for (window_index = 0; window_index != 8; window_index++)
    {
        stack_addr -= 16;
        for (register_index = 0; register_index != 16; register_index++)
        {
            /* stk[register_index] = 0xdeadbeef; by Rao */
            stack_addr[register_index] = 0xdeadbeef;
        }
        if (window_index == 0)
        {
            stack_addr[8] = (acoral_u32)args;
            stack_addr[15] = (acoral_u32)exit - 8;
        }
    }

    stack_addr -= 66;
    for (register_index = 0; register_index != 66; register_index++)
    {
        stack_addr[register_index] = 0;
    }

    stack_addr -= 4;
    stack_addr[0] = (acoral_u32)route; //pc
    stack_addr[1] = (acoral_u32)route + 4; //npc
    stack_addr[2] = 0x10C7; //psr
    stack_addr[3] = 0x2; //wim

    *stk = stack_addr;
}
