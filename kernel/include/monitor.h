#ifndef __KERNEL_MONITOR_H__
#define __KERNEL_MONITOR_H__

#include "acoral.h"

#define MONITOR_CMD_CREATE      0x01
#define MONITOR_CMD_SWITCH      (MONITOR_CMD_CREATE << 1)
//按对齐标准 不然会出问题
//#pragma pack(8)

// CPU核心信息
typedef struct  {
    acoral_u32 id;
    acoral_u32 cpuFrequency;
    char name[32];
} CPUcore_info;

typedef struct {
    acoral_u32   thread_policy;
    acoral_u32   prio;
//    acoral_u32   placeholders[2];
    acoral_thread_t  *tcb_ptr;
//    acoral_u32  placeholder_u32;  //反正不加这个垫子 不按对齐就有问题
    acoral_u32  period;
    acoral_u32  supercycle;
    acoral_u32  cpu;
    acoral_u32  create_tick;
    acoral_char thread_name[32];

} thread_info;


//#pragma pack()
typedef struct {
    acoral_u32 cpu;
    acoral_u32 tick;
    acoral_thread_t *from_tcb;
    acoral_thread_t *to_tcb;
//    acoral_char from_name[32];
//    acoral_char to_name[32];
} thread_switch_info; // 每个40 Byte


typedef struct {
    acoral_u32 total_size;
    acoral_u32 st_idx;
    acoral_u32 ed_idx;
    acoral_u32 info_type;
    acoral_u32 size;
    acoral_u32 item_bsize;
    acoral_u32 overflow;
    thread_info threads[CFG_MAX_THREAD];  // 这个每次发送的数组长度都是该结构体的size字段
} thread_info_set;       // 占用3kb以上大小


typedef struct {
    acoral_u32 total_size;
    acoral_u32 threshold; //取一半
    acoral_u32 st_idx;
    acoral_u32 ed_idx;
    acoral_u32 info_type;
    acoral_u32 size;
    acoral_u32 item_bsize;
    acoral_u32 overflow;
    thread_switch_info switch_infos[CFG_MAX_THREAD];   // 这个每次发送的数组长度都是该结构体的size字段
} thread_switch_info_set; //占用7kb以上大小

void acoral_monitor_init(void);

void acoral_tinfos_add(acoral_thread_t* thread);
void acoral_tswinfos_add(acoral_thread_t* from, acoral_thread_t* to);

void acoral_infos_send();
#endif
