#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../os/net/app-layer/packet/packet.h"
#include "../os/net/app-layer/packet/list.h"
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
struct sockaddr_in6 servaddrToSend;
unsigned long start;
bool usedToken[128];
size_t enc_pkt_size = 6;
lamp_types_t post_type;



void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = 0; i < size; i++) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
        printf("\n");
    }
    puts("");
}


unsigned long timer(){
    return clock();
}

int ackRoutine(pkt_t pkt_routine,struct sockaddr_in6* nAddr){
    if(pkt_get_ack(&pkt_routine)==1){
        printf("hello ack\n");
        uint8_t id = pkt_get_msgid(&pkt_routine);
        delete(id, list, nAddr);
    }
}


pkt_t *composePacket(pcode_t code, uint8_t ack, char * payload){
    pkt_t *new_pkt = pkt_new();
    if(new_pkt == NULL){
        printf("PKT is NULL pointer\n");
    }


    pkt_set_code(new_pkt, code);
    pkt_set_device(new_pkt,SERV);
    pkt_set_ack(new_pkt, ack);   
    pkt_set_msgid(new_pkt, msgid);
    pkt_set_payload(new_pkt, (const char *)payload,2);
    msgid++;
    msgid = msgid % 128;
    return new_pkt;

}


int receivHello(pkt_t pktHello,struct sockaddr_in6* nAddr){
    
    if(pkt_get_code(&pktHello) == PCODE_HELLO ){
        device_t device = pkt_get_device(&pktHello);
        insertLastDevice(list_device,nAddr, device,timer());    
    }

}

int handlePacket(char* b,struct sockaddr_in6* nAddr){

    pkt_t pktHandle;
    if(PKT_OK != pkt_decode(b,&pktHandle)){
        return -1;
    }
    struct sockaddr_in6 * mallocedNodeAddr1 = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
    struct sockaddr_in6 * mallocedNodeAddr2 = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
    memcpy(mallocedNodeAddr1,nAddr,sizeof(*nAddr));
    memcpy(mallocedNodeAddr2,nAddr,sizeof(*nAddr));
    free(nAddr);
    receivHello(pktHandle, mallocedNodeAddr1);
    ackRoutine(pktHandle,mallocedNodeAddr2);

}
void *inputThread(void *empty)
{
    char string[256];

    while (true)
    {   
        memset(&servaddrToSend, 0, sizeof(servaddrToSend));
        servaddrToSend.sin6_family = AF_INET6;
        servaddrToSend.sin6_port = htons(PORT);
        int err;
        
        printf("insert your command : \n");
        gets(string);
        int init_size = strlen(string);
        char delim[] = " ";
        char *device = strtok(string, delim);
        char * index_c = strtok(NULL, delim);
        int index = 0;
        if(index_c != NULL){
            index = atoi(index_c);
        }
        char *action = strtok(NULL, delim);
        printf("device : %s\n", device);
        printf("index : %d \n", index);
        printf("action : %s\n", action);
        if (strcmp(device, "lamp") == 0)
        {
            pcode_t type = PCODE_POST;

            if (strcmp(action, "on") == 0)
            {   
                post_type = PTYPE_LIGHT_ON;
                pkt_t *pkt = composePacket(PCODE_POST, 0,(char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6* d_addr = sendToDevice(list_device, LAMP, index,buf);
                if(d_addr != NULL){
                    insertFirst(*pkt,list, d_addr,timer()); 
                }

            }
            else if (strcmp(action, "off") == 0)
            {

                post_type = PTYPE_LIGHT_OFF;
                pkt_t *pkt = composePacket(PCODE_POST, 0, (char *)&post_type);
                pkt_encode(pkt, buf);
                // printBits(5,buf);
                struct sockaddr_in6* d_addr = sendToDevice(list_device, LAMP, index,buf);
                if(d_addr != NULL){
                    insertFirst(*pkt,list,d_addr,timer());
                }
            }
            
            if (err < 0)
            {
                perror("Error printed by perror");
            }

        }
        else if((strcmp(device, "list") == 0)){
            printList(list);
            printf("\n");
        }
        else if((strcmp(device, "listdev") == 0)){
            printListDevice(list_device);
            printf("\n");
        }
    }
}

void ipv6_to_str_unexpanded(const struct in6_addr * addr) {
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

int main()
{
    for(int i = 0;i<128;i++){
        usedToken[i] = false;
    }
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    list = init_list(sockfd,3*1000);
    if(list == NULL){
        printf("malloc failed");
    }
    printf("r_timer : %lu\n", list->r_timer);
    list_device = init_listDevice(sockfd,10*1000);
    if(list == NULL){
        printf("malloc failed");
    }
    memset(&myaddress, 0, sizeof(myaddress));
    myaddress.sin6_family = AF_INET6;
    myaddress.sin6_addr = in6addr_any;
    myaddress.sin6_port = htons( PORT );
    if (bind(sockfd, (struct sockaddr *)& myaddress, sizeof(myaddress))<0)
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
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, inputThread, NULL);

    int n, len, rc;
    len = sizeof(nodeAddr);
    while (true)
    {
        // printf("timer : %lu\n",timer());
        
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
            //this is sketchy
            if(n < 0){
                printf("fail \n");
            }
            // ipv6_to_str_unexpanded(&(nodeAddr.sin6_addr));
            // printf("\n");
            // printf("n: %d\n",n);
            struct sockaddr_in6 * mallocedNodeAddr = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
            memcpy(mallocedNodeAddr,&nodeAddr,sizeof(nodeAddr));
            handlePacket(bufMain,mallocedNodeAddr);
            
        }
        reTransmit(list,timer());
        deleteTOutDevice(list_device,timer());
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}