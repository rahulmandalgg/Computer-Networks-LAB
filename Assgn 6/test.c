#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    char *hostname = "www.facebook.com";
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) // get the host info        
    {
        herror("gethostbyname");
        return 1;
    }

    printf("Official name: %s\n", he->h_name); // print the official name
    printf("IP addresses:\n");

    addr_list = (struct in_addr **)he->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++)
    {                                               // loop through all the IP addresses
        printf("\t%s\n", inet_ntoa(*addr_list[i])); // convert the IP address to a string and print it
    }

    return 0;
}
