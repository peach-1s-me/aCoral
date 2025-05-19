/**
 * @file str.h
 * @author 胡博文 (@921576434@qq.com)
 * @brief component层lib库字符串相关头文件
 * @version 1.0
 * @date 2022-09-21
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-09-21 <td>增加注释
 */
#ifndef LIB_STR_H
#define LIB_STR_H
#include <type.h>

#define IS_DIGIT(c) ((c)>='0'&&(c)<'9')
#define IS_LOWER(c) ((c)>='a'&&(c)<='z')
#define IS_UPPER(c) ((c)>='A'&&(c)<='Z')

acoral_u8 acoral_tolower(acoral_u8 c);
acoral_u8 acoral_toupper(acoral_u8 c);
acoral_u32 acoral_str_cmp(const acoral_char * des,const acoral_char * src);
acoral_char *acoral_str_cpy(acoral_char *to, const acoral_char *from);
acoral_32 acoral_str_nicmp(const acoral_char *s1, const acoral_char *s2, acoral_u32 len);
acoral_char *acoral_str_ncpy(acoral_char * dest,const acoral_char *src,acoral_u32 count);
acoral_u32 acoral_str_lcpy(acoral_char *dest, const acoral_char *src, acoral_u32 size);
acoral_char *acoral_str_cat(acoral_char * dest, const acoral_char * src);
acoral_char *acoral_str_ncat(acoral_char *dest, const acoral_char *src, acoral_u32 count);
acoral_u32 acoral_str_lcat(acoral_char *dest, const acoral_char *src, acoral_u32 count);
acoral_32 acoral_str_ncmp(const acoral_char * cs,const acoral_char * ct,acoral_u32 count);
acoral_32 acoral_str_chr(const acoral_char * s, acoral_32 c);
acoral_32 acoral_str_rchr(const acoral_char * s, acoral_32 c);
acoral_32 acoral_str_nchr(const acoral_char *s, acoral_u32 count, acoral_32 c);
acoral_u32 acoral_str_len(const acoral_char * s);
acoral_u32 acoral_str_nlen(const acoral_char * s, acoral_u32 count);
acoral_u32 acoral_str_spn(const acoral_char *s, const acoral_char *accept);
acoral_u32 acoral_str_cspn(const acoral_char *s, const acoral_char *reject);
acoral_char *acoral_str_pbrk(const acoral_char * cs,const acoral_char * ct);
acoral_char *acoral_str_sep(acoral_char **s, const acoral_char *ct);
void *acoral_memset(void * s,acoral_32 c,acoral_u32 count);
void *acoral_memcpy(void * dest,const void *src,acoral_u32 count);
void *acoral_memmove(void * dest,const void *src,acoral_u32 count);
acoral_32 acoral_memcmp(const void * cs,const void * ct,acoral_u32 count);
void *acoral_memscan(void * addr, acoral_32 c, acoral_u32 size);
acoral_char *acoral_strstr(const acoral_char * s1,const acoral_char * s2);
void *acoral_memchr(const void *s, acoral_32 c, acoral_u32 n);

acoral_32 acoral_atoi(const char *src);

#endif

