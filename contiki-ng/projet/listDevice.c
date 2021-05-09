#include <stdio.h>
#include "listDevice.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../os/net/app-layer/packet/packet.h"

list_device_t *init_listDevice(unsigned long r_timer)
{
    list_device_t *list = malloc(sizeof(list_device_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = NULL;
    list->last = NULL;
    list->r_timer = r_timer;
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

//insert link at the first location
void insertLastDevice(list_device_t *list,struct sockaddr * addr,uint8_t token, device_t device, unsigned long timer)
{
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
//     struct sockaddr addr;
//     insertLastDevice(list,&addr,token, device1, 10);
//     insertLastDevice(list,&addr,token, device2, 0);
//     insertLastDevice(list,&addr,token, device3, 10);
//     // deleteTOutDevice(list, 25);
//     printListDevice(list);
// }