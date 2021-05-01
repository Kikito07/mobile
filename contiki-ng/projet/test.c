#include <stdio.h>
#include "/mnt/C072C89972C89616/school/embedded/mobile/contiki-ng/os/net/app-layer/packet/packet.h"


//big endian
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = 0; i < size; i++) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        printf("\n");
    }
    puts("");
}

int main(int argc, char *argv[]){
  pkt_t* pkt = pkt_new();
  char buf[19];
  post_types_t post_type = PTYPE_LIGHT_ON;
  uint16_t data = 16;
  size_t size = 3;
  uint32_t test = 2048;
  const size_t sizeBuf = 3;
  const size_t sizePkt = 21;
  size_t header_size = 1;
  size_t payload_size = 2;

  pkt_set_type(pkt, PTYPE_GET);

  pkt_set_msgid(pkt, 2);

  pkt_set_payload(pkt, (const char*)&post_type,2);

  // printBits(sizePkt,pkt);
  if( PKT_OK != pkt_encode(pkt, buf)){
    printf("loooooooser encode don't fonctionning");
  }
  printBits(sizeBuf,buf);
  
  if( PKT_OK != pkt_decode(buf, size, pkt)){
    printf("loooooooser decode don't fonctionning");
  }
  printf("===========\n");
  ptypes_t type = pkt_get_type(pkt);
  uint8_t msgid = pkt_get_msgid(pkt);
  const char *payload = pkt_get_payload(pkt);
  printBits(header_size,&type);
  printBits(payload_size,payload);
  printBits(header_size,&msgid);
  printf("===========\n");

  printBits(sizeBuf,buf);
  if( PKT_OK != pkt_encode(pkt, buf)){
    printf("loooooooser encode don't fonctionning");
  }
  printBits(sizeBuf,buf);
}