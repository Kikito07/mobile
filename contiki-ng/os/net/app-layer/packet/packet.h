#ifndef __PACKET_INTERFACE_H_
#define __PACKET_INTERFACE_H_

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintx_t */
#include <stdio.h>  /* ssize_t */
#include <time.h>
#include <stdbool.h>

typedef enum
{
  LAMP = 0,
  TEMP = 1,
  SERV = 2,
  ALARM = 3,
  DETECTOR = 4,
} device_t;

/* Code */
typedef enum
{
  PCODE_GET = 1,
  PCODE_POST = 2,
  PCODE_HELLO = 3,
  PCODE_ALARM = 4,
} pcode_t;

typedef enum
{
  PTYPE_TEMP = 1,
  PTYPE_SENSOR = 2,
} get_types_t;

typedef enum
{
  PTYPE_SENS = 1,
  PTYPE_THERM = 2,
} warmer_types_t;

typedef enum
{
  PTYPE_LIGHT_ON = 1,
  PTYPE_LIGHT_OFF = 2,
} lamp_types_t;

typedef enum
{
  ACTIVATE = 1,
  DESACTIVATE = 2,
  GET = 3,
} detector_types_t;

#define MAX_PAYLOAD_SIZE 2

typedef struct __attribute__((__packed__)) pkt
{
  pcode_t code;
  device_t device;
  uint8_t msgid;
  uint8_t ack;
  char payload[2];
} pkt_t;

typedef enum
{
  PKT_OK = 0,
  E_TYPE,
  E_TR,
  E_LENGTH,
  E_CRC,
  E_WINDOW,
  E_SEQNUM,
  E_NOMEM,
  E_NOHEADER,
  E_UNCONSISTENT,
} pkt_status_code;

pkt_t *pkt_new();

void pkt_del(pkt_t *);

pkt_status_code pkt_decode(const char *data, pkt_t *pkt);

pkt_status_code pkt_encode(const pkt_t *, char *buf);

pcode_t pkt_get_code(const pkt_t *);

uint8_t pkt_get_ack(const pkt_t *pkt);

device_t pkt_get_device(const pkt_t *pkt);

uint8_t pkt_get_msgid(const pkt_t *);

uint8_t pkt_get_token(const pkt_t *pkt);

const char *pkt_get_payload(const pkt_t *);

pkt_status_code pkt_set_code(pkt_t *, const pcode_t type);

pkt_status_code pkt_set_msgid(pkt_t *, const uint8_t seqnum);

pkt_status_code pkt_set_payload(pkt_t *, const char *data,
                                const uint16_t length);
pkt_status_code pkt_set_device(pkt_t *pkt, device_t device);

pkt_status_code pkt_set_ack(pkt_t *pkt, uint8_t ack);

#endif