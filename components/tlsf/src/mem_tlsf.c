/**
 * @file mem_tlsf.c
 * @author 胡旋 
 * @brief 
 * @version 0.1
 * @date 2025-03-07
 * @par 修订历史
 * 
 * <table>
 * <tr><td>v1.0 <td>胡旋 <td>2025-05-07<td>规范代码风格
 * </table>
 */

#include <acoral.h>
#include "mem_tlsf.h"

/*对齐宏定义*/
///内存块对齐粒度（8字节）
#define BLOCK_ALIGN		(0x8)	
///对齐掩码（0x7）
#define MEM_ALIGN					((BLOCK_ALIGN) - 1) 							
///向上对齐到8字节*/
#define ROUNDUP_SIZE(_r)			(((_r) + MEM_ALIGN) &(acoral_u32)(~MEM_ALIGN))
///向下对齐到8字节
#define ROUNDDOWN_SIZE(_r)			((_r) & (acoral_u32)(~MEM_ALIGN))

/*块大小范围与索引划分*/
///第一级索引的起始偏移（2^6=64字节）
#define FLI_OFFSET		(6)	
///第一级索引最大值（对应块大小2^30=1GB）
#define MAX_FLI			(30)
/// 第二级索引对数（SLI数量=2^5=32）
#define MAX_LOG2_SLI	(5)	
///第二级索引数（32）
#define MAX_SLI			(1 << MAX_LOG2_SLI)     /* MAX_SLI = 2^MAX_LOG2_SLI */					
///实际使用的FLI数量（24）
#define REAL_FLI		(MAX_FLI - FLI_OFFSET)	
#define SMALL_BLOCK		(128)					
/*最小块与头部开销*/
///最小内存块大小（8字节）
#define MIN_BLOCK_SIZE	(0x8)	
//#define MIN_BLOCK_SIZE	(sizeof (FREE_PTR_T))
///头部元数据开销（8字节）
#define BHDR_OVERHEAD	(0x10 - MIN_BLOCK_SIZE)	
//#define BHDR_OVERHEAD	(sizeof (BHDR) - MIN_BLOCK_SIZE)

/*内存池校验*/	
#define TLSF_SIGNATURE	(0x2A59FA59)

/*状态位掩码*/
///块状态掩码（低2位）
#define PTR_MASK		(3)
///块实际大小掩码（屏蔽低2位）
#define BLOCK_SIZE		(0xFFFFFFFF - PTR_MASK)

/*空闲链表遍历*/
#define GET_NEXT_BLOCK(_addr, _r)	((BHDR *) ((acoral_8 *) (_addr) + (_r)))

// #define ROUNDUP(_x, _v)				((((acoral_u32)(~(_x)) + 1) & ((_v)-1)) + (_x))
									
#define PREV_STATE	(0x2)
/*块状态宏*/
///当前块空闲标记											
#define FREE_BLOCK	(0x1)
///当前块已用标记		
#define USED_BLOCK	(0x0)
///前驱块空闲标记												
#define PREV_FREE	(0x2)
///前驱块已用标记		
#define PREV_USED	(0x0)			


/**
 * @brief 空闲链表节点
 * 
 */
typedef struct 		
{
	struct bhdr_struct *prev;/*前驱空闲块指针*/		
	struct bhdr_struct *next;/*后继空闲块指针*/		
} free_prt_t;

/**
 * @brief 内存块头部 
 * 
 */
typedef struct bhdr_struct			
{
	struct bhdr_struct *prev_hdr;/*物理相邻的前一块的头部指针*/	
	acoral_u32 size;			 /*内存块大小（含状态标记）*/				
	union							
	{
		free_prt_t free_ptr;	/*空闲时：维护空闲链表指针*/
		acoral_u8 buffer[1];	/*分配时：用户数据区起始地址*/
	} ptr;
} bhdr_struct_t;

typedef struct 	
{
	acoral_u32 tlsf_signature;				   /*校验内存池合法性*/		
	acoral_u32 fl_bitmap;					   /*一级位图*/
	acoral_u32 sl_bitmap[REAL_FLI];			   /*二级位图数组*/
	bhdr_struct_t *matrix[REAL_FLI][MAX_SLI];  /*空闲链表头指针二维数组*/
} tlsf_struct_t;
/*重定义结构体*/
typedef  bhdr_struct_t BHDR;
typedef  tlsf_struct_t TLSF;
			
