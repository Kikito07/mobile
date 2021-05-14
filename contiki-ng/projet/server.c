
/*
#########################################################################################################
#--Autor--		* Nikita Tyunyaev                                                                       #
#               * Arnaud Lahousse                                                                       #
#               * Anicet SIEWE KOUETA 																    #
#																		                                #
#																										#
#--Descroption--																						#
#																										#
#	This program allows to Control and interact with the different mote of the network                  #
#   (Alarm, detector, Lamp and temp)                                                                    #
#   All communications between motes pass through the server, and the forward server to                 #
#   the corresponding nodes.																            #
#########################################################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../os/net/app-layer/packet/packet.h"
#include "listPacket.h"
#include "listDevice.h"
#include <poll.h>
#include <pthread.h>
#include <time.h>

#define PORT 3000
#define MAXLINE 1024

#define SERVERADDRESS "bbbb::c30c:0:0:5"
#define NODE1ADDR "b::1"

//bbbb::1 => serveur addr
char *buffer[MAXLINE];
int sockfd;
int number = 127;
struct sockaddr_in6 nodeAddr;
struct sockaddr_in6 myaddress;
char buf[10];
char buf2[10];
char bufMain[6];
uint8_t msgid = 1;
pkt_t *pkt;
struct pollfd fds[1];
list_t *list;
list_device_t *list_device;
unsigned long start;
bool usedToken[128];
size_t enc_pkt_size = 6;
lamp_types_t post_type;

/*
@param:addr 
function "ipv6_to_str_unexpanded": displays the addresses of motes
*/
void ipv6_to_str_unexpanded(const struct in6_addr *addr)
{
    printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
           (int)addr->s6_addr[0], (int)addr->s6_addr[1],
           (int)addr->s6_addr[2], (int)addr->s6_addr[3],
           (int)addr->s6_addr[4], (int)addr->s6_addr[5],
           (int)addr->s6_addr[6], (int)addr->s6_addr[7],
           (int)addr->s6_addr[8], (int)addr->s6_addr[9],
           (int)addr->s6_addr[10], (int)addr->s6_addr[11],
           (int)addr->s6_addr[12], (int)addr->s6_addr[13],
           (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
}

/*
@param:code 
@param:ack 
@param:payload 
function "composePacket": used to composed Packets
*/
pkt_t *composePacket(pcode_t code, uint8_t ack, char *payload)
{
    pkt_t *new_pkt = pkt_new();
    if (new_pkt == NULL)
    {
        printf("PKT is NULL pointer\n");
    }
    pkt_set_code(new_pkt, code);
    pkt_set_device(new_pkt, SERV);
    pkt_set_ack(new_pkt, ack);
    pkt_set_msgid(new_pkt, msgid);
    pkt_set_payload(new_pkt, (const char *)payload, 2);
    msgid++;
    msgid = msgid % 128;
    return new_pkt;
}


unsigned long timer()
{
    return clock();
}

/*
@param:pkt_routine 
@param:nAddr 
function "ackRoutine": routine implementing the logic when receiving a packet
*/
int ackRoutine(pkt_t pkt_routine, struct sockaddr_in6 *nAddr)
{
    device_t dev = pkt_get_device(&pkt_routine);
    if (pkt_get_ack(&pkt_routine) == 1)
    {

        if ((pkt_get_code(&pkt_routine) == PCODE_GET) && (dev == TEMP))
        {

            const char *payload = pkt_get_payload(&pkt_routine);
            warmer_types_t post_type = payload[0];
            uint8_t temp = payload[1];

            int i = findDevice(list_device, dev, nAddr);
            if (i < 0)
            {
                printf("device not found\n");
            }
            if (post_type == PTYPE_SENS)
            {
                printf("room temperature %d is %u\n", i, temp);
                
            }
            if (post_type == PTYPE_THERM)
            {
                printf("temp of device %d is %u\n", i, temp);
            }
        }
        else if ((pkt_get_code(&pkt_routine) == PCODE_GET) && (dev == DETECTOR))
        {
            const char *payload = pkt_get_payload(&pkt_routine);
            detector_types_t post_type = payload[0];
            int i = findDevice(list_device, dev, nAddr);
            if (post_type == ACTIVATE)
            {
                printf("detector %d is activated \n", i);
            }
            else if (post_type == DESACTIVATE)
            {
                printf("detector %d is deactivated \n", i);
            }


        }

        else if ((pkt_get_code(&pkt_routine) == PCODE_POST) && (dev == LAMP))
        {
            const char *payload = pkt_get_payload(&pkt_routine);
            detector_types_t post_type = payload[0];
            int i = findDevice(list_device, dev, nAddr);
            if (post_type == PTYPE_LIGHT_ON)
            {
                printf("lamp %d is ON \n", i);
            }
            else if (post_type == PTYPE_LIGHT_OFF)
            {
                printf("lamp %d is OFF \n", i);
            }
        }

        else if ((pkt_get_code(&pkt_routine) == PCODE_POST) && (dev == DETECTOR))
        {
            const char *payload = pkt_get_payload(&pkt_routine);
            detector_types_t post_type = payload[0];
            int i = findDevice(list_device, dev, nAddr);
            if (post_type == ACTIVATE)
            {
                printf("detector %d is activated\n", i);
            }
            else if (post_type == DESACTIVATE)
            {
                printf("detector %d is deactivated \n", i);
            }
        }

        else if ((pkt_get_code(&pkt_routine) == PCODE_POST) && (dev == TEMP))
        {
            const char *payload = pkt_get_payload(&pkt_routine);
            uint8_t temperature = payload[0];
            int i = findDevice(list_device, dev, nAddr);
            printf("temperature of the temp %d is %u \n", i, temperature);
        }

        else if ((pkt_get_code(&pkt_routine) == PCODE_ALARM) && (dev == ALARM))
        {
            int i = findDevice(list_device, dev, nAddr);
            printf("alarm %d triggered\n", i);
        }

        else if ((pkt_get_code(&pkt_routine) == PCODE_POST) && (dev == ALARM))
        {
            int i = findDevice(list_device, dev, nAddr);
            printf("alarm %d is OFF \n", i);
        }

        uint8_t id = pkt_get_msgid(&pkt_routine);
        delete (id, list, nAddr);
    }
    else if ((pkt_get_code(&pkt_routine) == PCODE_ALARM) && (dev == DETECTOR))
    {
        int len = lengthDevice(list_device, ALARM);
        for (int i = 0; i < len; i++)
        {
            char empty[2] = "a";
            pkt_t *pkt = composePacket(PCODE_ALARM, 0, (char *)empty);
            pkt_encode(pkt, buf);
            struct sockaddr_in6 *d_addr = sendToDevice(list_device, ALARM, i + 1, buf);
            if (d_addr != NULL)
            {
                
                insertFirst(*pkt, list, d_addr, timer());
            }
        }
    }
    return 0;
}

/*
@param:pktHello 
@param:nAddr 
function "receivHello": function used to handle connection/disconection and refresh of devices
*/
int receivHello(pkt_t pktHello, struct sockaddr_in6 *nAddr)
{

    if (pkt_get_code(&pktHello) == PCODE_HELLO)
    {
        device_t device = pkt_get_device(&pktHello);
        insertLastDevice(list_device, nAddr, device, timer());
    }
    return 0;
}

int handlePacket(char *b, struct sockaddr_in6 *nAddr)
{
    
    pkt_t pktHandle;
    if (PKT_OK != pkt_decode(b, &pktHandle))
    {
        return -1;
    }
    struct sockaddr_in6 *mallocedNodeAddr1 = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
    struct sockaddr_in6 *mallocedNodeAddr2 = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
    memcpy(mallocedNodeAddr1, nAddr, sizeof(struct sockaddr_in6));
    memcpy(mallocedNodeAddr2, nAddr, sizeof(struct sockaddr_in6));
    free(nAddr);
    receivHello(pktHandle, mallocedNodeAddr1);
    ackRoutine(pktHandle, mallocedNodeAddr2);
    return 0;
}

/*
@param:empty 
function "inputThread": Thread that reads the user input in stdin
*/
void *inputThread(void *empty)
{
    char string[256];

    printf("insert your command : \n");
    while (true)
    {   
        gets(string);

        char delim[] = " ";
        char *device = strtok(string, delim);
        char *action = strtok(NULL, delim);
        char *index_c = strtok(NULL, delim);
        int index = 0;
        if (index_c != NULL)
        {
            index = atoi(index_c);
        }

        char *value_c = strtok(NULL, delim);
        int value = 0;
        if (value_c != NULL)
        {
            value = atoi(value_c);
        }
        printf("device : %s, ", device);
        printf("index : %d, ", index);
        printf("action : %s\n", action);
        if (strcmp(device, "lamp") == 0)
        {

            if (strcmp(action, "on") == 0)
            {
                post_type = PTYPE_LIGHT_ON;
                pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, LAMP, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
                
            }
            else if (strcmp(action, "allon") == 0)
            {

                int len = lengthDevice(list_device, LAMP);
                for (int i = 0; i < len; i++)
                {
                    post_type = PTYPE_LIGHT_ON;
                    pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                    pkt_encode(pkt, buf);
                    struct sockaddr_in6 *d_addr = sendToDevice(list_device, LAMP, i + 1, buf);
                    if (d_addr != NULL)
                    {
                        insertFirst(*pkt, list, d_addr, timer());
                    }
                    else
                    {
                        printf("failed to insert\n");
                    }
                }
            }
            else if (strcmp(action, "alloff") == 0)
            {
                int len = lengthDevice(list_device, LAMP);
                for (int i = 0; i < len; i++)
                {
                    post_type = PTYPE_LIGHT_OFF;
                    pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                    pkt_encode(pkt, buf);
                    struct sockaddr_in6 *d_addr = sendToDevice(list_device, LAMP, i + 1, buf);
                    if (d_addr != NULL)
                    {
                        insertFirst(*pkt, list, d_addr, timer());
                    }
                    else
                    {
                        printf("failed to insert\n");
                    }
                }
            }
            else if (strcmp(action, "off") == 0)
            {

                post_type = PTYPE_LIGHT_OFF;
                pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, LAMP, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }
            else{
                printf("Wrong command, retry \n");
            }
        }
        else if ((strcmp(device, "thermostat") == 0))
        {
            if ((strcmp(action, "set") == 0))
            {
                char payload[2];
                payload[0] = (uint8_t)value;
                pkt_t *pkt = composePacket(PCODE_POST, 0, payload);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, TEMP, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }

            else if (((strcmp(action, "getTemp") == 0)))
            {
                warmer_types_t post_type = PTYPE_SENS;
                pkt_t *pkt = composePacket(PCODE_GET, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, TEMP, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }

            else if (((strcmp(action, "getThermostat") == 0)))
            {
                warmer_types_t post_type = PTYPE_THERM;
                pkt_t *pkt = composePacket(PCODE_GET, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, TEMP, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }
            else{
                printf("Wrong command, retry \n");
            }
        }

        else if ((strcmp(device, "detector") == 0))
        {

            if (((strcmp(action, "on") == 0)))
            {

                detector_types_t post_type = ACTIVATE;
                pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, DETECTOR, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }

            else if (((strcmp(action, "off") == 0)))
            {

                detector_types_t post_type = DESACTIVATE;
                pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, DETECTOR, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
                }
            }

            else if (((strcmp(action, "get") == 0)))
            {
                detector_types_t post_type = GET;
                pkt_t *pkt = composePacket(PCODE_GET, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6 *d_addr = sendToDevice(list_device, DETECTOR, index, buf);
                if (d_addr != NULL)
                {
                    insertFirst(*pkt, list, d_addr, timer());
            }
                            }
            else{
            printf("Wrong command, retry \n");
            }
        }

        else if ((strcmp(device, "alarm") == 0) && (strcmp(action, "off") == 0))
        {
            char empty[2] = "a";
            pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)empty);
            pkt_encode(pkt, buf);
            struct sockaddr_in6 *d_addr = sendToDevice(list_device, ALARM, index, buf);
            if (d_addr != NULL)
            {
                insertFirst(*pkt, list, d_addr, timer());
            }
            else{
            printf("Wrong command, retry \n");
            }
        }

        else if ((strcmp(device, "listPackets") == 0))
        {
            printList(list);
            printf("\n");
        }
        else if ((strcmp(device, "listDevices") == 0))
        {
            printListDevice(list_device);
            printf("\n");
        }

        else{
            printf("Wrong command, retry \n");
        }
    }
}

// ################################################# Main #############################################################
//udp server
int main()
{
    for (int i = 0; i < 128; i++)
    {
        usedToken[i] = false;
    }
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    list = init_list(sockfd, 5 * 1000);
    if (list == NULL)
    {
        printf("malloc failed");
    }
    list_device = init_listDevice(sockfd, 10 * 1000);
    if (list == NULL)
    {
        printf("malloc failed");
    }
    memset(&myaddress, 0, sizeof(myaddress));
    myaddress.sin6_family = AF_INET6;
    myaddress.sin6_addr = in6addr_any;
    myaddress.sin6_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&myaddress, sizeof(myaddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    memset(&nodeAddr, 0, sizeof(nodeAddr));
    nodeAddr.sin6_family = AF_INET6;
    nodeAddr.sin6_port = htons(PORT);
    int err;
    err = inet_pton(AF_INET6, NODE1ADDR, &nodeAddr.sin6_addr);
    if (err <= 0)
    {
        printf("error");
    }
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, inputThread, NULL);

    int n, len, rc;
    len = sizeof(nodeAddr);
    while (true)
    {
      
        rc = poll(fds, 1, 50);
        if (rc == -1)
        {
            printf("error");
        }
        if (rc > 0)
        {
            n = recvfrom(sockfd, (char *)bufMain, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&nodeAddr, &len);
            bufMain[n] = '\0';
            
            if (n < 0)
            {
                printf("fail \n");
            }

            struct sockaddr_in6 *mallocedNodeAddr = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
            memcpy(mallocedNodeAddr, &nodeAddr, sizeof(nodeAddr));
            handlePacket(bufMain, mallocedNodeAddr);
        }
        
        // This function is use the retransmit packet if no ack has been received
        // This fonction is located in the file listPacket.c
        reTransmit(list, timer());

        // This function is use the delete device if the server don't receive a hello packet for a while.
        // This fonction is located in the file listDevice.c
        deleteTOutDevice(list_device, timer());
    }
    pthread_join(thread_id, NULL);
    return 0;
}