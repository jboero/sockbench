#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

extern int opterr;
extern char* optarg;
enum benchmode { server, client };

long buffsize = 32768;
long buffcount = 32768*2;

void* connection_handler(void *sock)
{
    char *sendbuf = malloc(buffsize);
    int fd = *((int*)sock);
    printf("Connection received fd %d\n", fd);
    for (int i = 0; i < buffcount; ++i)
        if(write(fd, sendbuf, buffsize) < 0)
            break;
    
    free(sendbuf);
    close(fd);
    printf("Connection finished %d\n", fd);
    return NULL;
}

long getnsnow()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return t.tv_sec * 1000000000 + t.tv_nsec;
}

int main(int argc, char **argv)
{
    char *p = NULL, *buf = NULL;
    enum benchmode mode;
    int sockfd, tcpport = 0;

    // Signal handlers
    signal(SIGPIPE, SIG_IGN);

    // No loop.  Just one option. Server or client.
    switch(getopt(argc, argv, "s::c::b:q:"))
    {
        case 's':
            mode = server;
            if (optarg)
                tcpport = atoi(optarg);
        break;

        case 'c':
            mode = client;
            if (optarg)
                tcpport = atoi(optarg);
        break;

        case 'b':
            if (optarg)
                buffsize = atoi(optarg);
        break;

        case 'q':
            if (optarg)
                buffcount = atoi(optarg);
        break;

        default :
        printf("Usage:\nServer: sockperf -s[TCP PORT]\nClient: sockperf -c[TCP PORT] [-qQUANTITY] [-bBLOCKSIZE]\n");
        break;
    }

    if (mode == server)
    {
        int client_sock, c, res;
        pthread_t thread_id;
        struct sockaddr_un client, serverun = {AF_UNIX, "sockperf.sock"};
        struct sockaddr_in serverin = {AF_INET, htons(9000), 0};
        serverin.sin_port = htons(tcpport);
        serverin.sin_addr.s_addr = htonl (INADDR_ANY);
        
        sockfd = socket(tcpport ? AF_INET : AF_UNIX, SOCK_STREAM, 0);
        if (!sockfd)
        {
            perror("Error on sockfd.\n");
            return 1;
        }

        if (tcpport) res = bind(sockfd, (struct sockaddr*)&serverin, sizeof(serverin));
        else         res = bind(sockfd, (struct sockaddr*)&serverun, sizeof(serverun));
        if (res < 0)
        {
            printf("Bind failed: %d\n", errno);
            return 1;
        }

        listen(sockfd, 3);

        c = sizeof(struct sockaddr_un);

        printf("Server listening: %s\n", tcpport ? "tcp" : "uds");
        while (client_sock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c))
            if (pthread_create(&thread_id, NULL, connection_handler, (void*)&client_sock) < 0)
                perror("Could not create handler thread.\n");
        
        if (client_sock < 0)
            perror("Accept failed.\n");
    }
    else if (mode == client)
    {
        long start = 0, end = 0, duration = 0, part = 0, res = 0;
        double total = 0;
        struct sockaddr_un serverun = {AF_UNIX, "sockperf.sock"};
        struct sockaddr_in serverin = {AF_INET, htons(9000), 0};
        serverin.sin_port = htons(tcpport);
        serverin.sin_addr.s_addr = htonl(INADDR_ANY);

        sockfd = socket(tcpport ? AF_INET : AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Error connecting to ./sockperf.sock\n");
            return -1;
        }

        if (tcpport) res = connect(sockfd, (struct sockaddr*)&serverin, sizeof(serverin));
        else         res = connect(sockfd, (struct sockaddr*)&serverun, sizeof(serverun));
        if (res < 0)
        {
            perror("Error connecting to socket.\n");
            return -3;
        }
        
        buf = malloc(buffsize);

        start = getnsnow();
        for (int i = 0; i < buffcount; ++i)
        {
            if ((part = read(sockfd, buf, buffsize)) < 0)
            {
                printf("Read problem: %d\n", errno);
                break;
            }
            total += part;
        }
        end = getnsnow();
        
        duration = end - start;
        close(sockfd);
        free (buf);
        printf("%fB received in %dns = %fGB/s", total, duration, total/duration);
    }

    return 0;
}
