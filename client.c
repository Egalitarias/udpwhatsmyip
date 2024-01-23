#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fcntl.h>

#define BUFFER_LEN 100
#define SLEEP_LEN 50000

int main(int argc, const char *argv[]) {
    const char *server;
    int port;
    int delay;

    switch(argc) {
        case 1: {
            printf("%s SERVER [PORT]\n", argv[0]);

            exit(0);
        }
        case 2: {
            server = argv[1];
            port = 8000;
            delay = 0;

            break;
        }
        case 3: {
            server = argv[1];
            port = atoi(argv[2]);
            delay = 0;

            break;
        }
        default: {
            printf("%s SERVER [PORT]\n", argv[0]);

            exit(0);
        }
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd <= 0) {
        printf("Socket() failed\n");

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

    struct sockaddr_in sockaddr;
    bzero(&sockaddr, sizeof(sockaddr));	
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
    unsigned int serverAddr;
    unsigned int buffer[4];
    if(sscanf(server, "%u.%u.%u.%u", &buffer[0], &buffer[1], &buffer[2], &buffer[3]) == 4) {
        sockaddr.sin_addr.s_addr = inet_addr(server);
    } else {
        struct hostent* host = gethostbyname(server);
        if (host != NULL) {
            in_addr_t addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
            sockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
        } else {
            printf("No server\n");
            close(sockfd);

            exit(0);
        }
    }
    
    serverAddr = sockaddr.sin_addr.s_addr;
    char response[BUFFER_LEN];
    struct sockaddr_in recvSockAddr;
    socklen_t sockLen = sizeof(recvSockAddr);
    int recvLen;

    do {
        if(sendto(sockfd, "\n", 1, 0, (const struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 1) {
            usleep(SLEEP_LEN);
            continue;
        }

        if ((recvLen = recvfrom(sockfd, response, BUFFER_LEN, 0, (struct sockaddr *) &recvSockAddr, &sockLen)) > 0) {
            if (serverAddr == recvSockAddr.sin_addr.s_addr) {
                response[recvLen] = 0;
                printf("%s\n", response);
            } else {
                recvLen = 0;
            }
        }

        if (recvLen <= 0) {
            usleep(SLEEP_LEN);
        }
        printf("retry\n");
    } while (recvLen <= 0);

    close(sockfd);

    return 0;
}