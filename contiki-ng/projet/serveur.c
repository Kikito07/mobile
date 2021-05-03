#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "/mnt/C072C89972C89616/school/embedded/mobile/contiki-ng/os/net/app-layer/packet/packet.h"
  
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
    post_types_t post_type = PTYPE_LIGHT_ON;
    ptypes_t type = PTYPE_POST;
    const uint8_t msgid = 1;
    
    while(true){
        char string [256];
        printf ("Insert your command : \n");
        gets (string);
        printf ("Your address is: %s\n",string);

        if(strcmp(string,"test") == 0){
            if(PKT_OK != pkt_set_type(pkt,type)){
                return -1;
            }

            if(PKT_OK != pkt_set_payload(pkt, (const char*)&post_type,2)){
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
                
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                        MSG_WAITALL, (struct sockaddr *) &servaddr,
                        &len);
            buffer[n] = '\0';
            printf("Server : %s\n", buffer);
        
            close(sockfd);

                }
    }
return 0;
}