acoral_u32 tlsf_mempool_addr_g; /*定义内存池地址*/	
static u32  tlsf_table[256];		
int OSErrNo = 0;				/*初始化错误号*/	

static void tlsf_setbit(acoral_u32 nr, acoral_u32 * addr);
static void tlsf_clearbit(acoral_u32 nr, acoral_u32 * addr);
static acoral_u32 tlsf_get_lsbit(acoral_u32 i);
static acoral_u32 tlsf_get_msbit(acoral_u32 i);
//static void InsertBlock(BHDR * _b, TLSF * _tlsf,I32 _fl,I32 _sl);
static void tlsf_mapping_search(acoral_u32 * _r, acoral_u32 *_fl, acoral_u32 *_sl);
static void tlsf_mapping_insert(acoral_u32 _r, acoral_u32 *_fl, acoral_u32 *_sl);
static BHDR *tlsf_find_suitableblock(TLSF * _tlsf, acoral_u32 *_fl, acoral_u32 *_sl);

/**
 * @brief 从 TLSF 的空闲内存块链表中提取一个内存块（BHDR），并更新链表和位图信息
 * 
 * @param _b 指向要提取的内存块（BHDR 结构体）
 * @param _tlsf 指向 TLSF 管理器的指针
 * @param _fl 第一级索引（First Level Index），表示内存块的大小范围
 * @param _sl 第二级索引（Second Level Index），表示内存块在某个大小范围内的具体位置
 */
void tlsf_mem_extractblock_hdr(BHDR *_b, TLSF *_tlsf, acoral_u32 _fl, acoral_u32 _sl) 
{
	(_tlsf) -> matrix [(_fl)] [(_sl)] = (_b) -> ptr.free_ptr.next;	
		if ((_tlsf) -> matrix[(_fl)][(_sl)])
		{															
			(_tlsf) -> matrix[(_fl)][(_sl)] -> ptr.free_ptr.prev = NULL;
		}															
		else													
		{														
			tlsf_clearbit ((_sl), &(_tlsf) -> sl_bitmap [(_fl)]);
			if (!(_tlsf) -> sl_bitmap [(_fl)])	
			{	
				tlsf_clearbit ((_fl), &(_tlsf) -> fl_bitmap);
			}										
		}												
		(_b) -> ptr.free_ptr.prev =  NULL;					
		(_b) -> ptr.free_ptr.next =  NULL;	
}
/**
 * @brief 从 TLSF 的空闲内存块链表中提取一个指定的内存块（_b），并更新链表和位图信息。
 * 
 * @param _b 指向要提取的内存块（BHDR结构体）。
 * @param _tlsf 指向 TLSF 管理器的指针。
 * @param _fl 第一级索引（First Level Index），表示内存块的大小范围。
 * @param _sl 第二级索引（Second Level Index），表示内存块在某个大小范围内的具体位置。
 */
 void tlsf_mem_extractblock(BHDR *_b, TLSF *_tlsf, acoral_u32 _fl, acoral_u32 _sl) 
 {
 	 	if ((_b) -> ptr.free_ptr.next)
		{				
			(_b) -> ptr.free_ptr.next -> ptr.free_ptr.prev = (_b) -> ptr.free_ptr.prev; 
		}							
		if ((_b) -> ptr.free_ptr.prev)							
		{														
			(_b) -> ptr.free_ptr.prev -> ptr.free_ptr.next = (_b) -> ptr.free_ptr.next; 
		}																
		if ((_tlsf) -> matrix [(_fl)][(_sl)] == (_b))							
		{																
			(_tlsf) -> matrix [(_fl)][(_sl)] = (_b) -> ptr.free_ptr.next;		
			if (!(_tlsf) -> matrix [(_fl)][(_sl)])							
			{															
				tlsf_clearbit ((_sl), &(_tlsf) -> sl_bitmap[(_fl)]);			
			}															
			if (!(_tlsf) -> sl_bitmap [(_fl)])								
			{															
				tlsf_clearbit ((_fl), &(_tlsf) -> fl_bitmap);				
			}														
		}																
		(_b) -> ptr.free_ptr.prev = NULL;								
		(_b) -> ptr.free_ptr.next = NULL;									
 }
