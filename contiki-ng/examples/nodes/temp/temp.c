#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "random.h"
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
uint16_t temp = 25;

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
    if( PKT_OK != pkt_decode(buf,size,&pkt)){
        printf("packet received but encode fail");
    }
// watching if it's a post or a get


    if(pkt_get_type(&pkt) == PTYPE_POST){
        printf("je ne rentre pas dedans margoulin \n");  
        uint16_t payload = (uint16_t)*pkt_get_payload(&pkt);
        printf("payload : %u \n",payload);
        temp = payload;

        pkt_t pkt;
        ptypes_t type = PTYPE_POST;
        const uint8_t msgid = 1;

        if(PKT_OK != pkt_set_type(&pkt,type)){
            printf("problem with setting type  \n");
        }

        if(PKT_OK != pkt_set_payload(&pkt, (const char*)&payload,2)){
            printf("problem with setting payload  \n");
        }
    
        if(PKT_OK != pkt_set_msgid(&pkt,msgid)){
            printf("problem with setting msg  \n");
        }

        if(PKT_OK != pkt_encode(&pkt, buf)){
            printf("problem with the encode \n");
        }
        printf("1 \n");
        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
        server_conn->rport = UIP_UDP_BUF->srcport;
         printf("2 \n");
        uip_udp_packet_send(server_conn, buf, len);
        /* Restore server connection to allow data from any node */
        uip_create_unspecified(&server_conn->ripaddr);
        server_conn->rport = 0;
        printf("3 \n");

    }

    else if (pkt_get_type(&pkt) == PTYPE_GET)
    {   
        printf("rentre dans le get \n");
        pkt_t pkt2;
        ptypes_t type = PTYPE_GET;
        const uint8_t msgid = 1;
        char buf2[MAX_PAYLOAD_LEN];

        const char *payload = pkt_get_payload(&pkt);
        warmer_types_t post_type = payload[0];
        

        if(PKT_OK != pkt_set_type(&pkt2,type)){
            printf("problem with setting type  \n");
        }
        if(PKT_OK != pkt_set_msgid(&pkt2,msgid)){
            printf("problem with setting msg  \n"); 
        }

        if (post_type == PTYPE_THERM){
            printf("PTYPE_SENS\n");
            pkt_set_payload(&pkt2, (const char*)&temp ,2);
                
        }

        if (post_type == PTYPE_SENS){
            int ra = abs(rand());
            uint16_t randi= ra%10 + temp;


            printf("PTYPE_TERM\n");
            pkt_set_payload(&pkt2, (const char*)&randi ,2);
                
        }
        

        if(PKT_OK != pkt_encode(&pkt2, buf2)){
            printf("problem with the encode \n");
        }

        uint16_t test = (uint16_t) *pkt_get_payload(&pkt2);
        printf("%u \n",test);
        printf("1 \n");
        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
        server_conn->rport = UIP_UDP_BUF->srcport;
        printf("2 \n");
        uip_udp_packet_send(server_conn, buf2, len);
        /* Restore server connection to allow data from any node */
        uip_create_unspecified(&server_conn->ripaddr);
        server_conn->rport = 0;
        printf("3 \n");
    }
    
    
    else {
        printf("packet is not reconized \n");
    }
    
    
}

static void
tcpip_handler(void)
{
    memset(buf, 0, MAX_PAYLOAD_LEN);
    if(uip_newdata()) {
        len = uip_datalen();
        memcpy(buf, uip_appdata, len);
        handle_packet();
        printf("packet : %u \n", *buf);
    // #if SERVER_REPLY
    //     uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    //     server_conn->rport = UIP_UDP_BUF->srcport;

    //     uip_udp_packet_send(server_conn, buf, len);
    //     /* Restore server connection to allow data from any node */
    //     uip_create_unspecified(&server_conn->ripaddr);
    //     server_conn->rport = 0;
    // #endif
    }
    
    return;
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_server_process, ev, data)
{

    PROCESS_BEGIN();
    PRINTF("Starting the server\n");

   
    
    // leds_toogle(LEDS_BLUE);

#if SERVER_RPL_ROOT
    create_dag();
#endif
    server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));

    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

    while(1) {
        PROCESS_YIELD();
        if(ev == tcpip_event) {

            tcpip_handler();
        }
    }

    PROCESS_END();
}
