#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "/mnt/C072C89972C89616/school/embedded/mobile/contiki-ng/os/net/app-layer/packet/packet.h"
#include <poll.h>
#include <pthread.h>

#define PORT 3000
#define MAXLINE 1024

#define SERVERADDRESS "bbbb::c30c:0:0:5"
#define NODE1ADDR "bbbb::c30c:0:0:1"
#define NODE2ADDR "bbbb::c30c:0:0:2"
#define NODE3ADDR "bbbb::c30c:0:0:3"
#define NODE4ADDR "bbbb::c30c:0:0:4"

char *buffer[MAXLINE];
int sockfd;
int number = 127;
struct sockaddr_in6 servaddr;
char buf[5];
post_types_t post_type = PTYPE_LIGHT_ON;
ptypes_t type = PTYPE_POST;
const uint8_t msgid = 1;
pkt_t *pkt;
struct pollfd fds[10];

void *inputThread(void *empty)
{
    char string[256];

    while (true)
    {
        printf("Insert your command : \n");
        gets(string);
        printf("Your message is: %s\n", string);
        pkt = pkt_new();
        if (strcmp(string, "test") == 0)
        {
            pkt_set_type(pkt, type);

            pkt_set_payload(pkt, (const char *)&post_type, 2);

            pkt_set_msgid(pkt, msgid);

            pkt_encode(pkt, buf);

            int n, len, err;

            err = sendto(sockfd, buf, sizeof(int),
                         MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                         sizeof(servaddr));
            if (err < 0)
            {
                perror("Error printed by perror");
            }

            printf("Hello message sent.\n");

        }
    }
}

int main()
{

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);
    int err;
    err = inet_pton(AF_INET6, NODE1ADDR, &servaddr.sin6_addr);
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
        printf("hello\n");
        rc = poll(fds, 1, 0);
        if (rc == -1)
        {
            printf("error");
        }
      
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        printf("%s\n",buffer);
        
    }
    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    return 0;
}