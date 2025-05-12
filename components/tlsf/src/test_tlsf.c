/**
 * @file test_theads.c
 * @author 胡旋
 * @brief tlfs部分函数测试代码
 * @version 0.1
 * @date -
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>胡旋 <td>- <td>内容
 * </table>
 */

#include <acoral.h>
#include "mem_tlsf.h"

#define MAXNUM   20
#define ASSIGNSIZE 32
#define POOL_SIZE 1024 * 1024

void test_tlsf()
{
    int *ptr[MAXNUM];
    int test[MAXNUM];
    int i;
    int count = 0;
    STATUS mem_status,free_mem;
    mem_status =  tlsf_memlib_init();
    if(mem_status == OSERROR)
    {
        acoral_print("mempool error!");
        OSErrNo =  KR_MEM_ERR_UNDEF; \
        return; 
    }
    acoral_print("mempool succeed \r\n");
    /*分配内存*/
    for (i=0; i< MAXNUM; i++)
    {
        if (!(ptr[i]=tlsf_mem_alloc(ASSIGNSIZE)))
        {
          acoral_print("Error\n");
          OSErrNo = KR_MEM_ERR_MALLOC;
          return;
        }
        acoral_print("ptr[%d] address: %x \r\n",i,ptr[i]); 
    }
    acoral_print("MemAlloc succeed \r\n");  
    
    /*测试数据*/
    acoral_print("TEST Cpy: \r\n");
    for (i=0; i< MAXNUM; i++)
    {
        test[i] = count++;
        tlsf_mem_cpy(ptr[i], &test[i],ASSIGNSIZE);
        acoral_print("ptr[%d] value: %d \r\n",i,*ptr[i]); 
    } 
    acoral_print("TEST Set: \r\n");
   int *temp = tlsf_mem_alloc(1);
    tlsf_mem_set(temp,6,1);
    acoral_print("temp value: %d \r\n",*temp); 
    acoral_print("TEST realloc: \r\n");
    tlsf_mem_realloc(temp,4);
    tlsf_mem_set(temp,6,4);
    acoral_print("temp value: %x \r\n",*temp); 
  
    /*释放内存*/
    for(i=0; i< MAXNUM; i++)
    {
        free_mem = tlsf_mem_free(ptr[i]);
        if(free_mem == OSERROR)
        {
            acoral_print("mempool error!");
            OSErrNo =  KR_MEM_ERR_UNDEF;  
            return;
        }  
    }  
    // destroy_memory_pool(pool);
    acoral_print("Test OK !\n");
    return;
}