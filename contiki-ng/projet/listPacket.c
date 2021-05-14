#include <stdio.h>
#include "listPacket.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>


//List that stores the packet that are already send waiting for a ACK

list_t *init_list(int sockfd, unsigned long r_timer)
{
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = NULL;
    list->last = NULL;
    list->sockfd = sockfd;
    list->r_timer = r_timer;

    pthread_mutex_init(&(list->lock), NULL);
    return list;
}

void printList(list_t *list)
{
    node_t *ptr = list->head;
    printf("\n[ ");
    //start from the beginning
    while (ptr != NULL)
    {
        printf("packet with MSGID : %u\n", (ptr->pkt.msgid));
        ptr = ptr->next;
    }
    printf(" ]");
}

//insert link at the first location
void insertFirst(pkt_t pkt, list_t *list, struct sockaddr_in6 *addr, unsigned long timer)
{
    pthread_mutex_lock(&(list->lock));
    node_t *head = list->head;
    //create a link
    node_t *link = (node_t *)malloc(sizeof(node_t));

    link->pkt = pkt;

    link->addr = addr;

    //point it to old first node
    link->next = head;

    link->timer = timer;

    link->counter = 5;

    //point first to new first node
    list->head = link;
    pthread_mutex_unlock(&(list->lock));
}
//is list empty
bool isEmpty(list_t *list)
{
    node_t *head = list->head;
    return head == NULL;
}

int length(list_t *list)
{
    int length = 0;
    node_t *current;

    for (current = list->head; current != NULL; current = current->next)
    {
        length++;
    }

    return length;
}

//find a link with given key
node_t *find(uint8_t msgid, list_t *list)
{

    node_t *head = list->head;

    //start from the first link
    node_t *current = head;

    //if list is empty
    if (head == NULL)
    {
        return NULL;
    }

    //navigate through list
    while (current->pkt.msgid != msgid)
    {

        //if it is last node
        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            //go to next link
            current = current->next;
        }
    }
    //if data found, return the current Link
    return current;
}

//retransmits packets with an expired timer
int reTransmit(list_t *list, unsigned long timer)
{
    pthread_mutex_lock(&(list->lock));
    node_t *current = list->head;
    node_t *previous = NULL;
    //if list is empty
    if (current == NULL)
    {
        pthread_mutex_unlock(&(list->lock));
        return -1;
    }
    //navigate through list
    while (current != NULL)
    {

        if ((timer - (current->timer)) > list->r_timer)
        {
            printf("packet with MSGID : %u retransmitted\n", (current->pkt.msgid));

            if (current->counter == 0)
            {
                printf("retransmission limit reached for packet with MSGID %u\n",(current->pkt.msgid));

                if (current == list->head)
                {
                    //change first to point to next link
                    list->head = list->head->next;
                }
                else
                {
                    //bypass the current link
                    previous->next = current->next;
                }
            }
            else
            {
                char buf[10];
                pkt_t pkt = current->pkt;
                pkt_encode(&pkt, buf);
                int err = sendto(list->sockfd, buf, 5,
                                 MSG_CONFIRM, (const struct sockaddr *)current->addr,
                                 sizeof(struct sockaddr_in6));
                if (err < 0)
                {
                    perror("send error : ");
                    pthread_mutex_unlock(&(list->lock));
                    return -1;
                }
                (current->counter)--;
            }
            current->timer = timer;
        }
        previous = current;
        current = current->next;
    }
    pthread_mutex_unlock(&(list->lock));
    return 1;
}

//function used to compare ipv6 adresses
int compare_ipv6(struct in6_addr *ipA, struct in6_addr *ipB)
{
    int i = 0;
    for (i = 0; i < 16; ++i)
    {
        if (ipA->s6_addr[i] < ipB->s6_addr[i])
            return -1;
        else if (ipA->s6_addr[i] > ipB->s6_addr[i])
            return 1;
    }
    return 0;
}

//removes a packet from the list
node_t *delete (uint8_t msgid, list_t *list, struct sockaddr_in6 *addr)
{

    pthread_mutex_lock(&(list->lock));

    node_t *head = list->head;
    node_t *current = head;
    node_t *previous = NULL;

    //if list is empty
    if (head == NULL)
    {
        return NULL;
    }

    //navigate through list
    while ((current->pkt.msgid != msgid) && compare_ipv6(&(current->addr->sin6_addr), &addr->sin6_addr) != 0)
    {

        //if it is last node
        if (current->next == NULL)
        {
            return NULL;
            pthread_mutex_unlock(&(list->lock));
        }
        else
        {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }
    //found a match, update the link

    node_t *tmp = current;
    if (current == head)
    {
        //change first to point to next link
        list->head = head->next;
    }
    else
    {
        //bypass the current link
        previous->next = current->next;
    }
    free(tmp->addr);
    free(tmp);
    pthread_mutex_unlock(&(list->lock));
    return NULL;
}
