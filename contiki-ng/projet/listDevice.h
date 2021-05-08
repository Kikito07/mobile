#include "../os/net/app-layer/packet/packet.h"
#include "stdbool.h"
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct node {
  struct node *next;
  struct sockaddr_in6 addr;
  uint8_t token;
  device_t device;
  int timer;
}node_device_t;

typedef struct listDevice // list structure
{
  node_device_t *head; // head of list
  node_device_t *last;  // tail of list
  int size;      // number of element in list
  int window;    // window of the reseau
  bool marker;
  int r_timer;
} list_device_t;


void printListDevice(list_device_t *list);
void insertFirstDevice(list_device_t *list,struct sockaddr_in6 addr,uint8_t token, device_t device, int timer);
list_device_t *init_listDevice(int r_timer);
int deleteTOutDevice (list_device_t *list,int timer);
node_device_t *findDevice(uint8_t token, device_t device, list_device_t *list);
