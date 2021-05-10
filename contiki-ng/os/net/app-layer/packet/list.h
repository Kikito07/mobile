#include "packet.h"
#include "stdbool.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct node {
  pkt_t pkt;
  struct node *next;
  unsigned long timer;
  struct sockaddr_in6 addr;
}node_t;

typedef struct List // list structure
{
  node_t *head; // head of list
  node_t *last;  // tail of list
  int sockfd;
  unsigned long r_timer;
} list_t;


void printList(list_t *list);
void insertFirst(pkt_t pkt, list_t *list,struct sockaddr_in6 addr,unsigned long timer);
list_t *init_list(int sockfd,unsigned long r_timer);
node_t *delete (uint8_t msgid, uint8_t token, list_t *list);
int reTransmit(list_t *list,unsigned long timer);