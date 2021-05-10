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

//bbbb::1 => serveur addr
char *buffer[MAXLINE];
int sockfd;
int number = 127;
struct sockaddr_in6 nodeAddr;
struct sockaddr_in6 myaddress;
char buf[10];
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
                printBits(5,buf);
                struct sockaddr_in6* d_addr = sendToDevice(list_device, LAMP, index,buf);
                if(d_addr != NULL){
                    insertFirst(*pkt,list,*d_addr);
                }

            }
            
            else if (strcmp(action, "turnoff") == 0)
            {

                post_type = PTYPE_LIGHT_OFF;
                pkt_t *pkt = composePacket(PCODE_POST, 0, QUERY, 1, 1, (char *)&post_type);
                pkt_encode(pkt, buf);
                printBits(5,buf);
                struct sockaddr_in6* d_addr = sendToDevice(list_device, LAMP, index,buf);
                if(d_addr != NULL){
                    insertFirst(*pkt,list,*d_addr);
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

void ipv6_to_str_unexpanded(char * str, const struct in6_addr * addr) {
   sprintf(str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
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
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

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
            struct sockaddr_in6 * mallocedNodeAddr = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
            if(mallocedNodeAddr == NULL){
                printf("malloced failed\n");
            }
            n = recvfrom(sockfd, (char *)bufMain, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)mallocedNodeAddr, &len);
            bufMain[n] = '\0';
            
            if(n>0){
                handlePacket(bufMain,mallocedNodeAddr);
            }
            else{
                free(mallocedNodeAddr);
            }
            
        }
        reTransmit(list,timer());
        // deleteTOutDevice (list_device,timer());
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}