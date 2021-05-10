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
#define NODE1ADDR "bbbb::c30c:0:0:1"
#define NODE2ADDR "bbbb::c30c:0:0:2"
#define NODE3ADDR "bbbb::c30c:0:0:3"
#define NODE4ADDR "bbbb::c30c:0:0:4"

//bbbb::1 => serveur addr
char *buffer[MAXLINE];
int sockfd;
int number = 127;
struct sockaddr_in6 nodeAddr;
struct sockaddr_in6 myaddress;
char buf[5];
char bufMain[5];
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


uint8_t giveToken(){
    int i;
    for(i = 0; i < 128;i++){
        if(usedToken[i]==false){
            break;
        }
    }
    return i;
}

unsigned long timer(){
    return clock()*1000 /CLOCKS_PER_SEC;
}
int ackRoutine(pkt_t pkt_routine){
    
    uint8_t one = 1;

    if(pkt_get_ack(&pkt_routine)==one){
        
        uint8_t id = pkt_get_msgid(&pkt_routine);
        uint8_t token = pkt_get_token(&pkt_routine);
        delete(id, token, list);
    }
}


pkt_t *composePacket(pcode_t code, uint8_t ack, query_t qr, uint8_t msgid, uint8_t token, char * payload){
    pkt_t *new_pkt = pkt_new();
    if(new_pkt == NULL){
        printf("PKT is NULL pointer\n");
    }


    pkt_set_code(new_pkt, code);
    pkt_set_ack(new_pkt, ack);
    pkt_set_query(new_pkt, qr);
    
    pkt_set_msgid(new_pkt, msgid);
    pkt_set_token(new_pkt,token);
    
    size_t size = 1;
    pkt_set_payload(new_pkt, (const char *)payload,2);


    return new_pkt;

}

int receivHello(pkt_t pktHello,struct sockaddr_in6* nAddr){
    uint8_t code = pkt_get_code(&pktHello);
    uint8_t token = pkt_get_token(&pktHello);
    if(code = PCODE_HELLO ){
        if(token == 0){
            //adding to the list of device
            device_t device = pkt_get_device(&pktHello);
            insertLastDevice(list_device,nAddr,token, device,timer());

        }
    }
    else{
        
    }

}


int handlePacket(char* b,struct sockaddr_in6* nAddr){

    pkt_t pktHandle;
    if(PKT_OK != pkt_decode(b,&pktHandle)){
        return -1;
    }
    receivHello(pktHandle, nAddr);
    ackRoutine(pktHandle);

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

            if (strcmp(action, "turnon") == 0)
            {   
                post_type = PTYPE_LIGHT_ON;
                pkt_t *pkt = composePacket(PCODE_POST, 0, QUERY, 1, 1, (char *)&post_type);
                pkt_encode(pkt, buf);
                struct sockaddr_in6* d_addr = sendToDevice(list_device, LAMP, index,buf);
                if(d_addr != NULL){
                    insertFirst(*pkt,list,*d_addr);
                }

            }
            
            else if (strcmp(action, "turnoff") == 0)
            {

                post_type = PTYPE_LIGHT_OFF;

                err = inet_pton(AF_INET6, NODE1ADDR, &servaddrToSend.sin6_addr);
                if (err <= 0){
                    printf("error");
                }
            }
            
            err = sendto(sockfd, buf, enc_pkt_size,
                         MSG_CONFIRM, (const struct sockaddr *)&servaddrToSend,
                         sizeof(nodeAddr));
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

    list = init_list(sockfd,1*1000);
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
    while (true)
    {
        
        rc = poll(fds, 1, 0);
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
            struct sockaddr_in6 * mallocedNodeAddr = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
            memcpy(mallocedNodeAddr,&nodeAddr,sizeof(nodeAddr));
            handlePacket(bufMain,mallocedNodeAddr);
        }
        reTransmit(list,timer());
        // deleteTOutDevice (list_device,timer());
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}