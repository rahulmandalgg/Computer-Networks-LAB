#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

uint16_t checksum(uint16_t* buffer, int size) {
    uint64_t cksum = 0;
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    if (size) {
        cksum += *(uint8_t*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (uint16_t)(~cksum);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname or IP address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* hostname = argv[1];
    struct hostent* host = gethostbyname(hostname);
    if (host == NULL) {
        perror("Error: could not resolve hostname");
        exit(EXIT_FAILURE);
    }

    // print out the IP address
    printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *(struct in_addr*)host->h_addr_list[0];
    dest_addr.sin_port = htons(2000);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Error: could not create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_addr.s_addr = INADDR_ANY;

    // if (bind(sockfd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
    //     perror("Error: could not bind socket");
    //     exit(EXIT_FAILURE);
    // }

    int ON = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &ON, sizeof(ON)) < 0) {
        perror("Error: could not set IP_HDRINCL");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    struct iphdr* ip_header = (struct iphdr*)buffer;
    struct icmphdr* icmp_header = (struct icmphdr*)(buffer + sizeof(struct iphdr));

    // add data to the icmp header
    char data[56];
    memset(data, 0, sizeof(data));
    strcpy(data, "Hello, world!");
    memcpy((buffer + sizeof(struct iphdr) + sizeof(struct icmphdr)), data, sizeof(data));



    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tos = 0;
    ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(data);
    ip_header->id = rand()%1000;
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_ICMP;
    ip_header->check = 0;
    ip_header->saddr = src_addr.sin_addr.s_addr;
    ip_header->daddr = dest_addr.sin_addr.s_addr;

    ip_header->check = checksum((unsigned short*)ip_header, sizeof(struct iphdr));

    icmp_header->type = ICMP_ECHO;
    icmp_header->code = 0;
    icmp_header->un.echo.id = 0;

    int seq = rand()%1000;
    icmp_header->un.echo.sequence = seq;

    icmp_header->checksum = 0;
    icmp_header->checksum = checksum((unsigned short*)icmp_header, sizeof(struct icmphdr) + sizeof(data));

    // print icmp header info in a formated way
    printf("ICMP header info:\n");
    printf("Type: %d\n", icmp_header->type);
    printf("Code: %d\n", icmp_header->code);
    printf("ID: %d\n", ntohs(icmp_header->un.echo.id));
    printf("Sequence: %d\n", ntohs(icmp_header->un.echo.sequence));
    printf("Checksum: %d\n", ntohs(icmp_header->checksum));
    printf("Data: %s\n", data);


    assert(checksum((unsigned short*)icmp_header, sizeof(struct icmphdr) + sizeof(data)) == 0);
    assert(checksum((unsigned short*)ip_header, sizeof(struct iphdr)) == 0);

    if (sendto(sockfd, buffer, ip_header->tot_len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Error: could not send packet");
        exit(EXIT_FAILURE);
    }

    // receive the reply
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL) < 0) {
        perror("Error: could not receive packet");
        exit(EXIT_FAILURE);
    }

    struct iphdr* ip_reply = (struct iphdr*)buffer;
    struct icmphdr* icmp_reply = (struct icmphdr*)(buffer + sizeof(struct iphdr));

    printf("ICMP reply info:\n");
    printf("Type: %d\n", icmp_reply->type);
    printf("Code: %d\n", icmp_reply->code);
    printf("ID: %d\n", ntohs(icmp_reply->un.echo.id));
    printf("Sequence: %d\n", ntohs(icmp_reply->un.echo.sequence));
    printf("Checksum: %d\n", ntohs(icmp_reply->checksum));
    printf("Data: %s\n", (buffer + sizeof(struct iphdr) + sizeof(struct icmphdr)));

    close(sockfd);




    return 0;
}