/**
 * @brief 将一个内存块（_b）插入到 TLSF 的空闲内存块链表中，并更新链表和位图信息。
 * 
 * @param _b 指向要插入的内存块（BHDR 结构体）
 * @param _tlsf 指向 TLSF 管理器的指针。
 * @param _fl 第一级索引（First Level Index），表示内存块的大小范围。
 * @param _sl 第二级索引（Second Level Index），表示内存块在某个大小范围内的具体位置。
 */
  void tlsf_mem_insertblock(BHDR *_b, TLSF *_tlsf, acoral_u32 _fl, acoral_u32 _sl) 
  {
		(_b) -> ptr.free_ptr.prev = NULL;									
		(_b) -> ptr.free_ptr.next = (_tlsf) -> matrix [(_fl)][(_sl)];		
		if ((_tlsf) -> matrix [(_fl)][(_sl)])									
		{																
			(_tlsf) -> matrix [(_fl)][(_sl)] -> ptr.free_ptr.prev = (_b);		
		}															
		(_tlsf) -> matrix [(_fl)][(_sl)] = (_b);							
		tlsf_setbit ((_sl), &(_tlsf) -> sl_bitmap [(_fl)]);						
		tlsf_setbit ((_fl), &(_tlsf) -> fl_bitmap);								
  }

/**
 * @brief 计算一个无符号 32 位整数（acoral_u32）中最低有效位（LSB）的位置。
 * 
 *@param i 一个无符号 32 位整数。
 *@return 最低有效位的位置（从 0 开始计数）
 */
acoral_u32 tlsf_get_lsbit(acoral_u32 i)
{
	acoral_u32 a;
	acoral_u32 x = (acoral_u32)i & (acoral_u32)(-i);

	a = (acoral_u32)(x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24));
	return tlsf_table[x >> a] + a;
}


/**
 * @brief 计算一个无符号 32 位整数（acoral_u32）中最高有效位（MSB）的位置。
 * 
 *@param i 一个无符号 32 位整数。
 *@return 最高有效位的位置（从 0 开始计数）
 */
acoral_u32 tlsf_get_msbit(acoral_u32 i)
{
	acoral_u32 a;
	acoral_u32 x = (acoral_u32) i;

	a = (acoral_u32)(x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24));
	return tlsf_table[x >> a] + a;
}

/**
 * @brief 设置一个无符号 32 位整数数组（acoral_u32 *addr）中某一位（nr）为 1。
 * 
 *@param nr 要设置的位的位置（从 0 开始计数）。
 *@param addr 指向无符号 32 位整数数组的指针。
 */
void tlsf_setbit(acoral_u32 nr,acoral_u32 * addr)
{
	addr[nr >> 5] |= (acoral_u32)(1 << (nr & 0x1f));
}

/**
 *@brief 清除一个无符号 32 位整数数组 addr 中某一位（nr）的值，即将该位设置为 0。
 * 
 *@param nr 要清除的位的位置（从 0 开始计数）。
 *@param addr 指向无符号 32 位整数数组的指针。
 */
void tlsf_clearbit(acoral_u32 nr,acoral_u32 * addr)
{
    addr[nr >> 5] &= (acoral_u32)(~(1 << ((acoral_u32)nr & 0x1f)));
}

/**
 *@brief 将请求的内存大小（_r）映射到 TLSF 的两级索引（_fl 和 _sl）中。
 * 
 *@param _r 指向请求的内存大小的指针。
 *@param _fl 指向第一级索引（First Level Index）的指针。
 *@param _sl 指向第二级索引（Second Level Index）的指针。
 */
