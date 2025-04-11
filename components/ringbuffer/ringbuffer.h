/**
 * @file ringbuffer.h
 * @author 文佳源 (648137125@qq.com)
 * @brief 环形缓冲区头文件
 * @version 2.0
 * @date 2025-04-11
 *
 * Copyright (c) 2023 EIC-UESTC
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2023-06-17 <td>新建文件
 * <tr><td>v2.0 <td>文佳源 <td>2025-04-11 <td>规范代码
 * </table>
 */
#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include "acoral.h"

#define MIN_RINGBUFFER_SIZE 4

typedef struct
{
    acoral_u8 *p_buffer;   /* 放置数据的缓冲区地址 */
    acoral_u32 ring_mask;  /* 用于循环头尾索引的掩码，用于代替取余操作 */
    acoral_u32 tail_index; /* 尾部索引 */
    acoral_u32 head_index; /* 头部索引 */
} ringbuffer_t;


void ringbuffer_init(
    ringbuffer_t *p_ringbuffer,
    acoral_u8    *p_buffer_space, 
    acoral_u32    buffer_size
);

void ringbuffer_clear(ringbuffer_t *p_ringbuffer);

acoral_32 ringbuffer_put(ringbuffer_t *p_ringbuffer, acoral_u8 data);
acoral_32 ringbuffer_get(ringbuffer_t *p_ringbuffer, acoral_u8 *p_data);

acoral_32 ringbuffer_put_more(
    ringbuffer_t    *p_ringbuffer,
    const acoral_u8 *p_data,
    acoral_u32       size
);
acoral_u32 ringbuffer_get_more(
    ringbuffer_t *p_ringbuffer,
    acoral_u8    *p_data,
    acoral_u32    size
);

/**
 * @brief 检查环形缓冲区是否已满
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @return acoral_u8      1：已满
 *                          0：未满
 */
static inline acoral_u8 ringbuffer_is_full(ringbuffer_t *p_ringbuffer)
{
    acoral_u8 ret;

    /**
     * 这里
     * ((p_ringbuffer->head_index - p_ringbuffer->tail_index) & (p_ringbuffer->ring_mask))
     * 这个最外层括号必须加，否则就会把(p_ringbuffer->ring_mask)) ==
        (p_ringbuffer->ring_mask)的结果(一定为真)和前面相减的结果按位与，就错辣！
     */
    ret = (((p_ringbuffer->head_index - p_ringbuffer->tail_index) & (p_ringbuffer->ring_mask)) ==
           (p_ringbuffer->ring_mask));

    return ret;
}

/**
 * @brief 检查环形缓冲区是否为空
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @return acoral_u8      1：非空
 *                          0；为空
 */
static inline acoral_u8 ringbuffer_is_empty(ringbuffer_t *p_ringbuffer)
{
    acoral_u8 ret;

    ret = ((p_ringbuffer->head_index) == (p_ringbuffer->tail_index));

    return ret;
}

#endif /* #ifndef RINGBUFFER_H */