/**
 * @file mem_tlsf.h
 * @author 胡旋
 * @brief
 * @version 0.1
 * @date 2025-03-07
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡旋 <td>2025-05-07<td>规范代码风格
 * </table>
 */

#ifndef __OSMEM_H__
#define __OSMEM_H__

#include <acoral.h>

/* typedef - 类型定义 */
typedef int BOOL;         /* 逻辑型变量类型 */
typedef int STATUS;       /* 状态型变量类型 */
typedef int (*FUNCPTR)(); /* 函数指针变量类型 */

#ifndef CFG_MEM_TLSF
/* function declarations - 函数声明 */
STATUS tlsf_memlib_init(void);
void *tlsf_mem_alloc(acoral_u32 size);
void *tlsf_mem_realloc(void *ptr, acoral_u32 size);
STATUS tlsf_mem_free(void *ptr);
void *tlsf_mem_cpy(void *to, const void *from, acoral_u32 size);
void *tlsf_mem_set(void *dst, acoral_u8 val, acoral_u32 count);
#endif

/* define - 定义 */
#define OSOK (0)
#define OSERROR (-1)

/* 错误号 */
int OSErrNo;

/* 设置错误号 */
#define ACORAL_ERR_SET_ERRNO(errno) \
    do                              \
    {                               \
        OSErrNo = (errno);          \
    } while (0)

/*定义内存池,根据平台配置*/
#define ACORAL_CFGMemPoolAddr ((volatile acoral_u32)0x10000000) /*DDR 内存起始地址*/
#define ACORAL_CFGMemPoolSize (1024 * 1024 * 258) /*定义32MB大小的内存池*/

#endif
