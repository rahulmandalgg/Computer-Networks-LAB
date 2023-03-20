#include "mysocket.h"

int main ()
{
    int ret, sockfd;
    char buffer[1000];

    struct sockaddr_in server_addr;
    sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);


    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(buffer, 1000);

    my_connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // printf("RETURNED FROM MY_CONNECT");

    my_recv(sockfd, buffer, 1090, 0);
    buffer[strlen(buffer)] = '\0';
    printf("MESSAGE RECVD\n");
    printf("MSG:%s\n", buffer);

    bzero(buffer,1000);
    my_recv(sockfd, buffer, 10, 0);
    buffer[strlen(buffer)] = '\0';
    printf("MESSAGE2 RECVD\n");
    printf("MSG:%s\n", buffer);
    bzero(buffer,1000);
    strcpy(buffer,"HELLO SERVER\0");
    my_send(sockfd, buffer, 12, 0);
    printf("MESSAGE SENT\n");
    
    my_close(sockfd);

}
