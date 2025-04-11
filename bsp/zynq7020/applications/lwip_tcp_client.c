/**
 * @file lwip_tcp_client.c
 * @author 文佳源 (648137125@qq.com)
 * @brief lwip的tcp客户端用户数据传输接口
 * @version 1.1
 * @date 2025-03-27
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-02-25 <td>内容
 * <tr><td>v1.1 <td>饶洪江 <td>2025-03-27 <td>消除warning
 * </table>
 */
#include "acoral.h"
#include "ringbuffer.h"


/* 平台相关 */
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"

/* lwip */
#include "lwip/init.h"
#include "lwip/tcp.h"

/* 要连接的远程服务端IP */
#define SERVER_IP0 192
#define SERVER_IP1 168
#define SERVER_IP2 1
#define SERVER_IP3 3
#define SERVER_PORT 8888

/* 客户端IP */
#define CLIENT_IP0 192
#define CLIENT_IP1 168
#define CLIENT_IP2 1
#define CLIENT_IP3 11 /* 不同的客户端设置不同的IP地址 */
#define CLIENT_PORT 8

/* 网关IP */
#define GATEWAY_IP0 192
#define GATEWAY_IP1 168
#define GATEWAY_IP2 1
#define GATEWAY_IP3 1

/* 子网掩码 */
#define NETMASK0 255
#define NETMASK1 255
#define NETMASK2 255
#define NETMASK3 0

/* 接收环形缓冲区大小 */
#define RCV_RB_SIZE 4096

void tcp_fasttmr(void);
void tcp_slowtmr(void);

struct tcp_pcb *client_pcb; /**< 作为客户端的pcb */
struct netif *echo_netif;/* platform_zynq.c要用 */
acoral_u8 is_connected_to_server = 0; /* 已连接上服务器 */

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

static void _lwip_client_init(void *args);
static void _lwip_client_loop_thread(void *args);
static acoral_32 lwip_deamon_init(void);
static void lwip_deamon_entry(void *args);
static void print_ip(char *msg, ip_addr_t *ip);
static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
static int start_client_background(void);
static err_t tcp_connect_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void connect_err_callback(void *arg, err_t err);
err_t client_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

static struct netif client_netif;

static ringbuffer_t lwip_tcp_client_rb;
static acoral_u8 lwip_tcp_client_rb_space[RCV_RB_SIZE];

