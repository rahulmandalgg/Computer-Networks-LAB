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
    char msg[1500]="What to submit All the functions should be implemented as a static library called libmsocket.a so that a user can write a C program using these calls for reliable communication and link with it Look up the ar command under Linux to see how to build a static library. Building a library means creating a .h file containing all definitions needed for the library (for ex., among other things, you will #define SOCK_MyTCP here), and a .c file containing the code for the functions in the library. This .c file should not contain any main() function, and will be compiled using ar command to create a .a library file. Thus, you will write the .h header file (name it mysocket.h) and the .c file (name it mysocket.c) from which the .a library file will be generated. Any application wishing to call these functions will include the .h file and link with the libmsocket.a library. For example, when we want to use functions in the math library like sqrt(), we include math.h in our C file, and then link with the math library libm.a. You should submit the following files in a single tar.gz file:";
    printf("SENDING HELLO\n");
    my_send(newsockfd, msg, 1005, 0);
    printf("Message sent\n");
    // my_recv(newsockfd, buffer, 1000, 0);
    // printf("Message received\n");
    // printf("%s", buffer);

    my_close(sockfd);

    return 0;
}