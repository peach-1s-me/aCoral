/**
 * @file cmd.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层ash终端命令头文件
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
#ifndef ACORAL_ASH_CMD_H
#define ACORAL_ASH_CMD_H
#include <type.h>
#include <list.h>
/**
 * @brief ash终端命令结构体
 * 
 */
struct acoral_ash_cmd{
    acoral_char *name;///<命令名称
    void (*exe)(int argc, acoral_char **);///<命令执行函数
    acoral_char *comment;///<命令介绍
    acoral_list_t list;///<链表节点
};
///重定义ash终端命令结构体为一个类型
typedef struct acoral_ash_cmd acoral_ash_cmd_t;

void ash_cmd_register(acoral_ash_cmd_t *cmd);
void ash_cmd_init();
void acoral_ash_cmd_exe(acoral_char *buf);
#endif