void tlsf_mapping_search(acoral_u32 * _r,acoral_u32 *_fl,acoral_u32 *_sl)
{
    acoral_u32 _t;

    if (*_r < SMALL_BLOCK)	
	{
        *_fl = 0;
        *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
    }
	else 					
	{
        _t = (acoral_u32)((1 << (tlsf_get_msbit(*_r) - MAX_LOG2_SLI)) - 1);
        *_r = *_r + _t;
        *_fl = tlsf_get_msbit(*_r);
        *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
        *_r &= ~_t;
    }
}

/**
 *@brief 将请求的内存大小（_r）映射到 TLSF 的两级索引（_fl 和 _sl）中。
 * 
 *@param _r 指向请求的内存大小的指针。
 *@param _fl 指向第一级索引（First Level Index）的指针。
 *@param _sl 指向第二级索引（Second Level Index）的指针。
 */
void tlsf_mapping_insert(acoral_u32 _r,acoral_u32 *_fl,acoral_u32 *_sl)
{
    if (_r < SMALL_BLOCK)	
	{
        *_fl = 0;
        *_sl = _r / (SMALL_BLOCK / MAX_SLI);
    }
	else					
	{
        *_fl = tlsf_get_msbit(_r);
        *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
    }
}

/***
 *@brief 在TLSF的空闲内存块链表中查找一个合适的内存块。
 * 
 *@param _tlsf 指向 TLSF 管理器的指针。
 *@param _fl 指向第一级索引（First Level Index）的指针。
 *@param _sl 指向第二级索引（Second Level Index）的指针。
 *@return 指向合适内存块的指针（BHDR *），如果未找到合适的内存块，则返回 Nacoral_uLL。
 */
BHDR *tlsf_find_suitableblock(TLSF * _tlsf,acoral_u32 *_fl,acoral_u32 *_sl)
{
    acoral_u32 _tmp = _tlsf->sl_bitmap[*_fl] & (0xFFFFFFFF << *_sl);
    BHDR *_b = (BHDR *)NULL;

    if (_tmp != (acoral_u32)NULL)		
	{
        *_sl = tlsf_get_lsbit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    }
	else						
	{
    *_fl = tlsf_get_lsbit(_tlsf->fl_bitmap & (0xFFFFFFFF << (*_fl + 1)));
    if(*_fl==0xFFFFFFFF)
    {
    		_b = (BHDR *)NULL;
    }
    else if (*_fl > 0)
		{
        *_sl = tlsf_get_lsbit(_tlsf->sl_bitmap[*_fl]);
        _b = _tlsf->matrix[*_fl][*_sl];
    }
    else
    {
    	_b = (BHDR *)NULL;
    }
  }
    return _b;
}

/**
 *@brief 初始化内存池和 TLSF 相关的数据结构。
 * 
 *@return STATU 类型，表示初始化是否成功。
 */
STATUS tlsf_memlib_init(void)
{
	BHDR *b, *lb;
	acoral_u32 i,j,k;
	acoral_8 *pchar;
	acoral_u32 size;
	acoral_u32	pool;

	pool = ACORAL_CFGMemPoolAddr;	
	size = ACORAL_CFGMemPoolSize;

	j=1;
	k=0;
	tlsf_table[0] = 0xffffffff;
	for(i=2;i<=256;i=i*2,k++)
	{
		for(;j<i;j++)
		{
			tlsf_table[j]=k;
		}
	}

	tlsf_mempool_addr_g = pool;
	pchar = (acoral_8 *)pool;
	do
	{
		*pchar++ = 0;
	}
	while (pchar < ((acoral_8 *)pool+sizeof(TLSF)));

	b = GET_NEXT_BLOCK(pool, ROUNDUP_SIZE(sizeof(TLSF)));
    b->size = ROUNDDOWN_SIZE(size -  sizeof(TLSF) - 2 * BHDR_OVERHEAD) | USED_BLOCK | PREV_USED;
    b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;

    lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    lb->prev_hdr = b;
    lb->size = 0x0 | USED_BLOCK | PREV_FREE;

    tlsf_mem_free(b->ptr.buffer);

	return OSOK;
}

/**
 *@brief 从内存池中分配指定大小的内存块。
 * 
 *@param size 请求分配的内存大小。
 */
