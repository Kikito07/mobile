#include "dev/button-sensor.h"
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

AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/

static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static char buf_hello[MAX_PAYLOAD_LEN];
static uint16_t len;
static struct etimer timer;

static uip_ipaddr_t ipaddr;
static size_t pkt_size = 5;

static int activate = 0;

static device_t device = DETECTOR;
static uint8_t msgid = 0;
static pcode_t hello = PCODE_HELLO;

/*---------------------------------------------------------------------------*/

static void 
handle_packet(int sensor)
{   
    
    pkt_t pkt;

    if( sensor == 0)
    {   
        pkt_set_ack(&pkt, 1);
        if (PKT_OK != pkt_decode(buf, &pkt) )
        {
            PRINTF("packet received but encode fail");
        }
        
        if (pkt_get_code(&pkt) == PCODE_POST)
        {
            const char *payload = pkt_get_payload(&pkt);
            alarm_types_t post_type = payload[0];

            uint8_t one = 1;
            pkt_set_ack(&pkt,one);

            if (post_type == ACTIVATE)
            {
                leds_on(LEDS_RED);
                activate = 1;

            }
            if (post_type == DESACTIVATE)
            {

                leds_off(LEDS_RED);
                activate = 0;
            }
        }

        else if((pkt_get_code(&pkt)) == PCODE_GET){
            if(activate == 1){
                alarm_types_t act = ACTIVATE;
                pkt_set_payload(&pkt, ((const char*)&act),2);
            }
            else{
                alarm_types_t act = DESACTIVATE;
                pkt_set_payload(&pkt,(const char*)&act,2);
            }
            
        }
    }
    if( sensor == 1){

        PRINTF("SENSEUR ALOO ALOOO MAX VERSTAPPEN EST SURPER NULL");
        
        pkt_set_code(&pkt, PCODE_ALARM);

        pkt_set_msgid(&pkt, 0);

        pkt_set_device(&pkt,DETECTOR);

        pkt_set_ack(&pkt,0);
    }



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
tcpip_handler(int sensor)
{
    memset(buf, 0, MAX_PAYLOAD_LEN);
    if (uip_newdata())
    {
        len = uip_datalen();
        memcpy(buf, uip_appdata, len);
        PRINTF("%u bytes from [", len);
        PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
        PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
        handle_packet(sensor);
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





PROCESS_THREAD(udp_server_process, ev, data)
{
    PROCESS_BEGIN();
    PRINTF("Starting the server\n");
    SENSORS_ACTIVATE(button_sensor);
    
    // leds_toogle(LEDS_BLUE);

#if SERVER_RPL_ROOT
    create_dag();
#endif

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

    etimer_set(&timer, 3 * CLOCK_CONF_SECOND);
    while (1)
    { 
        PROCESS_YIELD();
        if (ev == PROCESS_EVENT_TIMER)
        {  
            uip_udp_packet_send(server_conn, buf_hello, pkt_size);
            etimer_reset(&timer);
            
        }
        else if(ev == tcpip_event){
            PRINTF("tcp ip event\n");
            tcpip_handler(0);
        }

        else if(ev == sensors_event){
            tcpip_handler(1);
            etimer_set(&timer, 3 * CLOCK_CONF_SECOND);
        }
    }
    PROCESS_END();
}