#include "packet.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

//Implementation of the packet

pkt_t *pkt_new() {
  pkt_t *pkt = (pkt_t *) malloc(sizeof(pkt_t));
  return pkt;
}
void pkt_del(pkt_t *pkt) { free(pkt); }

pkt_status_code pkt_decode(const char *data, pkt_t *pkt) {

  uint8_t header = *data;
  int index = 0;

  pkt_set_code(pkt, (pcode_t)header >> 1);

  uint8_t ack = header;
  ack = ack << 7;
  ack = ack >> 7;
  pkt_set_ack(pkt,ack);
  index++;
  pkt_set_device(pkt, *(data+index));
  index++;
  pkt_set_msgid(pkt, *(data+index));
  index++;
  pkt_set_payload(pkt,(data+index),2);

  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *pkt, char *buf) {

  int index = 0;
  *buf  = pkt_get_code(pkt) << 1;
  *buf |= pkt_get_ack(pkt);
  index++;
  *(buf+index) = pkt_get_device(pkt);
  index++;
  *(buf+index) = pkt_get_msgid(pkt);
  index++;
  char *pl = (char *)(pkt_get_payload(pkt));
  memcpy((buf + index), pl, 2);
  return PKT_OK;
}



pkt_status_code pkt_set_ack(pkt_t *pkt,uint8_t ack){
  pkt->ack = ack;
  return PKT_OK;
}


pkt_status_code pkt_set_device(pkt_t *pkt,device_t device){
  pkt->device = device;
  return PKT_OK;
}

pcode_t pkt_get_code(const pkt_t *pkt) { return (pkt->code); }

uint8_t pkt_get_ack(const pkt_t *pkt) {return (pkt->ack); }


uint8_t pkt_get_msgid(const pkt_t *pkt) { return pkt->msgid; }

device_t pkt_get_device(const pkt_t *pkt) {return (pkt->device); }

const char *pkt_get_payload(const pkt_t *pkt) { return pkt->payload; }

pkt_status_code pkt_set_code(pkt_t *pkt, const pcode_t type) {
  // veryfing that type argument is valid
  pkt->code = type;
  return PKT_OK;
}

pkt_status_code pkt_set_msgid(pkt_t *pkt, const uint8_t msgid) {
  pkt->msgid = msgid;
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data,
                                const uint16_t length) {
  memcpy(pkt->payload, data, length);
  return PKT_OK;
}
