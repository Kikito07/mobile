CONTIKI_PROJECT = coap-example-server-z1 coap-example-server-zoul coap-example-server-zoul-smaller
all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky #z1

# Include CoAP resources
MODULES_REL += ./resources

MAKE_WITH_DTLS=0
#MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
#MODULES += os/services/shell

CONTIKI=../

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

include $(CONTIKI)/Makefile.include