void *tlsf_mem_alloc(acoral_u32 size)
{
    TLSF *tlsf;
    BHDR *b, *b2, *next_b;
    acoral_u32 fl=0, sl=0;
    acoral_u32 tmp_size;
	
	tlsf = (TLSF *) tlsf_mempool_addr_g;

	size = (acoral_u32)((size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size));

	tlsf_mapping_search(&size, &fl, &sl);
 				
	acoral_intr_disable();

	b = tlsf_find_suitableblock(tlsf, &fl, &sl);

	if (b == (BHDR *)NULL)
	{
		ACORAL_ERR_SET_ERRNO(KR_MEM_ERR_LESS);	
		acoral_intr_enable();
    	return (void *)NULL;            /* Not found */
	}

	tlsf_mem_extractblock_hdr(b, tlsf, fl, sl);

	next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);

	tmp_size = (b->size & BLOCK_SIZE) - size;
	if (tmp_size >= sizeof(BHDR))		
	{
		tmp_size -= BHDR_OVERHEAD;
		b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
		b2->size = tmp_size | FREE_BLOCK | PREV_USED;
		next_b->prev_hdr = b2;
		tlsf_mapping_insert(tmp_size, &fl, &sl);
		tlsf_mem_insertblock(b2, tlsf, fl, sl);

		b->size = size | (b->size & PREV_STATE);
	}
	else								
	{
		next_b->size &= (acoral_u32)(~PREV_FREE);
		b->size &= (acoral_u32)(~FREE_BLOCK);
	}			
	acoral_intr_enable();
	return (void *) b->ptr.buffer;
}

/**
 *@brief 重新分配内存块。
 * 
 *@param ptr 指向原有内存块的指针。
 *@param size 请求分配的内存大小。
 *@return 指向重新分配的内存块的指针（void *），如果分配失败则返回 NULL。
 */

void * tlsf_mem_realloc(void *ptr,acoral_u32 size)
{
    void *reptr;
    if(ptr == NULL)
    {
        reptr=tlsf_mem_alloc(size);
        return reptr;
    }
    if(((*(acoral_u32 *)((acoral_u32)ptr-4))&0x1)==0x1)
    {
        return NULL;
    }
    if(size == 0)
    {
        tlsf_mem_free(ptr);
        return NULL;
    }
    if(*(acoral_u32 *)((acoral_u32)ptr-4)>=size)
    {
    	
        return ptr;
    }
    else
    {
        reptr=tlsf_mem_alloc(size);
        if(reptr == NULL)
        {
            return NULL;
        }
        else
        {
            tlsf_mem_cpy(reptr,ptr,size);
	    	tlsf_mem_free(ptr);
            return reptr;
        }
    }
}
/**
 *@brief 释放内存块并将其归还到内存池中。
 * 
 *@param ptr 指向要释放的内存块的指针。
 *@return STATUS 类型，表示释放操作是否成功。
 */
STATUS tlsf_mem_free(void *ptr)
{
	TLSF *tlsf;
	BHDR *b, *tmp_b;
	acoral_u32 fl=0,sl=0;

	tlsf = (TLSF *) tlsf_mempool_addr_g;
	//#ifdef __INT_OK__
	// oldlevel = Bsp_DisableInt(ALL_acoral_uSR_ISR);
	acoral_intr_disable();	 	/*关中断*/
	//#endif
	if (ptr == (void *)NULL)
	{
		ACORAL_ERR_SET_ERRNO(KR_MEM_ERR_MALLOC);				
		acoral_intr_enable();	/*开中断*/
        return OSERROR;
    }
	
    b = (BHDR *) ((acoral_8 *) ptr - BHDR_OVERHEAD);
    b->size |= FREE_BLOCK;
    b->ptr.free_ptr.prev = NULL;
    b->ptr.free_ptr.next = NULL;	
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
	if ((tmp_b->size & FREE_BLOCK) != 0)
	{
        tlsf_mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);	
        tlsf_mem_extractblock(tmp_b, tlsf, fl, sl);
        b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;

    }

    if ((b->size & PREV_FREE) != 0)
	{
        tmp_b = b->prev_hdr;
        tlsf_mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);	
        tlsf_mem_extractblock(tmp_b, tlsf, fl, sl);
        tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        b = tmp_b;
    }

	
    tlsf_mapping_insert(b->size & BLOCK_SIZE, &fl, &sl);

    tlsf_mem_insertblock(b, tlsf, fl, sl);
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_b->size |= PREV_FREE;
    tmp_b->prev_hdr = b;

	acoral_intr_enable();
    return OSOK;
}

