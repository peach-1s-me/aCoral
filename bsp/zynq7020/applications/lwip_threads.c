/**
 * @file lwip_threads.c
 * @author 文佳源 (648137125@qq.com)
 * @brief lwip以太网server&client的demo
 * @version 0.1
 * @date 2024-08-10
 *
 * Copyright (c) 2024
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源        <td>2024-08-10 <td>tcp服务端应用demo
 * <tr><td>v2.0 <td>文佳源 饶洪江 <td>2024-12-30 <td>增加客户端代码,代码结构调整
 * </table>
 */
#if 0 /* 在测试lwip_tcp_client.c和lwip_test_client.c时不启用该代码 */
#include <acoral.h>

#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#if defined(__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "lwip/tcp.h"
#include "xil_cache.h"

#if LWIP_IPV6 == 1
#include "lwip/ip.h"
#else
#if LWIP_DHCP == 1
#include "lwip/dhcp.h"
#endif
#endif

/***
 * 配置流程:
 * 1.修改mac地址使每个板子mac不同
 * 2.根据是服务器还是客户端修改 IS_SERVER
 * 3.根据服务器地址和客户端地址修改 IP 和端口
 *  */

#define IS_SERVER 0 /**< 设置为1启动服务器程序, 为0启动客户端 */

/* 服务端IP */
#define SERVER_IP0 192
#define SERVER_IP1 168
#define SERVER_IP2 1
#define SERVER_IP3 3
#define SERVER_PORT 7

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

struct tcp_pcb *client_pcb; /**< 作为客户端的pcb */

/* missing declaration in lwIP */
void lwip_init();

void tcp_fasttmr(void);
void tcp_slowtmr(void);

acoral_32 lwip_deamon_init(void);

err_t server_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
int start_server_application();

err_t client_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t tcp_connect_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
int client_transfer_data(struct tcp_pcb *pcb);
int start_client_application(void);
static void connect_err_callback(void *arg, err_t err);

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

/***** 1 服务器部分 ******/

/**
 * @brief 服务器接收数据回调函数
 * 
 * @param  arg              参数(未使用)
 * @param  tpcb             建立连接的协议控制块指针
 * @param  p                收到数据的缓冲区指针
 * @param  err              如果已经收到一个错误该变量会被赋值
 * @return err_t            ERR_OK: 成功
 *                          else  : 失败
 */
err_t server_recv_callback(void *arg, struct tcp_pcb *tpcb,
                           struct pbuf *p, err_t err)
{
    /* do not read the packet if we are not in ESTABLISHED state */
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    /* indicate that the packet has been received */
    tcp_recved(tpcb, p->len);

    /* echo back the payload */
    /* in this case, we assume that the payload is < TCP_SND_BUF */
    if (tcp_sndbuf(tpcb) > p->len)
    {
        char buf[512];
        MEMCPY(buf, p->payload, p->len);
        buf[p->len] = '\0';
        xil_printf("rcv:\t%s\r\n", buf);
        err = tcp_write(tpcb, p->payload, p->len, 1);
    }
    else
        xil_printf("no space in tcp_sndbuf\n\r");

    /* free the received pbuf */
    pbuf_free(p);

    return ERR_OK;
}

/**
 * @brief accept回调函数
 * 
 * @param  arg              参数
 * @param  newpcb           建立的新连接pcb指针
 * @param  err              如果已经收到一个错误该变量会被赋值
 * @return err_t            ERR_OK: 成功
 *                          else  : 失败
 */
err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    static int connection = 1;

    xil_printf("client (%d.%d.%d.%d) accepted\r\n", ip4_addr1(&(newpcb->remote_ip)), ip4_addr2(&(newpcb->remote_ip)), ip4_addr3(&(newpcb->remote_ip)), ip4_addr4(&(newpcb->remote_ip)));

    /* set the receive callback for this connection */
    tcp_recv(newpcb, server_recv_callback);

    /* just use an integer number indicating the connection id as the
       callback argument */
    tcp_arg(newpcb, (void *)(UINTPTR)connection);

    /* increment for subsequent accepted connections */
    connection++;

    return ERR_OK;
}

/**
 * @brief 初始化服务器应用
 * 
 * @return int 0    : 成功
 *             else : 失败
 */
