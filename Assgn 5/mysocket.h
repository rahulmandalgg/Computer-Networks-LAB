#ifndef MYSOCKET_H
#define MYSOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#define SOCK_MyTCP 10
pthread_t R, S;

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int num);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
size_t my_send(int sockfd, const void *buf, size_t len, int flags);
size_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int fd);

typedef struct {
    int in_use;
    char *buffer;
}S_MSG;

typedef struct {
    char *buffer;
    int length;
    int in_use;
}R_MSG;


#endif