#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "coap-engine.h"

#ifndef DEBUG
#define DEBUG DEBUG_FULL
#endif

#define USE_RPL_CLASSIC 0

#include "dev/leds.h"

#include "net/ipv6/uip-debug.h"
#include "net/net-debug.h"
#include "dev/watchdog.h"

#if USE_RPL_CLASSIC
#include "net/routing/rpl-classic/rpl-dag-root.h"
#else
#include "net/routing/rpl-lite/rpl-dag-root.h"
#endif

#if PLATFORM_SUPPORTS_BUTTON_HAL
#include "dev/button-hal.h"
#else
#include "dev/button-sensor.h"
#endif

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_APP

#define MAX_PAYLOAD_LEN 120

#define SERVER_REPLY 1

/* Should we act as RPL root? */
#define SERVER_RPL_ROOT 0

#if SERVER_RPL_ROOT
static uip_ipaddr_t ipaddr;
#endif

/*---------------------------------------------------------------------------*/

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern coap_resource_t
  res_hello,
  res_mirror;
  //res_chunks,
  //res_separate,
  //res_push,
  //res_event,
  //res_sub,
 // res_b1_sep_b2;
/*
#if PLATFORM_HAS_LEDS
extern coap_resource_t res_leds, res_toggle;
#endif
#if PLATFORM_HAS_LIGHT
#include "dev/light-sensor.h"
extern coap_resource_t res_light;
#endif
#if PLATFORM_HAS_BATTERY
#include "dev/battery-sensor.h"
extern coap_resource_t res_battery;
#endif
#if PLATFORM_HAS_TEMPERATURE
#include "dev/temperature-sensor.h"
extern coap_resource_t res_temperature;
#end
*/

/*---------------------------------------------------------------------------*/

PROCESS(my_server, "Custom server with RPL and CoAP");
AUTOSTART_PROCESSES(&my_server);

/*---------------------------------------------------------------------------*/

// static struct uip_udp_conn *server_conn;
// static char buf[MAX_PAYLOAD_LEN];
// static uint16_t len;

/*---------------------------------------------------------------------------*/

// static void
// tcpip_handler(void)
// {
//     memset(buf, 0, MAX_PAYLOAD_LEN);
//     if(uip_newdata()) {
//         leds_on(LEDS_RED);
//         len = uip_datalen();
//         memcpy(buf, uip_appdata, len);
//         PRINTF("%u bytes from [", len);
//         PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//         PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
//     #if SERVER_REPLY
//         uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
//         server_conn->rport = UIP_UDP_BUF->srcport;

//         uip_udp_packet_send(server_conn, buf, len);
//         /* Restore server connection to allow data from any node */
//         uip_create_unspecified(&server_conn->ripaddr);
//         server_conn->rport = 0;
//     #endif
//     }
//     leds_off(LEDS_RED);
//     return;
// }

/*---------------------------------------------------------------------------*/

#if SERVER_RPL_ROOT
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state
        == ADDR_PREFERRED)) {
      PRINTF("  ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

void
create_dag()
{
    
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

    print_local_addresses();

    rpl_dag_root_set_prefix(NULL, NULL);
    int tmp = rpl_dag_root_start();

#if !USE_RPL_CLASSIC
    if (tmp == 0) {
        PRINTF("Server set as ROOT in the DAG\n");
    }
    else {
        PRINTF("RIP\n");
    }
#else
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,
                      &uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
    if(dag != NULL) {
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

PROCESS_THREAD(my_server, ev, data)
{

    PROCESS_BEGIN();
    PRINTF("Starting the custom server\n");

#if SERVER_RPL_ROOT
    create_dag();
#endif
    // server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    // udp_bind(server_conn, UIP_HTONS(3000));

    // PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

    // while(1) {
    //     PROCESS_YIELD();
    //     if(ev == tcpip_event) {
    //         PRINTF("WOW");
    //         tcpip_handler();
    //     }
    // }
    // while(1) {
    //     PROCESS_YIELD();
    //     PRINTF("OK\n");
    // }
    PRINTF("Starting Erbium Example Server\n");

    // /*
    // * Bind the resources to their Uri-Path.
    // * WARNING: Activating twice only means alternate path, not two instances!
    // * All static variables are the same for each URI path.
    // */
    coap_activate_resource(&res_hello, "test/hello");
    // coap_activate_resource(&res_mirror, "debug/mirror");
    // coap_activate_resource(&res_chunks, "test/chunks");
    // coap_activate_resource(&res_separate, "test/separate");
    // coap_activate_resource(&res_push, "test/push");
    // #if PLATFORM_HAS_BUTTON
    // coap_activate_resource(&res_event, "sensors/button");
    // #endif /* PLATFORM_HAS_BUTTON */
    // coap_activate_resource(&res_sub, "test/sub");
    // coap_activate_resource(&res_b1_sep_b2, "test/b1sepb2");
    // #if PLATFORM_HAS_LEDS
    // //coap_activate_resource(&res_leds, "actuators/leds");
    // coap_activate_resource(&res_toggle, "actuators/toggle");
    // #endif
    // #if PLATFORM_HAS_LIGHT
    // coap_activate_resource(&res_light, "sensors/light");
    // SENSORS_ACTIVATE(light_sensor);
    // #endif
    // #if PLATFORM_HAS_BATTERY
    // coap_activate_resource(&res_battery, "sensors/battery");
    // SENSORS_ACTIVATE(battery_sensor);
    // #endif
    // #if PLATFORM_HAS_TEMPERATURE
    // coap_activate_resource(&res_temperature, "sensors/temperature");
    // SENSORS_ACTIVATE(temperature_sensor);
    // #endif
    /* Define application-specific events here. */
    while(1) {
        PROCESS_YIELD();
        PRINTF("OK\n");
    }                             /* while (1) */

    PROCESS_END();
}
