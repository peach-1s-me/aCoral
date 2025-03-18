/**
 * @file shell.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层ash终端shell源文件
 * @version 1.2
 * @date 2023-09-09
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-07 <td>增加注释
 *         <tr><td>v1.1 <td>胡博文 <td>2022-11-11 <td>改为中断方式
 *         <tr><td>v1.2 <td>胡博文 <td>2023-09-09 <td>减少中断运行内容
 */
#include <acoral.h>
#include <ff.h>
#include <shell.h>
#include <cmd.h>
#include "xuartps.h"

///ash终端shell结构体实例
struct ash_shell acoral_shell;

///ash终端shell字符输入缓存数组
static acoral_u8 shell_input[ASH_CMD_SIZE] = {0};
///ash终端shell字符输入缓存获取位置
static int get_pos = 0;
///ash终端shell字符输入缓存输入位置
static int input_pos = 0;
///ash终端shell串口中断信号量
acoral_ipc_t shell_sem;

///ash终端shell串口结构体
XUartPs UartPs;

///ash终端shell串口设备ID
#define UART_DEVICE_ID     XPAR_PS7_UART_0_DEVICE_ID
///ash终端shell串口中断ID
#define UART_INT_ID    XPAR_XUARTPS_0_INTR

///ash终端shell线程栈大小
#define SHELL_STACK_SIZE 1024



/**
 * @brief 获取命令提示符
 * 
 * @return const acoral_char* 命令提示符字符串
 */
const acoral_char *ash_get_prompt()
{
    static acoral_char ash_prompt[ASH_PROMPT_SIZE] = {0};
    if(acoral_shell.dir_change == 1)
    {
        acoral_memset(ash_prompt, 0, ASH_PROMPT_SIZE);
        acoral_str_cat(ash_prompt, "aCoral:");
        acoral_str_cat(ash_prompt, acoral_shell.cur_dir);
        acoral_str_cat(ash_prompt, "$");
        acoral_shell.dir_change = 0;
    }
    return ash_prompt;
}
/**
 * @brief 打印命令历史
 * 
 * @param shell shell结构体指针
 * @return acoral_bool 错误检测
 */
static acoral_bool shell_handle_history(struct ash_shell *shell)
{
    acoral_prints("\033[2K\r");
    acoral_prints("%s%s", ASH_PROMPT, shell->line);
    return FALSE;
}

/**
 * @brief 存入命令历史
 * 
 * @param shell shell结构体指针
 */
static void shell_push_history(struct ash_shell *shell)
{
    if (shell->line_position != 0)//存在命令行
    {
        if (shell->history_count >= ASH_HISTORY_LINES)//当前历史数目将要超过最大命令历史数目
        {
            //如果命令和上一个历史相同，那么不需要改变，否则进行移动
            if (acoral_memcmp(&shell->cmd_history[ASH_HISTORY_LINES - 1], shell->line, ASH_CMD_SIZE))
            {
                int index;
                for (index = 0; index < ASH_HISTORY_LINES - 1; index++)
                {
                    acoral_memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0], ASH_CMD_SIZE);
                }
                acoral_memset(&shell->cmd_history[index][0], 0, ASH_CMD_SIZE);
                acoral_memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                shell->history_count = ASH_HISTORY_LINES;
            }
        }
        else
        {
            //如果命令和上一个历史相同，那么不需要改变，否则进行移动
            if (shell->history_count == 0 || acoral_memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, ASH_CMD_SIZE))
            {
                shell->current_history = shell->history_count;
                acoral_memset(&shell->cmd_history[shell->history_count][0], 0, ASH_CMD_SIZE);
                acoral_memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                shell->history_count++;
            }
        }
    }
    shell->current_history = shell->history_count;
}

/**
 * @brief 运行命令函数
 * 
 * @param shell shell结构体指针
 */
