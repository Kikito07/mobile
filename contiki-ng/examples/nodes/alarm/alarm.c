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
#define SERVER_REPLY 1
#define SERVER_RPL_ROOT 0

/*---------------------------------------------------------------------------*/
// GLOBAL VARIABLE

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;
static char buf_hello[MAX_PAYLOAD_LEN];
static uint16_t len;
static struct etimer timer;
static uip_ipaddr_t ipaddr;
static size_t pkt_size = 5;
static device_t device = ALARM;
static uint8_t msgid = 0;
static pcode_t hello = PCODE_HELLO;

/*---------------------------------------------------------------------------*/

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/

static void 
handle_packet()
{   
    pkt_t pkt;
    
    if (PKT_OK != pkt_decode(buf, &pkt))
    {
        PRINTF("packet received but encode fail");
    }

    // Turnon the alarm if the deveice receive a alarm packet
    pcode_t code = pkt_get_code(&pkt);

    if (code == PCODE_ALARM)
    {
        leds_on(LEDS_RED);

    }

    // turnoff the alarm if a post message is receive 
    if (code == PCODE_POST){
        leds_off(LEDS_RED);
    }
    
    pkt_set_device(&pkt,ALARM);
    pkt_set_ack(&pkt, 1);
    pkt_encode(&pkt, buf);
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(":%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
    uip_udp_packet_send(server_conn, buf, len);

}

/*---------------------------------------------------------------------------*/

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
    
    }
    return;
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_server_process, ev, data)
{
    PROCESS_BEGIN();
    PRINTF("Starting the server\n");
    

    uip_ip6addr(&ipaddr,0xBBBB,0,0,0,0,0,0,0x1);
    server_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));
    static pkt_t hello_pkt;
    pkt_set_msgid(&hello_pkt,msgid);
    pkt_set_code(&hello_pkt, hello);
    pkt_set_device(&hello_pkt,device);
    pkt_set_ack(&hello_pkt, 0);
    pkt_encode(&hello_pkt, buf_hello);
    
    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

    uip_udp_packet_send(server_conn, buf_hello, pkt_size);
    etimer_set(&timer, 3 * CLOCK_CONF_SECOND);
    while (1)
    { 
        PROCESS_YIELD();
        if (ev == PROCESS_EVENT_TIMER)
        {   
            // Send a hello packet to keep the connection alive.
            uip_udp_packet_send(server_conn, buf_hello, pkt_size);
            etimer_reset(&timer);
            
        }
        else if(ev == tcpip_event){
            PRINTF("tcp ip event\n");
            // If the device receive a packet tcpip_handler manage the packet.
            tcpip_handler();
        }
    }
    PROCESS_END();
}
