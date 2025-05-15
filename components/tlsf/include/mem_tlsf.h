/**
 * @file mem_tlsf.h
 * @author 胡旋
 * @brief
 * @version 0.1
 * @date 2025-03-07
 *
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡旋   <td>2025-05-07<td>初步完成tlsf移植
 *         <tr><td>v1.1 <td>文佳源 <td>2025-05-13<td>规范代码
 *         <tr><td>v1.2 <td>饶洪江 <td>2025-05-14<td>修改错误定义，规范代码
 * </table>
 */

#ifndef __OSMEM_H__
#define __OSMEM_H__

#include <acoral.h>

#ifndef CFG_MEM_TLSF
/* function declarations - 函数声明 */
acoral_err get_tlsf_err(void);
acoral_err tlsf_memlib_init(void);
void *tlsf_mem_alloc(acoral_u32 size);
void *tlsf_mem_realloc(void *ptr, acoral_u32 size);
acoral_err tlsf_mem_free(void *ptr);
void *tlsf_mem_cpy(void *to, const void *from, acoral_u32 size);
void *tlsf_mem_set(void *dst, acoral_u8 val, acoral_u32 count);
#endif

/*定义内存池,根据平台配置*/
#define ACORAL_CFG_MEM_POOl_ADDR ((volatile acoral_u32)0x10000000) /*DDR 内存起始地址*/
#define ACORAL_CFG_MEM_POOL_SIZE (1024 * 1024 * 258) /*定义32MB大小的内存池*/

#endif
