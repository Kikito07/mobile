#include "packet.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* Extra #includes */
/* Your code will be inserted here */
pkt_t *pkt_new() {
  pkt_t *pkt = malloc(sizeof(pkt_t));
  return pkt;
}

void pkt_del(pkt_t *pkt) { free(pkt); }

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {

  uint8_t header = *data;
  int index = 0;

  pkt_set_code(pkt, (ptypes_t)header >> 2);

  uint8_t ack = header;
  uint8_t qr = header;
  ack  = ack >> 1;
  ack = ack << 7;
  ack = ack >> 7;

  pkt_set_ack(pkt,ack);

  qr = qr << 7;
  qr = qr >> 7;
  
  pkt_set_query(pkt,qr);
  index++;
  pkt_set_msgid(pkt, *(data+index));
  index++;
  pkt_set_token(pkt,(data+index));
  index++;
  pkt_set_payload(pkt,(data+index),2);

  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *pkt, char *buf) {

  int index = 0;
  *buf  = pkt_get_code(pkt) << 2;
  *buf |= pkt_get_ack(pkt)<<1;
  *buf |= pkt_get_query(pkt);
  index++;
  *(buf+index) = pkt_get_msgid(pkt);
  index++;
  *(buf+index) = pkt_get_token(pkt);
  index++;
  char *pl = (char *)(pkt_get_payload(pkt));
  memcpy((buf + index), pl, 2);
  return PKT_OK;
}



int pkt_set_token(const pkt_t *pkt,const char* c){
  return 1;
}
int pkt_set_ack(const pkt_t *pkt,uint8_t ack){
  return 1;
}
int pkt_set_query(const pkt_t *pkt,uint8_t qr){
  return 1;
}

ptypes_t pkt_get_code(const pkt_t *pkt) { return (pkt->code); }

uint8_t pkt_get_ack(const pkt_t *pkt) {return (pkt->ack); }

query_t pkt_get_query(const pkt_t *pkt) {return (pkt->qr); }

uint8_t pkt_get_msgid(const pkt_t *pkt) { return pkt->msgid; }

uint8_t pkt_get_token(const pkt_t *pkt) {return (pkt->token); }

const char *pkt_get_payload(const pkt_t *pkt) { return pkt->payload; }

pkt_status_code pkt_set_code(pkt_t *pkt, const ptypes_t type) {
  // veryfing that type argument is valid
  if (type != PTYPE_GET && type != PTYPE_ACK && type != PTYPE_POST) {
    return E_TYPE;
  }
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
