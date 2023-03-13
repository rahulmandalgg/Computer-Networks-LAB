#include "mysocket.h"

pthread_mutex_t mutex_RM, mutex_SM;

S_MSG Send_Message[10];
R_MSG Recv_Message[10];
char *send_buffer;
char *recv_buffer;

void *Thread_R(void *arg)
{
    while (1)
    {
        sleep(10);
        int i;
        int sockfd = *(int *)arg;
        int ret;
        int count = 0;
        int contentlength = 0;
        int bytes_received = 0;
        char *ptr;
        while (1)
        {
            pthread_mutex_lock(&mutex_RM);
            for (i = 0; i < 10; i++)
            {
                if (Recv_Message[i].in_use == 1)
                {
                    if (contentlength = ParseH(sockfd))
                    {
                        int bytes = 0;

                        while (bytes_received = recv(sockfd, recv_buffer, 1000, 0))
                        {
                            if (bytes_received == -1)
                            {
                                perror("recieve");
                                exit(1);
                            }

                            strncpy(Recv_Message[i].rmsg + bytes, recv_buffer, bytes_received);
                            bytes += bytes_received;
                            if (bytes == contentlength)
                                break;
                        }
                        fclose(sockfd);
                    }
                }
            }
            pthread_mutex_unlock(&mutex_RM);
            if (count == 10)
            {
                break;
            }
        }
    }
}

void *Thread_S(void *arg)
{
    char msglen[6];

    while (1)
    {

        int i;
        int sockfd = *(int *)arg;
        int ret;
        int count = 0;

        int remlen;

        while (1)
        {
            pthread_mutex_lock(&mutex_SM);
            for (i = 0; i < 10; i++)
            {
                if (Send_Message[i].in_use == 1)
                {
                    Send_Message[i].in_use = 0;
                    remlen = Send_Message[i].length;
                    sprintf(msglen, "%d\r\n", Send_Message[i].length);
                    msglen[strlen(msglen)] = '\0';
                    send(sockfd, msglen, strlen(msglen), 0);
                    while (remlen > 0)
                    {
                        int to_send = (remlen > 1000) ? 1000 : remlen;
                        strncpy(send_buffer, Send_Message[i].smsg + (Send_Message[i].length - remlen), to_send);
                        if (send(sockfd, send_buffer, to_send, 0) < 0)
                        {
                            perror("Send");
                            exit(1);
                        }
                        remlen = remlen - to_send;
                        bzero(send_buffer, 1000);
                    }
                    bzero(Send_Message[i].smsg, 5000);
                }
                bzero(msglen, 6);
                bzero(send_buffer, 1000);
            }
            pthread_mutex_unlock(&mutex_SM);
            if (count == 10)
            {
                break;
            }
        }
        bzero(msglen, 6);
        sleep(10);
    }
}

int my_socket(int domain, int type, int protocol)
{
    if (SOCK_MyTCP != type)
    {
        printf("Error: Only SOCK_MyTCP is supported");
    }

    int sockfd;
    sockfd = socket(domain, SOCK_STREAM, protocol);

    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    send_buffer = (char *)malloc(1000 * sizeof(char));
    recv_buffer = (char *)malloc(1000 * sizeof(char));

    for (int i = 0; i < 10; i++)
    {
        Recv_Message[i].in_use = 0;
        Send_Message[i].in_use = 0;
        Recv_Message[i].rmsg = (char *)malloc(5000 * sizeof(char));
        Send_Message[i].smsg = (char *)malloc(5000 * sizeof(char));
        bzero(Recv_Message[i].rmsg, 5000);
        bzero(Send_Message[i].smsg, 5000);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&R, &attr, Thread_R, (void *)&sockfd);
    pthread_create(&S, &attr, Thread_S, (void *)&sockfd);

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

// my_send call behaves as follows: it puts the message in the Send_Message table if the table has a free entry and returns immediately. If the table does not have a free entry, it gets blocked until an entry is free
size_t my_send(int sockfd, const void *buf, size_t length, int flags)
{
    int i;
    int ret;
    int count = 0;
    while (1)
    {
        pthread_mutex_lock(&mutex_SM);
        for (i = 0; i < 10; i++)
        {
            if (Send_Message[i].in_use == 0)
            {
                Send_Message[i].in_use = 1;
                strncpy(Send_Message[i].smsg, buf, strlen(buf));
                Send_Message[i].smsg[strlen(buf)] = '\0';
                Send_Message[i].length = strlen(buf);
                pthread_mutex_unlock(&mutex_SM);
                return strlen(buf);
            }
        }
        pthread_mutex_unlock(&mutex_SM);
        sleep(2);
    }
}

// my_recv call behaves as follows: it returns the message from the Recv_Message table if the table has a message and returns immediately. If the table does not have a message, it gets blocked until a message is available
size_t my_recv(int sockfd, void *buf, size_t length, int flags)
{
    int i;
    int ret;
    int count = 0;
    while (1)
    {
        pthread_mutex_lock(&mutex_RM);
        for (i = 0; i < 10; i++)
        {
            if (Recv_Message[i].in_use == 1)
            {
                strncpy(buf, Recv_Message[i].rmsg, length);
                Recv_Message[i].in_use = 0;
                bzero(Recv_Message[i].rmsg, 5000);
                Recv_Message[i].length = 0;
                pthread_mutex_unlock(&mutex_RM);
                return (strlen(buf));
            }
        }
        pthread_mutex_unlock(&mutex_RM);
        sleep(2);
    }
}

int my_close(int sockfd)
{
    sleep(5);
    int ret;
    // free tables and kill threads
    pthread_kill(R, SIGHUP);
    pthread_kill(S, SIGHUP);

    ret = close(sockfd);
    if (ret < 0)
    {
        perror("close");
        exit(1);
    }
    return ret;
}

int ParseH(int sockfd)
{
    char c;
    char buff[10] = "", *ptr = buff + 4;
    int bytes_received, status;

    while (bytes_received = recv(sockfd, ptr, 1, 0))
    {
        if (bytes_received == -1)
        {
            perror("Parse Header");
            exit(1);
        }

        if ((ptr[-1] == '\r') && (*ptr == '\n'))
            break;
        ptr++;
    }
    *ptr = '\0';
    ptr[-1] = '\0';
    ptr = buff + 4;
    // printf("%s\n",ptr);
    
    return bytes_received;
}