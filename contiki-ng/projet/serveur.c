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


// int ackRoutine(pkt_t pkt_routine){
//     printf("hello\n");
//     uint8_t one = 1;
//     printf("receive ack : %u\n", pkt_get_ack(&pkt_routine));
//     if(pkt_get_ack(&pkt_routine)==one){
//         printf("je  suis rentreeeee \n");
//         uint8_t id = pkt_get_msgid(&pkt_routine);
//         printf("j'ai planté ici les mouks \n");
//         uint8_t token = pkt_get_token(&pkt_routine);
//         printf("j'ai planté ici les mouks2 \n");
//         printf("received id : %u\n",id);
//         printf("receive token : %u\n", token);
    
//         delete (id, token, list);
//     }
// }
unsigned long timer(){
    return clock()*1000 /CLOCKS_PER_SEC;
}

int receivHello(pkt_t pktHello,struct sockaddr* nAddr){
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


int handlePacket(char* b,struct sockaddr* nAddr){

    pkt_t pktHandle ;
    if(PKT_OK != pkt_decode(b,&pktHandle)){
        return -1;
    }
    receivHello(pktHandle, nAddr);

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
        char *action = strtok(NULL, delim);
        printf("device : %s\n", device);
        printf("action : %s\n", action);
        if (strcmp(device, "lamp") == 0)
        {
            lamp_types_t post_type = PTYPE_LIGHT_ON;
            pcode_t type = PCODE_POST;

            if (strcmp(action, "turnon") == 0)
            {
                post_type = PTYPE_LIGHT_ON;
                err = inet_pton(AF_INET6, NODE1ADDR, &servaddrToSend.sin6_addr);
                if (err <= 0){
                    printf("error");
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
            


            pkt = pkt_new();

            uint8_t tok = 2;
            pkt_set_token(pkt,tok);

            pkt_set_code(pkt, type);

            pkt_set_payload(pkt, (const char *)&post_type, 2);

            pkt_set_msgid(pkt, msgid);
            msgid++;

            pkt_encode(pkt, buf);
            int n, len, err;
            insertFirst(*pkt,list,servaddrToSend);

            
            err = sendto(sockfd, buf, sizeof(int),
                         MSG_CONFIRM, (const struct sockaddr *)&servaddrToSend,
                         sizeof(nodeAddr));
            if (err < 0)
            {
                perror("Error printed by perror");
            }

        }
        else if((strcmp(device, "list") == 0)){
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

    list = init_list(sockfd,1*1000);
    if(list == NULL){
        printf("malloc failed");
    }
    list_device = init_listDevice(10*1000);
    if(list == NULL){
        printf("malloc failed");
    }

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
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
            // printf("ret %s\n",bufMain);
            handlePacket(bufMain,(struct sockaddr *)&nodeAddr);

        }
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}