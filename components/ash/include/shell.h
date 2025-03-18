/**
 * @file shell.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层ash终端shell头文件
 * @version 1.0
 * @date 2022-09-07
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-07 <td>增加注释
 */
#ifndef COMPONENT_ASH_SHELL_H
#define COMPONENT_ASH_SHELL_H
#include <type.h>
///最大命令历史数目
#define ASH_HISTORY_LINES 5
///最大命令长度
#define ASH_CMD_SIZE 80
///最大命令提示符长度
#define ASH_PROMPT_SIZE 81
///重定义命令提示符
#define ASH_PROMPT ash_get_prompt()

/**
 * @brief 输入状态
 * 
 */
enum input_stat
{
    WAIT_NORMAL,///<正常状态
    WAIT_SPEC_KEY,///<SPEC状态
    WAIT_FUNC_KEY,///<FUNC状态
};

/**
 * @brief ash终端shell结构体
 * 
 */
struct ash_shell
{
    enum input_stat stat;///<输入状态，用于上下左右箭头按键

    acoral_u8 echo_mode:1;///<echo模式，1：是；0：否

    acoral_u16 current_history;///<当前历史
    acoral_u16 history_count;///<历史命令数量

    acoral_char cmd_history[ASH_HISTORY_LINES][ASH_CMD_SIZE];///<历史命令保存数组

    acoral_char *cur_dir;///<shell所在当前目录
    acoral_u8 dir_change;///<目录被改变标志，1：已被改变；0：未被改变

    acoral_char line[ASH_CMD_SIZE];///<行缓冲数组
    acoral_u16 line_position;///<行末尾位置
    acoral_u16 line_curpos;///<行光标位置
};


void acoral_ash_shell_init();
#endif
