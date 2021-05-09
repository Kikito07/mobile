#include <stdio.h>
#include "listDevice.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../os/net/app-layer/packet/packet.h"

list_device_t *init_listDevice(int sockfd,unsigned long r_timer)
{
    list_device_t *list = malloc(sizeof(list_device_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = NULL;
    list->last = NULL;
    list->r_timer = r_timer;
    list->sockfd = sockfd;
    list->enc_pkt_size = 6;
    return list;
}

void printListDevice(list_device_t *list)
{
    node_device_t *ptr = list->head;
    printf("\n[ ");

    //start from the beginning
    while (ptr != NULL)
    {
        printf("device : %u , token : %u \n", (ptr->device),(ptr->token));
        ptr = ptr->next;
    }

    printf(" ]");
}

int compare_ipv6(struct in6_addr *ipA, struct in6_addr *ipB)
{
    int i = 0;
    for(i = 0; i < 16; ++i) // Don't use magic number, here just for example
    {
        if (ipA->s6_addr[i] < ipB->s6_addr[i])
            return -1;
        else if (ipA->s6_addr[i] > ipB->s6_addr[i])
            return 1;
    }
    return 0;
}

int isNotInList(list_device_t *list,struct sockaddr_in6 *addr){
    node_device_t *ptr = list->head;

    //start from the beginning
    while (ptr != NULL)
    {        
        struct sockaddr_in6 *listAddr = ptr->addr;

        if(compare_ipv6(&listAddr->sin6_addr,&addr->sin6_addr) == 0){
            return 0;
        }
        ptr = ptr->next;

    }
    return 1;

}


//insert link at the first location

void sendToDevice(list_device_t *list, device_t device, int index, char * buf){
    
    size_t enc_pkt_size = 6;
    node_device_t *ptr = list->head;
    int i = 0;

    //start from the beginning
    while (ptr != NULL)
    {        
        device_t dev = ptr->device;

        if(dev == device){
            i++;
            if(i == index){
                
                int err = sendto(list->sockfd, buf, list->enc_pkt_size,
                            MSG_CONFIRM, (const struct sockaddr *)ptr->addr,
                            sizeof(*(ptr->addr)));
                if(err < 0){
                    perror("failed to send : ");
                }
                
            }
        }
        ptr = ptr->next;

    }
    
}
void insertLastDevice(list_device_t *list,struct sockaddr_in6 * addr,uint8_t token, device_t device, unsigned long timer)
{

    if(1 == isNotInList(list,addr)){
        printf("hello insert\n");
        node_device_t *head = list->head;
        //create a link
        node_device_t *link = (node_device_t *)malloc(sizeof(node_device_t));
    
        link->addr = addr;

        link-> token = token; 

        //point it to old first node
        link->device = device;

        //point first to new first node
        link->timer = timer;

        link->next = NULL;

        if(head == NULL){
            list->head = link;
            return;
        }
        node_device_t *current = head;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = link;
    }

}
//is list empty
bool isEmptyListDevice(list_device_t *list)
{
    node_device_t *head = list->head;
    return head == NULL;
}

int lengthDevice(list_device_t *list)
{
    int length = 0;
    node_device_t *current;

    for (current = list->head; current != NULL; current = current->next)
    {
        length++;
    }

    return length;
}

//find a link with given key
node_device_t *findDevice(uint8_t token, device_t device, list_device_t *list)
{

    node_device_t *head = list->head;

    //start from the first link
    node_device_t *current = head;

    //if list is empty
    if (head == NULL)
    {
        return NULL;
    }

    //navigate through list
    while ((current->token != token) && (current->device != device))
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

//delete a link with given key
int deleteTOutDevice (list_device_t *list,unsigned long timer)
{
    node_device_t *current = list->head;
    node_device_t *previous = NULL;

    //if list is empty
    if (current == NULL)
    {
        return -1;
    }
    //navigate through list
    while (current != NULL)
    {
        // printf("timer diff : %d\n", (timer - (current -> timer)));
        // printf("r_timer = %d \n", list->r_timer);
        printf("device = %u\n",current->device);


        if((timer-(current -> timer)) > list->r_timer){
            node_device_t *tmp = current;
            if (current == list->head)
            {
                list->head = list->head->next;
                current = current->next;
            }
            else
            {
                previous->next = current->next;
                current = current->next;
            }
            free(tmp->addr);
            free(tmp);

        }
        else{
            previous = current;
            current = current->next;
        } 
    }
return 1;
}

// void main()
// {
//     list_device_t *list = init_listDevice(20);
//     device_t device1 = LAMP;
//     device_t device2 = TEMP;
//     device_t device3 = SERV;
//     uint8_t token = 1;
//     struct sockaddr_in6 addr;
//     insertLastDevice(list,&addr,token, device1, 10);
//     insertLastDevice(list,&addr,token, device2, 0);
//     insertLastDevice(list,&addr,token, device3, 10);
//     // deleteTOutDevice(list, 25);
//     printListDevice(list);
// }