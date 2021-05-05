#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "/home/lahousse/contiki-ng/mobile/contiki-ng/os/net/app-layer/packet/packet.h"
  
#define PORT 3000
#define MAXLINE 1024

#define SERVERADDRESS "bbbb::c30c:0:0:5"
#define NODE1ADDR "bbbb::c30c:0:0:1"
#define NODE2ADDR "bbbb::c30c:0:0:2"
#define NODE3ADDR "bbbb::c30c:0:0:3"
#define NODE4ADDR "bbbb::c30c:0:0:4"

  
// Driver code
int main() {
    char *buffer[MAXLINE];
    int sockfd;
    char* data = "hello";
    int number = 127;
    struct sockaddr_in6 servaddr;
    pkt_t* pkt = pkt_new();
    char buf[5];
    post_types_t post_type = PTYPE_LIGHT_OFF;
    uint16_t temp = 18;
    ptypes_t type = PTYPE_GET;
    warmer_types_t warm = PTYPE_THERM;
    const uint8_t msgid = 1;
    

    if(PKT_OK != pkt_set_type(pkt,type)){
        return -1;
    }

    if(PKT_OK != pkt_set_payload(pkt, (const char*)&warm,2)){
        return -1;
    }
 
    if(PKT_OK != pkt_set_msgid(pkt,msgid)){
        return -1;
    }

    if(PKT_OK != pkt_encode(pkt, buf)){
        return -1;
    }
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);
    int err;
    err = inet_pton(AF_INET6,NODE1ADDR,&servaddr.sin6_addr);
    if(err <= 0){
        printf("error");
    }
    int n, len;
  


    err = sendto(sockfd, buf, sizeof(int),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));

    if(err < 0){
        perror("Error printed by perror");
    }

    printf("Hello message sent.\n");
       
    // n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
    //             MSG_WAITALL, (struct sockaddr *) &servaddr,
    //             &len);
    // buffer[n] = '\0';
    printf("Hello message sent1.\n");
    n = recvfrom(sockfd, buf, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    
    // buffer[n] = '\0';
    // printf("Server : %s\n", *buffer);
    
    if(PKT_OK != pkt_decode( buf,5,pkt)){
        return -1;
    }
    uint16_t yoland = (uint16_t)*pkt_get_payload(pkt);
    printf("la température est de : %u \n", yoland);
  
    close(sockfd);
    return 0;
}