int start_server_application()
{
    struct tcp_pcb *pcb;
    err_t err;

    /* create new TCP PCB structure */
    pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (NULL == pcb)
    {
        xil_printf("Error creating PCB. Out of Memory\n\r");
        return -1;
    }

    /* bind to specified @port */
    err = tcp_bind(pcb, IP_ANY_TYPE, 7);
    if (err != ERR_OK)
    {
        xil_printf("Unable to bind to port %d: err = %d\n\r", 7, err);
        return -2;
    }

    /* we do not need any arguments to callback functions */
    tcp_arg(pcb, NULL);

    /* listen for connections */
    pcb = tcp_listen(pcb);
    if (!pcb)
    {
        xil_printf("Out of memory while tcp_listen\n\r");
        return -3;
    }

    /* specify callback to use for incoming connections */
    tcp_accept(pcb, accept_callback);

    xil_printf("TCP echo server started @ port %d\n\r", 7);

    return 0;
}

/***** 2 客户端部分 ******/
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
    /* do not read the packet if we are not in ESTABLISHED state */
    if (NULL == p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    /* indicate that the packet has been received */
    tcp_recved(tpcb, p->len);

    /* echo back the payload */
    /* in this case, we assume that the payload is < TCP_SND_BUF */
    if (tcp_sndbuf(tpcb) > p->len)
    {
        char buf[512];
        MEMCPY(buf, p->payload, p->len);
        buf[p->len] = '\0';
        xil_printf("rcv:\t%s\r\n", buf);
    }
    else
        xil_printf("no space in tcp_sndbuf\n\r");

    /* free the received pbuf */
    pbuf_free(p);

    return ERR_OK;
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
err_t tcp_connect_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    /* set the receive callback for this connection */
    tcp_recv(newpcb, client_recv_callback);

    xil_printf("connect to server\r\n");

    char buffer[40];
    sprintf(buffer, "greeting");
    tcp_write(newpcb, buffer, strlen(buffer), 1);

    return ERR_OK;
}

/**
 * @brief 向server发送数据
 *        例子:发送一个随次数递增的count
 *
 * @param  pcb              tcp协议控制块指针
 * @return int              0:    成功
 *                          else: 失败
 */
int client_transfer_data(struct tcp_pcb *pcb)
{
    static int count = 0;
    char buffer[40];

    if (NULL == pcb)
    {
        return -1;
    }
    sprintf(buffer, "The sending count is [%d]", count++);

    tcp_write(pcb, buffer, strlen(buffer), 1);

    xil_printf("snd:\t%s\r\n", buffer);

    return 0;
}

/**
 * @brief 启动客户端应用
 * 
 * @param  ppcb             建立连接的pcb指针变量地址(二级指针)
 * @return int              0    : 成功
 *                          else : 失败
 */
int start_client_application(void)
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
    err = tcp_connect(pcb, &ipaddr, 7, tcp_connect_callback);
    while(err != ERR_OK)
    {
#if 1
        xil_printf("[client] fail to connect to server, try to reconncect\r\n");
        acoral_delay_ms(1000);
        err = tcp_connect(pcb, &ipaddr, 7, tcp_connect_callback);
#endif
    }

    tcp_err(pcb, connect_err_callback);

    client_pcb = pcb;

    return 0;
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

/* 3 主程序部分 */

#if LWIP_IPV6 == 0
#if LWIP_DHCP == 1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

