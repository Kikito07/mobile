#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include <stdlib.h>
#include "packet.h"
#ifndef DEBUG
#define DEBUG DEBUG_FULL
#endif

#define USE_RPL_CLASSIC 0

#include "net/ipv6/uip-debug.h"
#include "dev/watchdog.h"

#if USE_RPL_CLASSIC
#include "net/routing/rpl-classic/rpl-dag-root.h"
#else
#include "net/routing/rpl-lite/rpl-dag-root.h"
#endif

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

#define SERVER_REPLY 1

/* Should we act as RPL root? */
#define SERVER_RPL_ROOT 0

#if SERVER_RPL_ROOT
static uip_ipaddr_t ipaddr;
#endif

/*---------------------------------------------------------------------------*/

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

/*---------------------------------------------------------------------------*/

static void
handle_packet()
{
    pkt_t pkt;
    size_t size = 3;
    if (PKT_OK != pkt_decode(buf, size, &pkt))
    {
        printf("packet received but encode fail");
    }
    if (pkt_get_code(&pkt) == PTYPE_POST)
    {
        const char *payload = pkt_get_payload(&pkt);
        post_types_t post_type = payload[0];
        if (post_type == PTYPE_LIGHT_ON)
        {
            leds_on(LEDS_RED);
        }
        if (post_type == PTYPE_LIGHT_OFF)
        {
            leds_off(LEDS_RED);
        }
    }
    pkt_set_code(&pkt, PTYPE_ACK);
    pkt_encode(&pkt, buf);
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;
    uip_udp_packet_send(server_conn, buf, len);
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
}

static void
tcpip_handler(void)
{
    memset(buf, 0, MAX_PAYLOAD_LEN);
    if (uip_newdata())
    {
        len = uip_datalen();
        memcpy(buf, uip_appdata, len);
        PRINTF("%u bytes from [", len);
        PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
        PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
        handle_packet();
        printf("packet : %u", *buf);
    #if SERVER_REPLY
        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
        server_conn->rport = UIP_UDP_BUF->srcport;
        uip_udp_packet_send(server_conn, buf, len);
        uip_create_unspecified(&server_conn->ripaddr);
        server_conn->rport = 0;
    #endif
    }
    return;
}

/*------------------------------------------------------------------------------------------------------ */

/*---------------------------------------------------------------------------*/

#if SERVER_RPL_ROOT
static void
print_local_addresses(void)
{
    int i;
    uint8_t state;

    PRINTF("Server IPv6 addresses:\n");
    for (i = 0; i < UIP_DS6_ADDR_NB; i++)
    {
        state = uip_ds6_if.addr_list[i].state;
        if (uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED))
        {
            PRINTF("  ");
            PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
            PRINTF("\n");
            if (state == ADDR_TENTATIVE)
            {
                uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void create_dag()
{

    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

    print_local_addresses();

    rpl_dag_root_set_prefix(NULL, NULL);
    int tmp = rpl_dag_root_start();

#if !USE_RPL_CLASSIC
    if (tmp == 0)
    {
        PRINTF("Server set as ROOT in the DAG\n");
    }
    else
    {
        PRINTF("RIP\n");
    }
#else
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,
                       &uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
    if (dag != NULL)
    {
        uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
        rpl_set_prefix(dag, &ipaddr, 64);
        PRINTF("Created a new RPL dag with ID: ");
        PRINT6ADDR(&dag->dag_id);
        PRINTF("\n");
    }
#endif
}
#endif /* SERVER_RPL_ROOT */

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_server_process, ev, data)
{
    PROCESS_BEGIN();
    PRINTF("Starting the server\n");
    
    // leds_toogle(LEDS_BLUE);

#if SERVER_RPL_ROOT
    create_dag();
#endif



    
    
    
    // PROCESS_BEGIN();
    // while(1){
    //     
    // }
    // PROCESS_END();

    server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));
    
    
    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);
    PRINTF("hello\n");

    while (1)
    {
        PROCESS_YIELD();
        if (ev == tcpip_event)
        {
            // PRINTF("hello\n");
            // char msg[] = "hello guys\n";
            // uint16_t msglen = sizeof(msg);
            // uip_ipaddr_t ipaddr;
            // uip_ip6addr(&ipaddr,0xBBBB,0,0,0,0,0,0,0x1);
            // PRINT6ADDR(&ipaddr);
            // PRINTF("\n");
            // server_conn = udp_new(&ipaddr, UIP_HTONS(37600), NULL);
            // uip_udp_packet_send(server_conn, msg, msglen);
            // uip_create_unspecified(&server_conn->ripaddr);
            // server_conn->rport = 0;
            tcpip_handler();
        }
    }

    PROCESS_END();
}
