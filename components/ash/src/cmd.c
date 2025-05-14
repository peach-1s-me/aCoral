/**
 * @file cmd.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层ash终端命令源文件
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
#include <queue.h>
#include <mem.h>
#include <shell.h>
#include <cmd.h>
#include <str.h>
#include <print.h>
#include "ff.h"
///ash终端命令队列
acoral_queue_t acoral_ash_cmd_queue;
extern struct ash_shell acoral_shell;
///最大参数数目
#define MAX_ARGS_NUM 8

/**
 * @brief 语法状态
 * 
 */
enum parse_state
{
    PS_WHITESPACE,///<空格
    PS_TOKEN,///<token
    PS_STRING,///<字符串
    PS_ESCAPE///<escape
};
/**
 * @brief 对命令行进行语法分析
 * 
 * @param argstr 命令行字符串指针
 * @param argc_p 参数数目
 * @param argv 参数列表
 * @param resid id
 * @param stacked 语法态栈
 */
void parse_args(acoral_char *argstr, acoral_32 *argc_p, acoral_char **argv, acoral_char** resid,enum parse_state *stacked)
{
    acoral_32 argc = 0;
    acoral_char c;
    enum parse_state newState;
    enum parse_state stacked_state=*stacked;
    enum parse_state lastState = PS_WHITESPACE;

    while ((c = *argstr) != 0&&argc<MAX_ARGS_NUM) {

        if (c == ';' && lastState != PS_STRING && lastState != PS_ESCAPE)
            break;

        if (lastState == PS_ESCAPE) {
            newState = stacked_state;
        } else if (lastState == PS_STRING) {
            if (c == '"') {
                newState = PS_WHITESPACE;
                *argstr = 0;
            } else {
                newState = PS_STRING;
            }
        } else if ((c == ' ') || (c == '\t')) {
            *argstr = 0;
            newState = PS_WHITESPACE;
        } else if (c == '"') {
            newState = PS_STRING;
            *argstr++ = 0;
            argv[argc++] = argstr;
        } else if (c == '\\') {
            stacked_state = lastState;
            newState = PS_ESCAPE;
        } else {
            if (lastState == PS_WHITESPACE) {
                argv[argc++] = argstr;
            }
            newState = PS_TOKEN;
        }

        lastState = newState;
        argstr++;
    }

    argv[argc] = NULL;
    if (argc_p != NULL)
        *argc_p = argc;

    if (*argstr == ';') {
        *argstr++ = '\0';
    }
    *resid = argstr;
    *stacked=stacked_state;
}

/**
 * @brief 命令注册函数
 * 
 * @param cmd 要注册的命令结构体指针
 */
void ash_cmd_register(acoral_ash_cmd_t *cmd)
{
    acoral_list_init(&cmd->list);
    acoral_fifo_queue_add(&acoral_ash_cmd_queue, &cmd->list);
}
/**
 * @brief 按照名称寻找命令结构体指针
 * 
 * @param cmdname 命令名称
 * @return acoral_ash_cmd_t* 命令结构体指针
 */
static acoral_ash_cmd_t *find_cmd(acoral_char *cmdname)
{
    acoral_list_t *tmp,*head;
    acoral_ash_cmd_t *cmd;
    head = &acoral_ash_cmd_queue.head;
    if(acoral_list_empty(head))
        return NULL;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询cmd
    {
        cmd = list_entry(tmp, acoral_ash_cmd_t, list);
        if(acoral_str_cmp(cmd->name, cmdname) == 0)
            return cmd;
    }
    return NULL;
}

/**
 * @brief 实际命令运行函数
 * 
 * @param argc 参数数目
 * @param argv 参数列表
 */
static void real_cmd_exec(acoral_32 argc, acoral_char **argv)
{
    acoral_ash_cmd_t *cmd = find_cmd(argv[0]);
    if(cmd == NULL)//没有找到cmd
    {
        acoral_print("Could not found '%s' command\r\n", argv[0]);
        acoral_print("you can type 'help'\r\n");
        return;
    }
    cmd->exe(argc, argv);//执行cmd
}

