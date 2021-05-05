#ifndef __PACKET_INTERFACE_H_
#define __PACKET_INTERFACE_H_

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintx_t */
#include <stdio.h>  /* ssize_t */
#include <time.h>
#include <stdbool.h>

/* Types de paquets */
typedef enum {
  PTYPE_GET = 1,
  PTYPE_POST = 2,
  PTYPE_ACK = 3,
} ptypes_t;



/*   types de get packet */
typedef enum {
  PTYPE_TEMP = 1,
  PTYPE_SENSOR = 2,
} get_types_t;

typedef enum{
  PTYPE_SENS = 1,
  PTYPE_THERM = 2,
} warmer_types_t;

/*   types de post packet */
typedef enum {
  PTYPE_LIGHT_ON = 1,
  PTYPE_LIGHT_OFF = 2,
} post_types_t;

/* Taille maximale permise pour le payload */
#define MAX_PAYLOAD_SIZE 2
/* Valeur de retours des fonctions */
typedef struct __attribute__((__packed__)) pkt {
  ptypes_t type;
  uint8_t msgid;
  char payload[2];
} pkt_t;

typedef enum {
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

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt);

pkt_status_code pkt_encode(const pkt_t *, char *buf);

ptypes_t pkt_get_type(const pkt_t *);

uint8_t pkt_get_msgid(const pkt_t *);

const char *pkt_get_payload(const pkt_t *);

pkt_status_code pkt_set_type(pkt_t *, const ptypes_t type);

pkt_status_code pkt_set_msgid(pkt_t *, const uint8_t seqnum);

pkt_status_code pkt_set_payload(pkt_t *, const char *data,
                                const uint16_t length);


#endif