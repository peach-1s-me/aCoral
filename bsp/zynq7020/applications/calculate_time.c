/**
 * @file caculate_time.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 开销测试
 * @version 0.1
 * @date 2024-07-10
 * 
 * Copyright (c) 2024
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2024-07-10 <td>内容
 * </table>
 */
#include <acoral.h>
#include "calculate_time.h"

#define CAT_INT_LONG    10
#define MAX_STRNG_LEN   300
#define CAT_FLOAT_LONG  42

void print_float(double num, acoral_u8 keep, acoral_32 width)
{
    /* cat_float_t : [+/- 1.18 * 10^-38, +/- 3.4 * 10^38], 算上符号, 小数点和末尾符号一共 42 个字符 */
    acoral_u8 buf[CAT_FLOAT_LONG] = {0};
    acoral_u8 i = 0;

    /* 最少保留一位小数 */
    if(0 == keep)
    {
        keep = 1;
    }
    

    if(num < 0)
    {
        acoral_print("%c", '-');
        num = -num;
    }

    acoral_32 int_part = (acoral_32)num;
    num -= int_part;
    if(0 == int_part)
    {
        buf[i++] = 0;
    }
    else
    {
        do
        {
            buf[i++] = (acoral_u8)(int_part % 10);
            int_part = int_part / 10;
        }while((0 != int_part) && (i < CAT_FLOAT_LONG));
    }

    /* 计算保留位数的截止处(还要多计算一位) */
    acoral_u32 keep_end = i + keep + 1;
    
    if(i < CAT_INT_LONG - 1)
    {
        buf[i++] = '.';

        if(0.0f == num)
        {
            buf[i++] = 0;
        }
        else
        {
            do
            {
                num *= 10;
                int_part = (acoral_32)num;
                buf[i++] = int_part;
                num -= int_part;
            }while((num != 0.0f) && (i <= keep_end)); /* 多算一位是为了四舍五入 */

            while(i <= keep_end)
            {
                buf[i++] = 0;
            }

            /* 减掉用于四舍五入的那一位 */
            i--;

            if(buf[keep_end] >= 5)
            {

                do
                {
                    keep_end--;
                    buf[keep_end] = (buf[keep_end] + 1) % 10;
                }while(
                    (0   == buf[keep_end]) &&
                    ('.' != buf[keep_end])
                );

                if('.' == buf[keep_end])
                {
                    /* 跳过小数点 */
                    keep_end--;

                    if(0 == buf[keep_end + 2])
                    {
                        do
                        {
                            keep_end--;
                            buf[keep_end] = (buf[keep_end] + 1) % 10;
                        }while(
                            (0 == buf[keep_end]) &&
                            (0 != keep_end)
                        );
                    }
                }
            } /* buf[keep_end] > 5 */
        }
    }
    else
    {
        acoral_print("[printf-float] ERROR: buf overflow!\r\n");
    }

    /* 如果有对齐要求则先输出空格 */
    width = width - i;
    while(width > 0)
    {
        acoral_print("%c", ' ');
        width--;
    }

    /* 找到小数点位置, 整数部分 buf 要倒序输出, 小数部分 buf 要正序输出 */
    acoral_u8 int_idx = 0, dec_idx = 0;
    while(('.' != buf[int_idx]) && (int_idx < CAT_INT_LONG))
    {
        int_idx++;
    }

    /* 小数点位置下一个就是小数开始的位置 */
    dec_idx = int_idx + 1;

    /* 输出整数部分 */
    while(int_idx > 0)
    {
        acoral_print("%c", buf[--int_idx] + '0');
    }

    acoral_print("%c", '.');

    /* 输出小数部分 */
    while(dec_idx < i)
    {
        acoral_print("%c", buf[dec_idx++] + '0');
    }
}

void tic(void)
{
   *((volatile int*)(GTC_BASE+GTC_CTRL)) = 0x00;
   *((volatile int*)(GTC_BASE+GTC_DATL)) = 0x00000000;
   *((volatile int*)(GTC_BASE+GTC_DATH)) = 0x00000000;     //清零定时器的计数值
   *((volatile int*)(GTC_BASE+GTC_CTRL)) = 0x01;
}
double toc(void)
{
   *((volatile int*)(GTC_BASE+GTC_CTRL)) = 0x00;
   long long j=*((volatile int*)(GTC_BASE+GTC_DATH));
   double elapsed_time = j<<32;
   j=*((volatile int*)(GTC_BASE+GTC_DATL));              //读取64bit定时器值，转换为double
   elapsed_time+=j;
   elapsed_time=elapsed_time*2000.0/666666687.0;
//    acoral_print("%f\r\n",elapsed_time);
   return elapsed_time;
}

acoral_u8 cal_flag = 0;
void cal_time_start()
{
   if(cal_flag == 0)
   {
       tic();
       cal_flag = 1;
   }
}

/**
 * @brief 
 * 
 * @return double >0: timer value
 *                else: gg
 */
double cal_time_end()
{
    double ret = -1.0;
    if(cal_flag == 1)
    {
       ret = toc();
       cal_flag = 0;
    }
    return ret;
}