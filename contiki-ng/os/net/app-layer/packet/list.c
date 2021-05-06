#include <stdio.h>
#include "list.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "packet.h"

list_t *init_list()
{
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = NULL;
    list->last = NULL;
    return list;
}

void printList(list_t *list)
{
    node_t *ptr = list->head;
    printf("\n[ ");

    //start from the beginning
    while (ptr != NULL)
    {
        printf("%s\n", (ptr->pkt.payload));
        ptr = ptr->next;
    }

    printf(" ]");
}

//insert link at the first location
void insertFirst(pkt_t pkt, list_t *list)
{
    node_t *head = list->head;
    //create a link
    node_t *link = (node_t *)malloc(sizeof(node_t));

    link->pkt = pkt;

    //point it to old first node
    link->next = head;

    //point first to new first node
    list->head = link;
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

//delete a link with given key
node_t *delete (uint8_t msgid, list_t *list)
{

    node_t *head = list->head;
    node_t *current = head;
    node_t *previous = NULL;

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
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if (current == head)
    {
        //change first to point to next link
        head = head->next;
    }
    else
    {
        //bypass the current link
        previous->next = current->next;
    }

    return current;
}

// void main()
// {
//     list_t *list = init_list();
//     pkt_t pkt1;
//     pkt_t pkt2;
//     uint16_t len = 2;
//     char message1[] = "a";
//     char message2[] = "b";
//     uint8_t msgid1 = 1;
//     uint8_t msgid2 = 2;
//     pkt_set_payload(&pkt1, (const char *)(&message1), 2);
//     pkt_set_payload(&pkt2, (const char *)(&message2), 2);
//     pkt_set_msgid(&pkt1,msgid1);
//     pkt_set_msgid(&pkt2,msgid2);

//     insertFirst(pkt1, list);
//     insertFirst(pkt2, list);

//     printList(list);
//     delete(msgid1,list);
//     printf("============\n");
//     printList(list);
// }