void ash_cmd_process(struct ash_shell *shell)
{
    acoral_u8 recv_number;
    acoral_char ch = 0;
    while(1)
    {
        acoral_sem_pend(&shell_sem);
        //获取缓冲区所有内容
        while((shell_input[input_pos]==0)&&((recv_number = XUartPs_Recv(&UartPs, &shell_input[input_pos], 1))!=0))
        {
            input_pos++;
            if(input_pos==ASH_CMD_SIZE)
                input_pos = 0;
        }
        //开始解析
        while((ch = (acoral_char)shell_input[get_pos]) != 0)
        {
            shell_input[get_pos] = 0;
            get_pos++;
            if(get_pos==ASH_CMD_SIZE)
                get_pos = 0;
            //上下左右箭头按键
            if (ch == 0x1b)
            {
                shell->stat = WAIT_SPEC_KEY;
                continue;
            }
            else if (shell->stat == WAIT_SPEC_KEY)
            {
                if (ch == 0x5b)
                {
                    shell->stat = WAIT_FUNC_KEY;
                    continue;
                }

                shell->stat = WAIT_NORMAL;
            }
            else if (shell->stat == WAIT_FUNC_KEY)
            {
                shell->stat = WAIT_NORMAL;

                if (ch == 0x41)//up key
                {
                    //prev history
                    if (shell->current_history > 0)
                        shell->current_history --;
                    else
                    {
                        shell->current_history = 0;
                        continue;
                    }

                    //copy the history command
                    acoral_memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                        ASH_CMD_SIZE);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
                    continue;
                }
                else if (ch == 0x42)//down key
                {
                    //next history
                    if (shell->current_history < shell->history_count - 1)
                        shell->current_history ++;
                    else
                    {
                        //set to the end of history
                        if (shell->history_count != 0)
                            shell->current_history = shell->history_count - 1;
                        else
                            continue;
                    }

                    acoral_memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                        ASH_CMD_SIZE);
                    shell->line_curpos = shell->line_position = strlen(shell->line);
                    shell_handle_history(shell);
                    continue;
                }
                else if (ch == 0x44)//left key
                {
                    if (shell->line_curpos)
                    {
                        acoral_prints("\b");
                        shell->line_curpos --;
                    }

                    continue;
                }
                else if (ch == 0x43)//right key
                {
                    if (shell->line_curpos < shell->line_position)
                    {
                        acoral_prints("%c", shell->line[shell->line_curpos]);
                        shell->line_curpos++;
                    }

                    continue;
                }
            }
            //字符检测
            switch(ch)
            {
                case 0x08:
                case 0x7F://backspace
                    if(shell->line_curpos)
                    {
                        shell->line_position--;
                        shell->line_curpos--;
                        if(shell->line_position > shell->line_curpos)
                        {
                            acoral_memmove(&shell->line[shell->line_curpos],
                                    &shell->line[shell->line_curpos + 1],
                                    shell->line_position - shell->line_curpos);
                            shell->line[shell->line_position] = 0;

                            acoral_prints("\b%s  \b", &shell->line[shell->line_curpos]);

                            for (int i = shell->line_curpos; i <= shell->line_position; i++)//移动光标
                                acoral_prints("\b");
                        }
                        else
                        {
                            acoral_prints("\b \b");
                            shell->line[shell->line_position] = 0;
                        }
                    }
                    break;
                case '\r':
                case '\n'://回车
                    shell_push_history(shell);
                    acoral_prints("\r\n");
                    acoral_ash_cmd_exe(shell->line);
                    acoral_memset(shell->line, 0, sizeof(shell->line));
                    shell->line_curpos = 0;
                    shell->line_position = 0;
                    return;
                default://正常输入
                    if(shell->line_position < ASH_CMD_SIZE)//没超出范围
                    {
                        if(shell->line_curpos < shell->line_position)//光标不在末尾
                        {
                            acoral_memmove(&shell->line[shell->line_curpos + 1],
                                    &shell->line[shell->line_curpos],
                                    shell->line_position - shell->line_curpos);
                            shell->line[shell->line_curpos] = ch;
                            shell->line[shell->line_position + 1] = 0;

                            acoral_prints("%s", &shell->line[shell->line_curpos]);

                            for (int i = shell->line_curpos; i < shell->line_position; i++)//移动光标
                                acoral_prints("\b");
                        }
                        else//光标在末尾
                        {
                            shell->line[shell->line_curpos] = ch;
                            if(shell->echo_mode)
                                outbyte(ch);
                        }
                        shell->line_position++;
                        shell->line_curpos++;
                    }
                    else//超出范围，重新来
                    {
                        acoral_prints("\r\nbeyond cmd size, please try again\r\n");
                        shell->line_position = 0;
                        shell->line_curpos = 0;
                        return;
                    }
                    break;
            }
        }
    }
}

