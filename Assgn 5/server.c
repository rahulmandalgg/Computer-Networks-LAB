#include "mysocket.h"

int main()
{
    int ret, sockfd;
    char buffer[1000];

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    my_bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    my_listen(sockfd, 1);

    int client_len = sizeof(client_addr);
    int newsockfd = my_accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    char *msg="Hello";
    my_send(newsockfd, msg, 5, 0);
    my_recv(newsockfd, buffer, 1000, 0);
    printf("%s", buffer);

    my_close(sockfd);

    return 0;
}