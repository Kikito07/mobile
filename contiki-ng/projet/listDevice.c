#include <stdio.h>
#include "listDevice.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


int compare_ipv6Bis(struct in6_addr *ipA, struct in6_addr *ipB)
{
    int i = 0;
    for(i = 0; i < 16; ++i)
    {
        if (ipA->s6_addr[i] < ipB->s6_addr[i])
            return -1;
        else if (ipA->s6_addr[i] > ipB->s6_addr[i])
            return 1;
    }
    return 0;
}

void printDevice(device_t device){
    if(device == LAMP){
        printf("lamp ");
    }
    else if(device == ALARM){
        printf("alarm ");
    }
    else if(device == DETECTOR){
        printf("detector ");
    }
    else if(device == TEMP){
        printf("thermostat ");
    }
}

void ipv6_to_str_unexpandedBis(const struct in6_addr *addr)
{
    printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
           (int)addr->s6_addr[0], (int)addr->s6_addr[1],
           (int)addr->s6_addr[2], (int)addr->s6_addr[3],
           (int)addr->s6_addr[4], (int)addr->s6_addr[5],
           (int)addr->s6_addr[6], (int)addr->s6_addr[7],
           (int)addr->s6_addr[8], (int)addr->s6_addr[9],
           (int)addr->s6_addr[10], (int)addr->s6_addr[11],
           (int)addr->s6_addr[12], (int)addr->s6_addr[13],
           (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
}


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
    list->enc_pkt_size = 5;
    return list;
}

void printListDevice(list_device_t *list)
{
    node_device_t *ptr = list->head;
    printf("[\n");

    //start from the beginning
    while (ptr != NULL)
    {   
        int i = findDevice(list, ptr->device,ptr->addr);

        printDevice(ptr->device);
        printf(": %d\n at address :",i);
        ipv6_to_str_unexpandedBis(&(ptr->addr->sin6_addr));
        printf("\n");
        ptr = ptr->next;
    }

    printf(" \n ]");
}



int isNotInList(list_device_t *list,struct sockaddr_in6 *addr){
    node_device_t *ptr = list->head;

    //start from the beginning
    while (ptr != NULL)
    {        
        struct sockaddr_in6 *listAddr = ptr->addr;
       
        if(compare_ipv6Bis(&listAddr->sin6_addr,&addr->sin6_addr) == 0){
            return 0;
        }
        ptr = ptr->next;

    }
    return 1;

}


//insert link at the first location

struct sockaddr_in6* sendToDevice(list_device_t *list, device_t device, int index, char * buf){
    
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
                    return NULL;
                }
                struct sockaddr_in6 * mallocedNode = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
                memcpy(mallocedNode,ptr->addr,sizeof(struct sockaddr_in6));
                return mallocedNode;
                
            }
        }
        ptr = ptr->next;

    }
    return NULL;
    
}


void refreshDevice(list_device_t *list,struct sockaddr_in6 * addr, unsigned long timer){
    node_device_t *ptr = list->head;
    int i = 0;
    //start from the beginning
    while (ptr != NULL)
    {
        i++;
        
        if(compare_ipv6Bis(&(ptr->addr->sin6_addr),&addr->sin6_addr) == 0){
            ptr->timer = timer;
        }
        ptr = ptr->next;

    }
}

void insertLastDevice(list_device_t *list,struct sockaddr_in6 * addr, device_t device, unsigned long timer)
{


    if(1 == isNotInList(list,addr)){
        printf("device ");
        printDevice(device);
        printf("connected\n");
        node_device_t *head = list->head;
        //create a link
        node_device_t *link = (node_device_t *)malloc(sizeof(node_device_t));
    
        link->addr = addr;

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
    else{
        refreshDevice(list, addr,timer);
        free(addr);
    }

}
//is list empty
bool isEmptyListDevice(list_device_t *list)
{
    node_device_t *head = list->head;
    return head == NULL;
}

int lengthDevice(list_device_t *list,device_t device)
{
    int length = 0;
    node_device_t *current;

    for (current = list->head; current != NULL; current = current->next)
    {
        if(current->device == device){
            length++;
        }
        
    }

    return length;
}


int findDevice(list_device_t *list,device_t device, struct sockaddr_in6 * addr)
{

    node_device_t *head = list->head;

    //start from the first link
    node_device_t *current = head;

    //if list is empty
    if (head == NULL)
    {
        return -1;
    }

    int index = 0;
    //navigate through list
    while (compare_ipv6Bis(&(current->addr->sin6_addr),&addr->sin6_addr) != 0)
    {
        //if it is last node
        if (current->next == NULL)
        {
            return -1;
        }
        else
        {
            if(current->device == device){
                index++;
            }
            //go to next link
            current = current->next;
            
        }
    }
    //if data found, return the current Link
    return index + 1;
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
        
        if((timer-(current -> timer)) > list->r_timer){
            printf("lost connection with");
            printDevice(current->device);
            printf("\n");
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
