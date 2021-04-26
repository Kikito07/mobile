#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "coap-engine.h"

#ifndef DEBUG
#define DEBUG DEBUG_FULL
#endif

#define USE_RPL_CLASSIC 0

#include "net/ipv6/uip-debug.h"
#include "net/net-debug.h"
#include "dev/watchdog.h"

#if USE_RPL_CLASSIC
#include "net/routing/rpl-classic/rpl-dag-root.h"
#else
#include "net/routing/rpl-lite/rpl-dag-root.h"
#endif

#define MAX_PAYLOAD_LEN 120

/*---------------------------------------------------------------------------*/

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */

extern coap_resource_t res_hello;

/*---------------------------------------------------------------------------*/

PROCESS(my_server, "Custom server with RPL and CoAP");
AUTOSTART_PROCESSES(&my_server);

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(my_server, ev, data)
{

    PROCESS_BEGIN();
    printf("Starting the custom server\n");

    // /*
    // * Bind the resources to their Uri-Path.
    // * WARNING: Activating twice only means alternate path, not two instances!
    // * All static variables are the same for each URI path.
    // */

    coap_activate_resource(&res_hello, "test/hello");
    while(1) {
        PROCESS_YIELD();
        printf("OK\n");
    }                             /* while (1) */

    PROCESS_END();
}