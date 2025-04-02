/**
 * @file bitops.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层位图相关源文件
 * @version 2.0
 * @date 2025-03-27
 * 
 * @copyright Copyright (c) 2022 EIC-UESTC
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-02 <td>增加注释
 *         <tr><td>v2.0 <td>饶洪江 <td>2025-03-27 <td>规范代码风格，小幅重构
 */
#include "type.h"
#include "bitops.h"

/**
 * @brief 从低位寻找第一个为1的位（单位图）
 * 
 * @param bitmap 需要寻找的32位图
 * @return acoral_u32 从低位开始第一个为1的位数
 */
acoral_u32 acoral_find_first_set(acoral_u32 bitmap)
{
    acoral_u32 position;
    position = 31;
    if (bitmap & 0x0000ffff)
    { 
        position -= 16;
        bitmap <<= 16;
    }
    if (bitmap & 0x00ff0000)
    {
        position -= 8;
        bitmap <<= 8;
    }
    if (bitmap & 0x0f000000)
    {
        position -= 4;
        bitmap <<= 4;
    }
    if (bitmap & 0x30000000)
    {
        position -= 2;
        bitmap <<= 2;
    }
    if (bitmap & 0x40000000)
    {
        position -= 1;
    }
    return position;
}

/**
 * @brief 从低位寻找第一个为1的位（多位图）
 * 
 * @param bitmaps 需要寻找的32位图指针
 * @param length  32位图长度
 * @return acoral_u32 从低位开始第一个为1的位数
 */
acoral_u32 acoral_find_first_bit(const acoral_u32 *bitmaps, acoral_u32 length)
{
    acoral_u32 bitmap;
    acoral_u32 off;

    for (off = 0; bitmap = bitmaps[off], off < length; off++)
    {
        if (bitmap)
        {
            break;
        }
    }
    return acoral_find_first_set(bitmap) + off * 32;
}

/**
 * @brief 把位图某位置1
 * 
 * @param position 需要置1的位数
 * @param bitmap   32位图指针
 */
void acoral_set_bit(acoral_32 position,acoral_u32 *bitmap)
{
    acoral_u32 old_val, mask = 1UL << (position & 31);
    acoral_u32 *temp;

    temp    = bitmap + (position >> 5);
    old_val = *temp;
    *temp   = old_val | mask;
}

/**
 * @brief 把位图某位清0
 * 
 * @param position 需要清0的位数
 * @param bitmap   32位图指针
 */
void acoral_clear_bit(acoral_32 position, acoral_u32 *bitmap)
{
    acoral_u32 old_val, mask = 1UL << (position & 31);
    acoral_u32 *temp;

    temp    = bitmap + (position >> 5);
    old_val = *temp;
    *temp   = old_val & (~mask);
}

/**
 * @brief 获取位图中某一位的值
 * 
 * @param position 想要获取值的位数
 * @param bitmap   32位图指针
 * @return acoral_u32 某位的值
 */
acoral_u32 acoral_get_bit(acoral_32 position, acoral_u32 *bitmap)
{
    acoral_u32 old_val, mask = 1UL << (position & 31);
    acoral_u32 *temp;

    temp    = bitmap + (position >> 5);
    old_val = *temp;
    return old_val & mask;
}
