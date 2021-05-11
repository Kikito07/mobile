#include "../os/net/app-layer/packet/packet.h"
#include "stdbool.h"
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct nodeDevice {
  struct nodeDevice *next;
  struct sockaddr_in6 * addr;
  uint8_t token;
  device_t device;
  unsigned long timer;
}node_device_t;

typedef struct listDevice // list structure
{
  node_device_t *head; // head of list
  node_device_t *last;  // tail of list
  int sockfd;
  int r_timer;
  size_t enc_pkt_size;

} list_device_t;


void printListDevice(list_device_t *list);
void insertLastDevice(list_device_t *list,struct sockaddr_in6 * addr,uint8_t token, device_t device, unsigned long timer);
list_device_t *init_listDevice(int sockfd,unsigned long r_timer);
int deleteTOutDevice (list_device_t *list,unsigned long timer);
node_device_t *findDevice(uint8_t token, device_t device, list_device_t *list);
struct sockaddr_in6* sendToDevice(list_device_t *list, device_t device, int index, char * buf);
void refreshDevice(list_device_t *list,uint8_t token, unsigned long timer);