/**
 * @brief shell任务函数
 * 
 * @param args 回调参数
 */
void ash_shell_thread(void *args)
{
    while(1)
    {
        acoral_prints(ASH_PROMPT);
        ash_cmd_process(&acoral_shell);
    }
}



void shell_uart_intr_handler(void *call_back_ref)
{
    XUartPs *uart_instance_ptr = (XUartPs *) call_back_ref;
    u32 isr_status ;                           //中断状态标志
    //读取中断ID寄存器，判断触发的是哪种中断
    isr_status = XUartPs_ReadReg(uart_instance_ptr->Config.BaseAddress,
                   XUARTPS_IMR_OFFSET);
    isr_status &= XUartPs_ReadReg(uart_instance_ptr->Config.BaseAddress,
                   XUARTPS_ISR_OFFSET);
    //判断中断标志位RxFIFO是否触发
    if (isr_status & (u32)XUARTPS_IXR_RXOVR)
    {
        acoral_sem_post(&shell_sem);
    }
    //清除中断标志
    XUartPs_WriteReg(uart_instance_ptr->Config.BaseAddress,
                        XUARTPS_ISR_OFFSET, isr_status) ;
}

//UART初始化函数
static int shell_uart_init()
{
    int status;
    XUartPs_Config *uart_cfg;

    uart_cfg = XUartPs_LookupConfig(UART_DEVICE_ID);
    if (NULL == uart_cfg)
        return XST_FAILURE;
    status = XUartPs_CfgInitialize(&UartPs, uart_cfg, uart_cfg->BaseAddress);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    //设置工作模式:正常模式
    XUartPs_SetOperMode(&UartPs, XUARTPS_OPER_MODE_NORMAL);
    //设置波特率:115200
    XUartPs_SetBaudRate(&UartPs,115200);
    //设置RxFIFO的中断触发等级
    XUartPs_SetFifoThreshold(&UartPs, 1);

    //为中断设置中断处理函数
    XScuGic_Connect(&int_ctrl[0], UART_INT_ID,
            (Xil_ExceptionHandler) shell_uart_intr_handler,(void *) &UartPs);
    //设置UART的中断触发方式
    XUartPs_SetInterruptMask(&UartPs, XUARTPS_IXR_RXOVR);
    //使能GIC中的串口中断
    XScuGic_Enable(&int_ctrl[0], UART_INT_ID);

    return XST_SUCCESS;
}


/**
 * @brief ash中断shell初始化
 * 
 */
void acoral_ash_shell_init()
{
#ifdef CFG_SHELL
    acoral_comm_policy_data_t p_data;
#endif
    ash_cmd_init();
    shell_uart_init();
    acoral_sem_init(&shell_sem, 0);
    acoral_shell.echo_mode = 1;
    acoral_shell.current_history = 0;
    acoral_shell.history_count = 0;
    acoral_shell.line_curpos = 0;
    acoral_shell.line_position = 0;
    acoral_shell.cur_dir = "/";//初始目录为根目录
    acoral_shell.dir_change = 1;//目录改变标志先置1
    acoral_memset(acoral_shell.cmd_history, 0, sizeof(acoral_shell.cmd_history));
    acoral_memset(acoral_shell.line, 0, sizeof(acoral_shell.line));
#ifdef CFG_SHELL
    p_data.cpu=0;
    p_data.prio=ACORAL_SHELL_PRIO;
    acoral_create_thread(ash_shell_thread,SHELL_STACK_SIZE,(void *)acoral_cur_thread->console_id,"shell",NULL,ACORAL_SCHED_POLICY_COMM,&p_data,NULL,NULL);
#endif
}