/**
 * @brief ash终端命令运行函数
 * 
 * @param buf 命令行字符串指针
 */
void acoral_ash_cmd_exe(acoral_char *buf)
{
    acoral_32 argc;
    acoral_char *argv[MAX_ARGS_NUM];
    acoral_char *resid;
    enum parse_state stacked_state;
    while(*buf)
    {
        acoral_memset(argv, 0, sizeof(argv));//清零
        parse_args(buf, &argc, argv, &resid,&stacked_state);//语法分析
        if(argc > 0)
            real_cmd_exec(argc, argv);//cmd实际执行函数
        buf = resid;//id
    }
}

/**
 * @brief ash终端命令之help
 * 
 * @param argc 参数数目
 * @param argv 参数列表
 */
void help(acoral_32 argc,acoral_char **argv)
{
    acoral_list_t *tmp,*head;
    acoral_ash_cmd_t *cmd;
    head = &acoral_ash_cmd_queue.head;
    if(acoral_list_empty(head))
        return;
    for(tmp=head->next;tmp!=head;tmp=tmp->next)//轮询cmd
    {
        cmd = list_entry(tmp, acoral_ash_cmd_t, list);
        acoral_prints("%s  \t%s\r\n",cmd->name,cmd->comment);
    }
}
/**
 * @brief help命令结构体
 * 
 */
acoral_ash_cmd_t help_cmd =
{
    .name = "help",
    .exe = help,
    .comment = "View all Shell Command info"
};


/***********************************file system cmd******************************/
/**
 * @brief ash终端命令之ls
 * 
 * @param argc 参数数目
 * @param argv 参数列表
 */
void ls_dir(acoral_32 argc,acoral_char **argv)
{
    FRESULT res;
    DIR dir;
    acoral_char path[ASH_CMD_SIZE]  = {0};
    static FILINFO fno;
    acoral_32 pos_last;
    acoral_str_cpy(path, acoral_shell.cur_dir);//拷贝当前目录
    if(argc == 2)//有一个参数
    {
        pos_last = acoral_str_chr(argv[1], '\0') - 2;//找到字符串末尾
        if(argv[1][pos_last] == '/')//fatfs文件系统不能用'/'结尾
            argv[1][pos_last] = '\0';
        acoral_str_cat(path, argv[1]);//添加到当前目录
    }
    res = f_opendir(&dir, path);//目录判断
    if(res == FR_OK)
    {
        for(;;)//轮询打印文件与文件夹名称
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
            {
                acoral_prints("\r\n");
                break;
            }
            acoral_prints("%s   ", fno.fname);
        }
        f_closedir(&dir);
    }
}
/**
 * @brief ls命令结构体
 * 
 */
acoral_ash_cmd_t ls_cmd =
{
    .name = "ls",
    .exe = ls_dir,
    .comment = "List file/dir in current dir"
};
/**
 * @brief ash终端命令之cd
 * 
 * @param argc 参数数目
 * @param argv 参数列表
 */
