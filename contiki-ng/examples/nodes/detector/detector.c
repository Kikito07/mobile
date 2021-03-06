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
static int activate = 0;
static device_t device = DETECTOR;
static uint8_t msgid = 0;
static pcode_t hello = PCODE_HELLO;

/*---------------------------------------------------------------------------*/

PROCESS(udp_server_process, "UDP server process");

AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/


static void
handle_packet(int sensor)
{

    pkt_t pkt;

    if (sensor == 0)
    {
        
        if (PKT_OK != pkt_decode(buf, &pkt))
        {
            PRINTF("packet received but encode fail");
        }
        pkt_set_device(&pkt, DETECTOR);
        pkt_set_ack(&pkt, 1);

        // activate or deactivate the detector
        if (pkt_get_code(&pkt) == PCODE_POST)
        {
            const char *payload = pkt_get_payload(&pkt);
            detector_types_t post_type = payload[0];

            uint8_t one = 1;
            pkt_set_ack(&pkt, one);

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
        
        // know if the device is activate or not
        else if (((pkt_get_code(&pkt)) == PCODE_GET) )
        {   
            PRINTF("HOOOOO PUTAINGG \n");
            if (activate == 1)
            {
                detector_types_t act = ACTIVATE;
                pkt_set_payload(&pkt, ((const char *)&act), 2);
            }
            else
            {
                detector_types_t act = DESACTIVATE;
                pkt_set_payload(&pkt, (const char *)&act, 2);
            }
        }
    }

    // If the device detect something and it's activated, it send an alarm packet to the server
    else if (sensor == 1 && (activate == 1))
    {

        PRINTF("SENSEUR ALOO ALOOO MAX VERSTAPPEN EST SURPER NULL\n ");

        pkt_set_code(&pkt, PCODE_ALARM);

        pkt_set_msgid(&pkt, 0);

        pkt_set_device(&pkt, DETECTOR);

        pkt_set_ack(&pkt, 0);
    }

    pkt_encode(&pkt, buf);
    uip_udp_packet_send(server_conn, buf, len);
}


/*---------------------------------------------------------------------------*/

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

    uip_ip6addr(&ipaddr, 0xBBBB, 0, 0, 0, 0, 0, 0, 0x1);
    server_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));
    static pkt_t hello_pkt;
    pkt_set_msgid(&hello_pkt, msgid);
    pkt_set_code(&hello_pkt, hello);
    pkt_set_device(&hello_pkt, device);
    pkt_set_ack(&hello_pkt, 0);
    pkt_encode(&hello_pkt, buf_hello);

    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);
    // Send the first hello packet to connect the device to the server. 
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
        else if (ev == tcpip_event)
        {   
            // manage the packet if the device receive a packet
            PRINTF("tcp ip event\n");
            tcpip_handler(0);
        }

        else if (ev == sensors_event)
        {   
            // manage the packet if the sensor of the device detect something
            PRINTF("nikita easy game \n");
            handle_packet(1);
            etimer_set(&timer, 3 * CLOCK_CONF_SECOND);
        }
    }
    PROCESS_END();
}