/**
 *@brief 将数据从源地址（from）复制到目标地址（to）。
 * 
 *@param to 目标地址。
 *@param from 源地址。
  *@param size 要复制的字节数。
 *@return 返回目标地址（to）。
 */
void * tlsf_mem_cpy(void * to, const void * from, acoral_u32 size)
{
   acoral_u8  * dstend;
   acoral_u8  * source;
   acoral_u8  * destination;
   acoral_u32 * src;
   acoral_u32 * dst;
   acoral_32  tmp;

	source = (acoral_u8 *)from;
	destination = (acoral_u8 *)to;
	tmp = (acoral_32)(destination - source);
    if ((tmp <= 0) || ((acoral_u32)tmp >= size))
	{
		dstend = destination + size;
		/* do byte copy if less than ten or alignment mismatch */
		if (!((size < 10) || (((acoral_u32)destination ^ (acoral_u32)source) & 0x3)))
		{
			/* if odd-aligned copy byte */
			while ((acoral_u32)destination & 0x3)
			{
			    *destination++ = *source++;
			}

			src = (acoral_u32 *) source;
			dst = (acoral_u32 *) destination;

			do{
			    *dst++ = *src++;
			}
			while (((acoral_u8 *)dst + sizeof (acoral_u32)) <= dstend);

			destination = (acoral_u8 *)dst;
			source      = (acoral_u8 *)src;
		}

		while (destination < dstend)
		{
		    *destination++ = *source++;
		}
	}
    else
	{
		/* backward copy */
		dstend       = destination;
		destination += size - sizeof (acoral_8);
		source      += size - sizeof (acoral_8);

		/* do byte copy if less than ten or alignment mismatch */
		if (!((size < 10) || (((acoral_u32)destination ^ (acoral_u32)source) & 0x3)))
								
		{

			/* if odd-aligned copy byte */
			while (((acoral_u32)destination+1) & 0x3)
			{
			    *destination-- = *source--;
			}

			src = (acoral_u32 *)((acoral_u8 *)source-3);		
			dst = (acoral_u32 *)((acoral_u8*) destination-3); 

			do
		    {
			    *dst-- = *src--;
		    }
			while ((acoral_u8 *)dst  >= dstend);  

			destination = (acoral_u8 *)dst + sizeof (acoral_u32)-1;  
			source      = (acoral_u8 *)src + sizeof (acoral_u32)-1;
		}

		while (destination >= dstend)
		{
		    *destination-- = *source--;
		}
	}

	return to;
}

/**
 *@brief 将目标内存区域（dst）的每个字节设置为指定的值（val）。
 * 
 *@param dst 目标内存区域的起始地址。
 *@param val 要设置的值（acoral_u8 类型）。
 *@param count 要设置的字节数。
 *@return 返回目标内存区域的起始地址（dst）。
 */
void *tlsf_mem_set(void *dst,acoral_u8 val, acoral_u32 count)
{
	acoral_u8 *dstend;
	acoral_u8 *u8dst;
	acoral_u32 *u32dst;
	acoral_u32 tmpdata;
	
	u8dst = (acoral_u8 *)dst;
	dstend = u8dst + count;
	
	if(count > 10)
	{

		while( (acoral_u32)u8dst & 0x3 )
		{
			*u8dst++ = val;
		}
		
		u32dst = (acoral_u32 *)u8dst;
		tmpdata = ((acoral_u32)val << 24) + ((acoral_u32)val << 16) + ((acoral_u32)val << 8) + (acoral_u32)val;
		do
		{
			*u32dst++ = tmpdata;
		}
		while( ((acoral_u8 *)u32dst + sizeof(acoral_u32)) <= dstend );
		u8dst = (acoral_u8 *)u32dst;
	}
	
	while(u8dst < dstend)
	{
		*u8dst++ = val;
	}
	
	return dst;
}
