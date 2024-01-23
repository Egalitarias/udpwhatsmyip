#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/errno.h>

#define BUFFER_LEN 1000
#define SLEEP_LEN 50000

int main(int argc, const char *argv[]) {
    const char *server;
    int port;

    switch(argc) {
        case 1: {
            server = NULL;
            port = 8000;

            break;
        }
        case 2: {
            server = NULL;
            port = atoi(argv[1]);

            break;
        }
        case 3: {
            server = argv[1];
            port = atoi(argv[2]);
        
            break;
        }
        default: {
            printf("%s [SERVER] [PORT]\n", argv[0]);

            exit(0);
        }
    }

    struct sockaddr_in sockaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd <= 0) {
        printf("Socket() failed\n");

        exit(0);
    }

    bzero(&sockaddr, sizeof(sockaddr));	
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
    if (server == NULL || strcmp(server, "*") == 0) {
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        int a,b,c,d;
        if(sscanf(server, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
            sockaddr.sin_addr.s_addr = inet_addr(server);
        } else {
            struct hostent* host = gethostbyname(server);
            if (host != 0) {
                sockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
            } else {
                printf("No server\n");
                close(sockfd);
                exit(0);
            }
        }
    }

    if (bind(sockfd, (struct sockaddr* ) &sockaddr, sizeof(sockaddr)) == -1) {
        printf("Address and port unavailable\n");
        close(sockfd);

        exit(0);
    }

    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        printf("File control failed\n");
        close(sockfd);

        exit(0);
    }

    if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        printf("File control failed\n");
        close(sockfd);

        exit(0);
    }

    char buffer[BUFFER_LEN];
    socklen_t sockLen = sizeof(sockaddr);
    int sentLen;
    int totalSent;
    int recvLen;
    int responseLen;
    int retries;
    const char *clientIpAddress;

    do {
        bzero(&sockaddr, sizeof(sockaddr));
        recvLen = recvfrom(sockfd, buffer, BUFFER_LEN, 0, (struct sockaddr *) &sockaddr, &sockLen);

        if (recvLen > 0) {
            clientIpAddress = inet_ntoa(sockaddr.sin_addr);
            if (clientIpAddress != NULL) {
                responseLen = strlen(clientIpAddress);
                totalSent = 0;
                retries = 5;
                do {
                    sentLen = sendto(sockfd, &clientIpAddress[totalSent], responseLen - totalSent, 0, (const struct sockaddr *) &sockaddr, sockLen);
                    if (sentLen > 0) {
                        totalSent += sentLen;
                    } else {
                        --retries;
                    }
                } while (totalSent < responseLen && retries > 0);
            }
        }

        usleep(SLEEP_LEN);
    } while(1);

    close(sockfd);

    return 0;
}