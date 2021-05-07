#include "packet.h"
#include "stdbool.h"
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct node {
   pkt_t pkt;
   struct node *next;
   uint8_t timer;
  struct sockaddr_in6 addr;
}node_t;

typedef struct List // list structure
{
  node_t *head; // head of list
  node_t *last;  // tail of list
  int size;      // number of element in list
  int window;    // window of the reseau
  bool marker;
  unsigned long r_timer;
} list_t;


void printList(list_t *list);
void insertFirst(pkt_t pkt, list_t *list,struct sockaddr_in6 addr);
list_t *init_list();
node_t *delete (uint8_t msgid, uint8_t token, list_t *list);