void lwip_client_init(void)
{
    acoral_print("[lwip_app_thread_init] create lwip thread\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu = 0;  /* 指定运行的cpu */
    p_data.prio = 2; /* 指定优先级 */
    acoral_id lc_id = acoral_create_thread(
        _lwip_client_init,
        4096,
        NULL,
        "lwip_client_init",
        NULL,
        ACORAL_SCHED_POLICY_COMM,
        &p_data,
        NULL,
        NULL);
    if (lc_id == -1)
    {
        while (1)
            ;
    }
}

static void _lwip_client_init(void *args)
{
    ip_addr_t ipaddr, netmask, gw;

    /* ! 确保每块开发板拥有独立的mac地址 */
    unsigned char mac_ethernet_address[] =
        {0x00, 0x0a, 0x35, 0x00, 0x01, 0x03};

    echo_netif = &client_netif;

    /* 初始化平台 */
    init_platform();

    /* 指定ip */
    IP4_ADDR(&ipaddr, CLIENT_IP0, CLIENT_IP1, CLIENT_IP2, CLIENT_IP3);
    IP4_ADDR(&netmask, NETMASK0, NETMASK1, NETMASK2, NETMASK3);
    IP4_ADDR(&gw, GATEWAY_IP0, GATEWAY_IP1, GATEWAY_IP2, GATEWAY_IP3);

    /* lwip初始化 */
    lwip_init();

    /* 添加网卡 */
    if (!xemac_add(echo_netif, &ipaddr, &netmask,
                   &gw, mac_ethernet_address,
                   PLATFORM_EMAC_BASEADDR))
    {
        xil_printf("Error adding N/W interface\n\r");
        return;
    }

    /* 设为默认网络接口 */
    netif_set_default(echo_netif);

    /* 初始化守护线程 */
    lwip_deamon_init();

    /* 将网口状态设定为启动 */
    netif_set_up(echo_netif);

    /* 打印ip信息 */
    print_ip_settings(&ipaddr, &netmask, &gw);

    /* 初始化环形缓冲区 */
    ringbuffer_init(&lwip_tcp_client_rb, lwip_tcp_client_rb_space, RCV_RB_SIZE);

    start_client_background();

#if 1
    acoral_print("[_lwip_app_client_init] create lwip loop\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu = 1;  /* 指定运行的cpu */
    p_data.prio = 4; /* 指定优先级 !比应用的优先级低 */
    acoral_id lc_id = acoral_create_thread(
        _lwip_client_loop_thread,
        4096,
        NULL,
        "lwip_client_loop",
        NULL,
        ACORAL_SCHED_POLICY_COMM,
        &p_data,
        NULL,
        NULL);
    if (lc_id == -1)
    {
        while (1)
            ;
    }
#endif

#if 0
    while (1)
    {
        static int cnt = 0;
        if (TcpFastTmrFlag)
        {
            tcp_fasttmr();
            TcpFastTmrFlag = 0;
        }
        if (TcpSlowTmrFlag)
        {
            tcp_slowtmr();
            TcpSlowTmrFlag = 0;
            cnt++;
        }
        xemacif_input(echo_netif);

        if (cnt == 2)
        {
            client_transfer_data(client_pcb);
            cnt = 0;
        }
    }
#endif
}

static void _lwip_client_loop_thread(void *args)
{
    while (1)
    {
        static int cnt = 0;
        if (TcpFastTmrFlag)
        {
            tcp_fasttmr();
            TcpFastTmrFlag = 0;
        }
        if (TcpSlowTmrFlag)
        {
            tcp_slowtmr();
            TcpSlowTmrFlag = 0;
            cnt++;
        }
        xemacif_input(echo_netif);
    }
}

/**
 * @brief 客户端tcp写
 * 
 * @param  buf              待写的数据缓冲区
 * @param  len              要写的长度
 * @param  errcode          错误代码,
 *                          0:成功
 *                          else:失败
 * @return size_t           实际写的长度,
 *                          len:成功
 *                          else:失败
 */
size_t lwip_client_write(
    const acoral_u8 *buf,
    size_t len,
    acoral_u8 *errcode
)
{
    size_t real_write_len = -1;
    u8_t err = (acoral_u8)tcp_write(client_pcb, buf, len, 1);

    *errcode = (acoral_u8)err;

    if(0 == err)
    {
        real_write_len = len;
    }

    return real_write_len;
}

size_t lwip_client_read(
    acoral_u8 *buf,
    size_t len,
    acoral_32 timeout,
    acoral_u8 *errcode
)
{
#if 0
    acoral_u32 result = ringbuffer_get_more(&lwip_tcp_client_rb, buf, len);

    if(result != len)
    {
        *errcode = ERR_MEM;
    }
    else
    {
        *errcode = ERR_OK;
    }

    return result;
#else
    size_t wrote = 0;
    acoral_u32 starttime_ms = acoral_get_ticks(); /* 如果系统时钟频率调整需要修改此处 */

    while(
        (wrote < len) &&
        ((acoral_get_ticks() - starttime_ms) <= timeout)
    )
    {   
        acoral_u32 result = ringbuffer_get(&lwip_tcp_client_rb, buf+wrote);

        if(result != len)
        {
            *errcode = ERR_MEM;
            break;
        }

        wrote++;
    }

    return wrote;
#endif
}

/**
 * @brief 初始化守护线程
 * 
 * @return acoral_32 0   :成功
 *                   else:这里出错直接assert了,不会返回
 */
static acoral_32 lwip_deamon_init(void)
{
    acoral_32 ret = 0;

    acoral_print("[lwip_deamon_init] create deamon\r\n");
    acoral_period_policy_data_t *period_policy_data = (acoral_period_policy_data_t *)acoral_vol_malloc(sizeof(acoral_period_policy_data_t));
    // func 2
    period_policy_data->prio = 2;
    period_policy_data->cpu = 1;
    period_policy_data->time = 250; // 周期单位 ms
    acoral_id ld_id = acoral_create_thread(lwip_deamon_entry,
                                           4096,
                                           NULL,
                                           "lwip_deamon",
                                           NULL,
                                           ACORAL_SCHED_POLICY_PERIOD,
                                           period_policy_data,
                                           NULL,
                                           NULL);
    if (ld_id == -1)
    {
        while (1)
            ;
    }

    return ret;
}

void timer_callback(void);

/**
 * @brief lwip守护线程入口
 *        主要是调用lwip时钟回调
 *
 * @param  args             未使用
 */
static void lwip_deamon_entry(void *args)
{
    timer_callback();
}

static void print_ip(char *msg, ip_addr_t *ip)
{
    print(msg);
    xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
               ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

    print_ip("Board IP: ", ip);
    print_ip("Netmask : ", mask);
    print_ip("Gateway : ", gw);
}

/**
 * @brief 启动客户端后台程序
 * 
 * @param  ppcb             建立连接的pcb指针变量地址(二级指针)
 * @return int              0    : 成功
 *                          else : 失败
 */
static int start_client_background(void)
{
    struct tcp_pcb *pcb;
    err_t err;
    ip_addr_t ipaddr;

    /* 将目标服务器的IP写入一个结构体，为pc机本地连接IP地址 */
    IP4_ADDR(&ipaddr, SERVER_IP0, SERVER_IP1, SERVER_IP2, SERVER_IP3);

    /* create new TCP PCB structure */
    pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (NULL == pcb)
    {
        xil_printf("Error creating PCB. Out of Memory\n\r");
        return -1;
    }

    /* bind to specified @port */
    err = tcp_bind(pcb, IP_ANY_TYPE, CLIENT_PORT);
    if (err != ERR_OK)
    {
        xil_printf("Unable to bind to port %d: err = %d\n\r", CLIENT_PORT, err);
        return -2;
    }

    /* we do not need any arguments to callback functions */
    tcp_arg(pcb, NULL);

    //	/* listen for connections */
    if (NULL == pcb)
    {
        xil_printf("Out of memory while tcp_listen\n\r");
        return -3;
    }

    xil_printf("tcp client started\r\n");

    /* 指定连接成功回调函数 */
    // acoral_delay_ms(20000);
    err = tcp_connect(pcb, &ipaddr, SERVER_PORT, tcp_connect_callback);
    while(err != ERR_OK)
    {
#if 1
        xil_printf("[client] fail to connect to server, try to reconncect\r\n");
        acoral_delay_ms(1000);
        err = tcp_connect(pcb, &ipaddr, SERVER_PORT, tcp_connect_callback);
#endif
    }

    tcp_err(pcb, connect_err_callback);

    client_pcb = pcb;

    return 0;
}

/**
 * @brief 连接到服务器(connect)操作的回调函数
 * 
 * @param  arg              参数
 * @param  newpcb           建立的新连接pcb指针
 * @param  err              如果已经收到一个错误该变量会被赋值
 * @return err_t            ERR_OK: 成功
 *                          else  : 失败
 */
static err_t tcp_connect_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    /* set the receive callback for this connection */
    tcp_recv(newpcb, client_recv_callback);

    is_connected_to_server = 1;

    xil_printf("connect to server\r\n");

#if 1
    char buffer[40];
    sprintf(buffer, "greeting");
    tcp_write(newpcb, buffer, strlen(buffer), 1);
#endif
    return ERR_OK;
}

static void connect_err_callback(void *arg, err_t err)
{
    xil_printf("[ERROR] fatal error accured\r\n");
    /* 重新初始化客户端应用 */
#if 0
    /* 这里重启会显示无法绑定同一个端口 */
    start_client_application();
#endif
}

/**
 * @brief 客户端接收数据回调函数
 * 
 * @param  arg              参数(未使用)
 * @param  tpcb             建立连接的协议控制块指针
 * @param  p                收到数据的缓冲区指针
 * @param  err              如果已经收到一个错误该变量会被赋值
 * @return err_t            ERR_OK: 成功
 *                          else  : 失败
 */
err_t client_recv_callback(void *arg, struct tcp_pcb *tpcb,
                           struct pbuf *p, err_t err)
{
    err_t ret = ERR_OK;

    /* do not read the packet if we are not in ESTABLISHED state */
    if (NULL == p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    /* indicate that the packet has been received */
    tcp_recved(tpcb, p->len);

    acoral_u8 buf[512];
    MEMCPY(buf, p->payload, p->len);

    acoral_u32 result = ringbuffer_put_more(&lwip_tcp_client_rb, buf, p->len);

    if(result != 0)
    {
        acoral_print("[ERROR] ringbuffer space not enough\r\n");

        ret = ERR_MEM;
    }

    /* free the received pbuf */
    pbuf_free(p);

    return ret;
}

