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
  

    my_recv(sockfd, buffer, 1000, 0);
    my_send(sockfd, "Hello", 5, 0);
    printf("%s", buffer);

    my_close(sockfd);

}