#if LWIP_IPV6 == 1
void print_ip6(char *msg, ip_addr_t *ip)
{
    print(msg);
    xil_printf(" %x:%x:%x:%x:%x:%x:%x:%x\n\r",
               IP6_ADDR_BLOCK1(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK2(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK3(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK4(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK5(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK6(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK7(&ip->u_addr.ip6),
               IP6_ADDR_BLOCK8(&ip->u_addr.ip6));
}
#else
void print_ip(char *msg, ip_addr_t *ip)
{
    print(msg);
    xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
               ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

    print_ip("Board IP: ", ip);
    print_ip("Netmask : ", mask);
    print_ip("Gateway : ", gw);
}
#endif

void lwip_main_thread_entry(void *args)
{
#if LWIP_IPV6 == 0
    ip_addr_t ipaddr, netmask, gw;

#endif
    /* the mac address of the board. this should be unique per board */
    /* ! 确保每块开发板拥有独立的mac地址 */
    unsigned char mac_ethernet_address[] =
        {0x00, 0x0a, 0x35, 0x00, 0x01, 0x03};

    echo_netif = &server_netif;

    init_platform();

#if LWIP_IPV6 == 0
#if LWIP_DHCP == 1
    ipaddr.addr = 0;
    gw.addr = 0;
    netmask.addr = 0;
#else
    /* initliaze IP addresses to be used */
#if IS_SERVER
    IP4_ADDR(&ipaddr, SERVER_IP0, SERVER_IP1, SERVER_IP2, SERVER_IP3);
#else
    IP4_ADDR(&ipaddr, CLIENT_IP0, CLIENT_IP1, CLIENT_IP2, CLIENT_IP3);
#endif
    IP4_ADDR(&netmask, NETMASK0, NETMASK1, NETMASK2, NETMASK3);
    IP4_ADDR(&gw, GATEWAY_IP0, GATEWAY_IP1, GATEWAY_IP2, GATEWAY_IP3);
#endif
#endif

    /* lwip初始化 */
    lwip_init();

    /* 网卡初始化 */
#if (LWIP_IPV6 == 0)
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(echo_netif, &ipaddr, &netmask,
                   &gw, mac_ethernet_address,
                   PLATFORM_EMAC_BASEADDR))
    {
        xil_printf("Error adding N/W interface\n\r");
        return -1;
    }
#else
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(echo_netif, NULL, NULL, NULL, mac_ethernet_address,
                   PLATFORM_EMAC_BASEADDR))
    {
        xil_printf("Error adding N/W interface\n\r");
        return -1;
    }
    echo_netif->ip6_autoconfig_enabled = 1;

    netif_create_ip6_linklocal_address(echo_netif, 1);
    netif_ip6_addr_set_state(echo_netif, 0, IP6_ADDR_VALID);

    print_ip6("\n\rBoard IPv6 address ", &echo_netif->ip6_addr[0].u_addr.ip6);

#endif
    netif_set_default(echo_netif);

    /* now enable interrupts */
    // platform_enable_interrupts();
    /* 初始化守护线程 */
    lwip_deamon_init();

    /* specify that the network if is up */
    netif_set_up(echo_netif);

#if (LWIP_IPV6 == 0)
#if (LWIP_DHCP == 1)
    /* Create a new DHCP client for this interface.
     * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
     * the predefined regular intervals after starting the client.
     */
    dhcp_start(echo_netif);
    dhcp_timoutcntr = 24;

    while (((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
        xemacif_input(echo_netif);

    if (dhcp_timoutcntr <= 0)
    {
        if ((echo_netif->ip_addr.addr) == 0)
        {
            xil_printf("DHCP Timeout\r\n");
            xil_printf("Configuring default IP of 192.168.1.10\r\n");
            IP4_ADDR(&(echo_netif->ip_addr), SERVER_IP0, SERVER_IP1, SERVER_IP2, SERVER_IP3);
            IP4_ADDR(&(echo_netif->netmask), NETMASK0, NETMASK1, NETMASK2, NETMASK3);
            IP4_ADDR(&(echo_netif->gw), GATEWAY_IP0, GATEWAY_IP1, GATEWAY_IP2, GATEWAY_IP3);
        }
    }

    ipaddr.addr = echo_netif->ip_addr.addr;
    gw.addr = echo_netif->gw.addr;
    netmask.addr = echo_netif->netmask.addr;
#endif

    print_ip_settings(&ipaddr, &netmask, &gw);

#endif

/* 这里进入循环时分服务器和客户端 */
#if IS_SERVER
    /* start the application (web server, rxtest, txtest, etc..) */
    start_server_application();

    /* receive and process packets */
    while (1)
    {
        if (TcpFastTmrFlag)
        {
            tcp_fasttmr();
            TcpFastTmrFlag = 0;
        }
        if (TcpSlowTmrFlag)
        {
            tcp_slowtmr();
            TcpSlowTmrFlag = 0;
        }
        xemacif_input(echo_netif);
    }
#else
    /* start the application (web server, rxtest, txtest, etc..) */
    start_client_application();
    /* 这里client_pcb在返回时会被赋值为建立连接的pcb,用来发送数据 */

    /* receive and process packets */
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

    /* never reached */
    cleanup_platform();
}

static struct netif server_netif;
struct netif *echo_netif;
/**
 * @brief 初始化守护线程
 * 
 * @return acoral_32 0   :成功
 *                   else:这里出错直接assert了,不会返回
 */
acoral_32 lwip_deamon_init(void)
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

/**
 * @brief 初始化lwip线程
 * 
 * @return acoral_32 0   :成功
 *                   else:这里出错直接assert了,不会返回
 */
acoral_32 lwip_app_thread_init(void)
{
    acoral_32 err = -1;

    acoral_print("[lwip_app_thread_init] create lwip thread\r\n");
    acoral_comm_policy_data_t p_data;
    p_data.cpu = 0;  /* 指定运行的cpu */
    p_data.prio = 3; /* 指定优先级 */
    acoral_id jr_id = acoral_create_thread(
        lwip_main_thread_entry,
        4096,
        NULL,
        "lwip_main",
        NULL,
        ACORAL_SCHED_POLICY_COMM,
        &p_data,
        NULL,
        NULL);
    if (jr_id == -1)
    {
        while (1)
            ;
    }
}
#endif
