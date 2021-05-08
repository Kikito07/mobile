#include "packet.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

/* Extra #includes */
/* Your code will be inserted here */
pkt_t *pkt_new() {
  pkt_t *pkt = malloc(sizeof(pkt_t));
  return pkt;
}

void pkt_del(pkt_t *pkt) { free(pkt); }

pkt_status_code pkt_decode(const char *data, pkt_t *pkt) {

  uint8_t header = *data;

  pkt_set_type(pkt, (ptypes_t)header >> 6);

  header = header << 2;
  header = header >> 2;
  
  pkt_set_msgid(pkt, header);
  
  pkt_set_payload(pkt,(data + 1),2);

  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t *pkt, char *buf) {

  size_t pkt_size = 0;
  int index = 0;
  *buf  = pkt_get_type(pkt) << 6;
  *buf |= pkt_get_msgid(pkt);
  index++;

  char *pl = (char *)(pkt_get_payload(pkt));
  memcpy((buf + index), pl, 2);
  return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t *pkt) { return (pkt->type); }

uint8_t pkt_get_msgid(const pkt_t *pkt) { return pkt->msgid; }

const char *pkt_get_payload(const pkt_t *pkt) { return pkt->payload; }

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type) {
  // veryfing that type argument is valid
  if (type != PTYPE_GET && type != PTYPE_ACK && type != PTYPE_POST) {
    return E_TYPE;
  }
  pkt->type = type;
  return PKT_OK;
}

pkt_status_code pkt_set_msgid(pkt_t *pkt, const uint8_t msgid) {
  pkt->msgid = msgid;
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data,
                                const uint16_t length) {
  pkt_status_code err;
  memcpy(pkt->payload, data, length);
  return PKT_OK;
}
