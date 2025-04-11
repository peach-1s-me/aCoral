/**
 * @file ringbuffer.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 环形缓冲区源文件
 * @version 2.0
 * @date 2025-04-11
 *
 * Copyright (c) 2023 EIC-UESTC
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2023-06-17 <td>新建文件
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * <tr><td>v2.0 <td>文佳源 <td>2025-04-11 <td>规范代码
 * </table>
 */
#include "acoral.h"
#include "ringbuffer.h"

#define RB_EOK 0
#define RB_ERR -1

/* 是否为2^n */
#define RB_IS_POWER_OF_TWO(_num) \
    ((_num & (_num - 1)) == 0)

/* 将环形缓冲区指针向前移动 */
#define RB_INDEX_MOVE_FOWARD(_ringbuffer, _index_name)                                                \
    do {                                                                                              \
        (_ringbuffer)->_index_name = (((_ringbuffer)->_index_name + 1) & ((_ringbuffer)->ring_mask)); \
    } while (0)

/**
 * @brief 缓冲区初始化
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @param  p_buffer_space   缓冲区内存空间首地址
 * @param  buffer_size      缓冲区大小(字节)
 */
void ringbuffer_init(
    ringbuffer_t *p_ringbuffer,
    acoral_u8    *p_buffer_space, 
    acoral_u32    buffer_size
)
{
    ACORAL_ASSERT(NULL != p_ringbuffer);
    ACORAL_ASSERT(NULL != p_buffer_space);
    ACORAL_ASSERT(buffer_size >= MIN_RINGBUFFER_SIZE);
    /* 如果要使用ring_mask来代替取余操作实现循环，buffer大小必须为2的n次方 */
    ACORAL_ASSERT(1 == (RB_IS_POWER_OF_TWO(buffer_size)));

    p_ringbuffer->p_buffer = p_buffer_space;
    p_ringbuffer->ring_mask = buffer_size - 1;
    p_ringbuffer->tail_index = 0;
    p_ringbuffer->head_index = 0;
}

/**
 * @brief 清空缓冲区里所有数据
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 */
void ringbuffer_clear(ringbuffer_t *p_ringbuffer)
{
    ACORAL_ASSERT(NULL != p_ringbuffer);

    p_ringbuffer->tail_index = 0;
    p_ringbuffer->head_index = 0;
}

/**
 * @brief 向环形缓冲区放一个字节数据
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @param  data             要存入的数据值
 * @return acoral_32    0：   成功
 *                      else：失败
 */
acoral_32 ringbuffer_put(ringbuffer_t *p_ringbuffer, acoral_u8 data)
{
    acoral_32 ret = RB_ERR;

    if (0 != ringbuffer_is_full(p_ringbuffer))
    {
        /* 如果满了就覆盖最早的 */
        acoral_print("[ringbuffer] buffer overflow\r\n");

        /* 尾部索引往前走 */
        RB_INDEX_MOVE_FOWARD(p_ringbuffer, tail_index);
    }
    else
    {
        ret = RB_EOK;
    }

    p_ringbuffer->p_buffer[p_ringbuffer->head_index] = data;
    RB_INDEX_MOVE_FOWARD(p_ringbuffer, head_index);

    return ret;
}

/**
 * @brief 从环形缓冲区取一个字节数据
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @param  p_data           取出的数据存放的数据地址
 * @return acoral_32    0：   成功
 *                      else：失败
 */
acoral_32 ringbuffer_get(ringbuffer_t *p_ringbuffer, acoral_u8 *p_data)
{
    acoral_32 ret = RB_ERR;

    if (0 == ringbuffer_is_empty(p_ringbuffer))
    {
        /* 不为空才执行 */
        *p_data = p_ringbuffer->p_buffer[p_ringbuffer->tail_index];
        RB_INDEX_MOVE_FOWARD(p_ringbuffer, tail_index);

        ret = RB_EOK;
    }

    return ret;
}

/**
 * @brief
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @param  p_data           要存入的数据的起始地址
 * @param  size             要存入的数据的长度
 * @return acoral_32        放入环形缓冲区实际的数据个数(目前每个数据单位为字节)
 */
acoral_32 ringbuffer_put_more(
    ringbuffer_t    *p_ringbuffer,
    const acoral_u8 *p_data,
    acoral_u32       size
)
{
    acoral_32 ret = 0;
    acoral_u32 i = 0;

    for (i = 0; i < size; i++)
    {
        ret += ringbuffer_put(p_ringbuffer, p_data[i]);
    }

    if (0 != ret)
    {
        ret = RB_ERR;
    }

    return ret;
}

/**
 * @brief
 *
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @param  p_data           取出数据要存放的起始地址
 * @param  size             要取出数据的长度
 * @return acoral_u32     从环形缓冲区实际取出的数据个数(目前每个数据单位为字节)
 */
acoral_u32 ringbuffer_get_more(
    ringbuffer_t *p_ringbuffer,
    acoral_u8    *p_data,
    acoral_u32    size
)
{
    acoral_32 err = RB_ERR;
    acoral_u8 *p = NULL;
    acoral_u32 cnt = 0;

    if (0 == ringbuffer_is_empty(p_ringbuffer))
    {
        /* 不为空才继续 */
        p = p_data;
        cnt = 0;

        do {
            err = ringbuffer_get(p_ringbuffer, p);
            cnt++;
            p++;
        } while ((cnt < size) && (RB_EOK == err));
    }

    return cnt;
}

/* 测试用函数 */
void ringbuffer_print_all(ringbuffer_t *p_ringbuffer)
{
    acoral_u32 idx = p_ringbuffer->tail_index;

    acoral_print("************\n****tail=%2d, head=%2d****\n", (p_ringbuffer->tail_index), (p_ringbuffer->head_index));
    while (idx != (p_ringbuffer->head_index))
    {
        acoral_print("index=%2d, data=0x%x\n", idx, p_ringbuffer->p_buffer[idx]);
        idx = (idx + 1) & (p_ringbuffer->ring_mask);
    }
    acoral_print("************\n");
}
