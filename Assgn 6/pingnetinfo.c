#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("Usage: %s <host name>");
        exit(1);
    }
    struct hostent *h = gethostbyname(argv[1]);

    if (h == NULL)
    {
        perror("Could not resolve host name");
        exit(1);
    }

    struct in_addr dest_ip = *(struct in_addr *)h->h_addr_list[0];

    printf("IP for %s: %s\n\n", argv[1], inet_ntoa(dest_ip));

}