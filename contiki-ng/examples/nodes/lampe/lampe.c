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

// static uip_ipaddr_t ipServAddr;


/*---------------------------------------------------------------------------*/

PROCESS(udp_server_process, "UDP server process");
PROCESS(boot_process, "boot process");

AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static char buf_hello[MAX_PAYLOAD_LEN];
static uint16_t len;
static struct etimer timer;
static int helloDeviceDone = 0;
static uip_ipaddr_t ipaddr;
static size_t pkt_size = 5;
static uint8_t token = 0;
static pkt_t hello_pkt;

/*---------------------------------------------------------------------------*/

static void 
handle_packet()
{   
    pkt_t pkt;
    
    if (PKT_OK != pkt_decode(buf, &pkt))
    {
        PRINTF("packet received but encode fail");
    }
    PRINTF("code : %u\n",(uint8_t)pkt_get_code(&pkt));
    PRINTF("hello msg : %u\n",PCODE_HELLO );
    if(pkt_get_code(&pkt) == PCODE_HELLO){
        PRINTF("je suis rentre dans le premier if\n");
        if(helloDeviceDone == 0){
            PRINTF("inside token\n");
            token = *(pkt_get_payload(&pkt));
            PRINTF("token lamp : %u\n",token);
            pkt_set_token(&hello_pkt,token);
            pkt_encode(&hello_pkt,buf_hello);
            PRINTF("token apres encode : %u \n",pkt_get_token(&hello_pkt));

            helloDeviceDone = 1;


        }
    }

    if (pkt_get_code(&pkt) == PCODE_POST)
    {

        const char *payload = pkt_get_payload(&pkt);
        PRINTF("token : %u\n",pkt_get_token(&pkt));
        PRINTF("msgid : %u\n",pkt_get_msgid(&pkt));
        lamp_types_t post_type = payload[0];
        printf("post_type : %u\n",post_type);
        if (post_type == PTYPE_LIGHT_ON)
        {
            leds_on(LEDS_RED);
            uint8_t one = 1;
            pkt_set_ack(&pkt,one);
            PRINTF("ddddddddddddd");
            PRINTF("ack : %u\n",pkt_get_ack(&pkt));
            PRINTF("mais putain j'envois le ack mes couilles\n");
        }
        if (post_type == PTYPE_LIGHT_OFF)
        {
            PRINTF("hello off\n");
            leds_off(LEDS_RED);
        }
    }
    pkt_set_ack(&pkt, 1);
    pkt_encode(&pkt, buf);
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(":%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));

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
        printf("packet : %u \n", *buf);
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


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(boot_process, ev, data){

    PROCESS_BEGIN();
    etimer_set(&timer, 3 * CLOCK_CONF_SECOND);
    while (1)
    {   PRINTF("hello boy\n");
        PROCESS_YIELD();
        if (ev == PROCESS_EVENT_TIMER)
        {
            PRINTF("HELLO\n");
            etimer_reset(&timer);
            
        }
    }
    PROCESS_END();


}




PROCESS_THREAD(udp_server_process, ev, data)
{
    PROCESS_BEGIN();
    PRINTF("Starting the server\n");
    
    // leds_toogle(LEDS_BLUE);

#if SERVER_RPL_ROOT
    create_dag();
#endif

    uip_ip6addr(&ipaddr,0xBBBB,0,0,0,0,0,0,0x1);
    server_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));
    uint8_t msgid = 5;
    pcode_t hell = PCODE_HELLO;
    pkt_set_msgid(&hello_pkt,msgid);
    pkt_set_token(&hello_pkt,token);
    pkt_set_code(&hello_pkt, hell);
    pkt_encode(&hello_pkt, buf_hello);

    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

    etimer_set(&timer, 1 * CLOCK_CONF_SECOND);
    while (1)
    { 
        PROCESS_YIELD();
        if (ev == PROCESS_EVENT_TIMER)
        {
            // PRINTF("normal pritn\n");
            
            if(helloDeviceDone == 0){
                uip_udp_packet_send(server_conn, buf_hello, pkt_size);
                // PRINTF("HELLO\n");

            }
            else{
                uip_udp_packet_send(server_conn, buf_hello, pkt_size);
            }
            etimer_reset(&timer);
            
        }
        else if(ev == tcpip_event){
            PRINTF("hello tcp\n");
            tcpip_handler();
            etimer_set(&timer, 3 * CLOCK_CONF_SECOND);

        }
    }
    PROCESS_END();
}