void cd_dir(acoral_32 argc,acoral_char **argv)
{
    FRESULT res;
    DIR dir;
    acoral_char path[ASH_CMD_SIZE]  = {0};
    acoral_char *tmp = NULL;
    acoral_32 pos_1,pos_2,pos_last,i;
    if(argc == 2)//有一个参数
    {
        if(acoral_str_cmp(acoral_shell.cur_dir, "/") != 0)//当前为根目录的话，先不拷贝
            acoral_str_cpy(path, acoral_shell.cur_dir);
        pos_last = acoral_str_chr(argv[1], '\0') - 2;//找到字符串末尾
        if(argv[1][pos_last] == '/')//fatfs文件系统不能用'/'结尾
            argv[1][pos_last] = '\0';
        do//轮询检测文件夹
        {
            if(argv[1][0] == '/')//'/'开头的目录参数先去掉
                acoral_str_cpy(argv[1], &argv[1][1]);
            pos_1 = acoral_str_chr(argv[1], '/') - 1;//获取子目录位置
            if(pos_1 >= 0)//存在子目录
            {
                tmp = acoral_vol_malloc(pos_1);
                acoral_str_ncpy(tmp, argv[1], pos_1);//拷贝子目录
                if(acoral_str_cmp(tmp, "..") == 0)//".."
                {
                    pos_2 = acoral_str_rchr(path, '/') - 1;//找到最后一个'/'
                    pos_last = acoral_str_chr(path, '\0') - 2;
                    if(pos_2 >= 0)//有找到'/'
                    {
                        for(i=pos_2;i<=pos_last;i++)
                            path[i] = '\0';
                    }
                }
                else if(acoral_str_cmp(tmp, ".") != 0)//除了".."和"."的其他子目录名称
                {
                    acoral_str_cat(path, "/");
                    acoral_str_cat(path, tmp);
                }
                res = f_opendir(&dir, path);
                if(res == FR_OK)//目录判断
                {
                    acoral_str_cpy(argv[1], &argv[1][pos_1]);//目录往下一级走
                    acoral_memset(tmp, 0, pos_1);//重置字符串暂存区
                    acoral_vol_free(tmp);
                    tmp = NULL;
                    f_closedir(&dir);
                }
                else
                {
                    acoral_prints("Wrong directory!\r\n");
                }
            }
            else//不存在子目录
            {
                if(acoral_str_cmp(argv[1], "..") == 0)//".."
                {
                    pos_2 = acoral_str_rchr(path, '/') - 1;//找到最后一个'/'
                    pos_last = acoral_str_chr(path, '\0') - 2;
                    if(pos_2 >= 0)//有找到'/'
                    {
                        for(i=pos_2;i<=pos_last;i++)
                            path[i] = '\0';
                    }
                    if(path[0] == '\0')
                        path[0] = '/';
                }
                else if(acoral_str_cmp(argv[1], ".") == 0)//除了"."
                {
                    if(path[0] == '\0')
                        path[0] = '/';
                }
                else//其他子目录名称
                {
                    acoral_str_cat(path, "/");
                    acoral_str_cat(path, argv[1]);
                }
                res = f_opendir(&dir, path);
                if(res == FR_OK)//目录判断
                {
                    if(acoral_str_cmp(acoral_shell.cur_dir, path) != 0)//改变当前目录
                    {
                        acoral_str_cpy(acoral_shell.cur_dir, path);
                        acoral_shell.dir_change = 1;
                    }
                    f_closedir(&dir);
                }
                else
                {
                    acoral_prints("Wrong directory!\r\n");
                }
                return;
            }
        }while(1);
    }
    else if(argc == 1)//没有目录参数
    {
        acoral_str_cpy(path, "/");
        if(acoral_str_cmp(path, acoral_shell.cur_dir) != 0)
        {
            acoral_str_cpy(acoral_shell.cur_dir, path);
            acoral_shell.dir_change = 1;
        }
    }
}
/**
 * @brief cd命令结构体
 * 
 */
acoral_ash_cmd_t cd_cmd =
{
    .name = "cd",
    .exe = cd_dir,
    .comment = "Change current dir"
};
/***********************************file system cmd******************************/

extern acoral_ash_cmd_t test_tlsf_cmd;

/**
 * @brief ash终端命令初始化
 * 
 */
void ash_cmd_init()
{
    acoral_fifo_queue_init(&acoral_ash_cmd_queue);//命令队列初始化
    ash_cmd_register(&ls_cmd);
    ash_cmd_register(&cd_cmd);
    ash_cmd_register(&help_cmd);
    ash_cmd_register(&test_tlsf_cmd);
}
