/**
 * @file bitops.c
 * @author 胡博文 (@921576434@qq.com)
 * @brief kernel层位图相关源文件
 * @version 1.0
 * @date 2022-07-02
 * 
 * @copyright Copyright (c) 2022
 * 
 * @par 修订历史
 *     <table>
 *         <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 *         <tr><td>v1.0 <td>胡博文 <td>2022-07-02 <td>增加注释
 */
#include <type.h>
#include <bitops.h>
/**
 * @brief 从低位寻找第一个为1的位（单位图）
 * 
 * @param word 需要寻找的32位图
 * @return acoral_u32 从低位开始第一个为1的位数
 */
acoral_u32 acoral_ffs(acoral_u32 word)
{
    acoral_u32 k;
    k = 31;
    if (word & 0x0000ffff) { k -= 16; word <<= 16; }
    if (word & 0x00ff0000) { k -= 8;  word <<= 8;  }
    if (word & 0x0f000000) { k -= 4;  word <<= 4;  }
    if (word & 0x30000000) { k -= 2;  word <<= 2;  }
    if (word & 0x40000000) { k -= 1; }
    return k;
}

/**
 * @brief 从低位寻找第一个为1的位（多位图）
 * 
 * @param b 需要寻找的32位图指针
 * @param length 32位图长度
 * @return acoral_u32 从低位开始第一个为1的位数
 */
acoral_u32 acoral_find_first_bit(const acoral_u32 *b,acoral_u32 length)
{
    acoral_u32 v;
    acoral_u32 off;

    for (off = 0; v = b[off], off < length; off++)
    {
        if (v)
            break;
    }
    return acoral_ffs(v) + off * 32;
}

/**
 * @brief 把位图某位置1
 * 
 * @param nr 需要置1的位数
 * @param bitmap 32位图指针
 */
void acoral_set_bit(acoral_32 nr,acoral_u32 *bitmap)
{
    acoral_u32 oldval, mask = 1UL << (nr & 31);
    acoral_u32 *p;
    p =bitmap+(nr>>5);
    oldval = *p;
    *p = oldval | mask;
}
/**
 * @brief 把位图某位清0
 * 
 * @param nr 需要清0的位数
 * @param bitmap 32位图指针
 */
void acoral_clear_bit(acoral_32 nr,acoral_u32 *bitmap)
{
    acoral_u32 oldval, mask = 1UL << (nr & 31);
    acoral_u32 *p;
    p =bitmap+(nr >> 5);
    oldval = *p;
    *p = oldval &(~mask);
}
/**
 * @brief 获取位图中某一位的值
 * 
 * @param nr 想要获取值的位数
 * @param bitmap 32位图指针
 * @return acoral_u32 某位的值
 */
acoral_u32 acoral_get_bit(acoral_32 nr,acoral_u32 *bitmap)
{
    acoral_u32 oldval, mask = 1UL << (nr & 31);
    acoral_u32 *p;
    p =bitmap+(nr>>5);
    oldval = *p;
    return oldval & mask;
}
