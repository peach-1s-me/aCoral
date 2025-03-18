/**
 * @file mem.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层内存相关头文件
 * @version 1.0
 * @date 2022-07-08
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-08 <td>增加注释
 */
#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H
#include <config.h>
#ifdef CFG_MEM_BUDDY
///重定义伙伴系统分配内存
#define acoral_malloc(size) buddy_malloc(size)
///重定义伙伴系统回收
#define acoral_free(ptr) buddy_free(ptr)
///重定义伙伴系统内存计算
#define acoral_malloc_size(size) buddy_malloc_size(size)
///重定义伙伴系统初始化
#define acoral_mem_init(start,end) buddy_init(start,end)
///重定义伙伴系统扫描
#define acoral_mem_scan() buddy_scan()
#endif

#ifdef CFG_MEM_VOL
///重定义任意大小内存系统初始化
#define acoral_vol_mem_init() vol_mem_init()
///重定义任意大小内存分配
#define acoral_vol_malloc(size) vol_malloc(size)
///重定义任意大小内存回收
#define acoral_vol_free(p) vol_free(p)
///重定义任意大小内存系统扫描
#define acoral_vol_mem_scan() vol_mem_scan()
#endif

void acoral_mem_sys_init(void);

void buddy_scan(void);
acoral_err buddy_init(acoral_u32 start, acoral_u32 end);
acoral_u32 buddy_malloc_size(acoral_u32 size);
void *buddy_malloc(acoral_u32  size);
void buddy_free(void *p);

void * vol_malloc(acoral_32 size);
void vol_free(void * p);
void vol_mem_init();
void vol_mem_scan(void);

#endif
