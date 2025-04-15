/**
 * @file type.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief acoral类型定义头文件
 * @version 1.0
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-06-26 <td>增加注释
 *         <tr><td>v1.1 <td>饶洪江 <td>2025-03-13 <td>增加ACORAL_ALIGN
 */
#include <stddef.h>
#include <stdarg.h>
#ifndef ACORAL_TYPE_H
#define ACORAL_TYPE_H
#ifndef NULL
///空
#define NULL (void *)0
#endif
#ifndef false
///bool类型：false
#define false 0
#endif
#ifndef true
///bool类型：ture
#define true 1
#endif
#ifndef FALSE
///bool类型：FALSE
#define FALSE 0
#endif
#ifndef TRUE
///bool类型：TRUE
#define TRUE 1
#endif

typedef unsigned char acoral_u8;
typedef signed char acoral_8;
typedef char acoral_char;
typedef unsigned char acoral_bool;
// typedef acoral_8 bool;
typedef unsigned short acoral_u16;
typedef signed short acoral_16;
typedef unsigned int acoral_u32;
typedef signed int acoral_32;
typedef unsigned long long acoral_u64;
typedef signed long long acoral_64;
typedef float acoral_fl;
typedef acoral_32 acoral_vector;
typedef acoral_u32 acoral_time;
typedef acoral_u32 acoral_adr;
typedef acoral_u32 acoral_err;
typedef acoral_u32 acoral_size;
typedef acoral_32 acoral_id;
typedef acoral_u32 *acoral_ptr;

///重定义函数指针类型
typedef void (*acoral_func_point)(void *args);

///重定义弱定义符号
#define __weak __attribute__((weak))

#ifndef _INTSIZEOF
///地址对齐
#define _INTSIZEOF(n) \
((sizeof(n)+sizeof(acoral_32)-1)&~(sizeof(acoral_32) - 1) )
#endif
#ifndef __va_list__
///可变长度参数链表
typedef acoral_char * acoral_va_list;
#endif
#ifndef va_start
///获取可变长度参数链表头
#define va_start(ap,v) ( ap = (acoral_va_list)&v + _INTSIZEOF(v) )
#endif
#ifndef va_arg
///获取可变长度参数链表中的参数
#define va_arg(ap,t) \
( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#endif
#ifndef va_end
///获取可变长度参数链表尾
#define va_end(ap) ( ap = (acoral_va_list)0 )
#endif
#ifndef offsetof
///求结构体中一个成员在该结构体中的偏移量
#define offsetof(TYPE, MEMBER) ((acoral_u32) &((TYPE *)0)->MEMBER)
#endif
///找到某成员属于的结构体的起始地址
#define container_of(ptr, type, member) ((type *)((acoral_8 *)ptr - offsetof(type,member)))
///返回输入size对齐到特定宽度align的最小值
#define ACORAL_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
#endif
