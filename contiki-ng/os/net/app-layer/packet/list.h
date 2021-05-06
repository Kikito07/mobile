#include "packet.h"
#include "stdbool.h"


typedef struct node {
   pkt_t pkt;
   struct node *next;
   uint8_t timer;
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



void insertFirst(pkt_t pkt, list_t *list);
list_t *init_list();