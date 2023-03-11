#include "mysocket.h"

pthread_mutex_t mutex_RM,mutex_SM;

S_MSG Send_Message[10];
R_MSG Recv_Message[10];
char buffer[1000];

void *Thread_S(void *arg)
{
    while(1)
    {
        int i;
        int sockfd = *(int *)arg;
        int ret;
        int count = 0;
        while(1)
        {
            pthread_mutex_lock(&mutex_SM);
            for(i=0;i<10;i++)
            {
                if(Send_Message[i].in_use == 1)
                {
                    while(1)
                    {
                        
                    }
                }
            }
            pthread_mutex_unlock(&mutex_SM);
            if(count == 10)
            {
                break;
            }
        }
        sleep(10);
    }
}



int my_socket(int domain, int type, int protocol)
{
    if(SOCK_MyTCP != type)
    {
        printf("Error: Only SOCK_MyTCP is supported");
    }
    int sockfd;
    sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }
    return sockfd;
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    ret = bind(sockfd, addr, addrlen);
    if (ret < 0)
    {
        perror("bind");
        exit(1);
    }
    return ret;
}

int my_listen(int sockfd, int num)
{
    int ret;
    ret = listen(sockfd, num);
    if (ret < 0)
    {
        perror("listen");
        exit(1);
    }
    return ret;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;
    ret = accept(sockfd, addr, addrlen);
    if (ret < 0)
    {
        perror("accept");
        exit(1);
    }
    return ret;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    ret = connect(sockfd, addr, addrlen);
    if (ret < 0)
    {
        perror("connect");
        exit(1);
    }
    return ret;
}

size_t my_send(int sockfd, const void *buf, size_t len, int flags)
{
    int ret;
    ret = send(sockfd, buf, len, flags);
    if (ret < 0)
    {
        perror("send");
        exit(1);
    }
    return ret;
}

size_t my_recv(int sockfd, void *buf, size_t len, int flags)
{
    int ret;
    ret = recv(sockfd, buf, len, flags);
    if (ret < 0)
    {
        perror("recv");
        exit(1);
    }
    return ret;
}

int my_close(int sockfd)
{
    int ret;
    ret = close(sockfd);
    if (ret < 0)
    {
        perror("close");
        exit(1);
    }
    return ret;
}