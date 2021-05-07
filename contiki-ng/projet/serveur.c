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
#include <poll.h>
#include <pthread.h>

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
struct sockaddr_in6 servaddrToSend;


int ackRoutine(pkt_t pkt_routine){
    printf("hello\n");
    uint8_t one = 1;
    printf("receive ack : %u\n", pkt_get_ack(&pkt_routine));
    if(pkt_get_ack(&pkt_routine)==one){
        printf("je  suis rentreeeee \n");
        uint8_t id = pkt_get_msgid(&pkt_routine);
        printf("j'ai planté ici les mouks \n");
        uint8_t token = pkt_get_token(&pkt_routine);
        printf("j'ai planté ici les mouks2 \n");
        printf("received id : %u\n",id);
        printf("receive token : %u\n", token);
    
        delete (id, token, list);
    }
}


int handlePacket(char* b){

    pkt_t pktHandle ;
    if(PKT_OK != pkt_decode(b,5,&pktHandle)){
        return -1;
    }
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
        char *action = strtok(NULL, delim);
        printf("device : %s\n", device);
        printf("action : %s\n", action);
        if (strcmp(device, "lamp") == 0)
        {
            post_types_t post_type = PTYPE_LIGHT_ON;
            ptypes_t type = PTYPE_POST;

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
            printList(list);
            printf("\n");
        }
    }
}

int main()
{
    list = init_list();
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
            handlePacket(bufMain);

        }
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}