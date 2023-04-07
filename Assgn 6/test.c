#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>

#define PACKET_SIZE 64
#define MAX_HOPS 10

// Calculate ICMP checksum
unsigned short calculateChecksum(unsigned short *buffer, int length)
{
    unsigned long sum = 0;
    while (length > 1)
    {
        sum += *buffer++;
        length -= 2;
    }

    if (length == 1)
    {
        sum += *(unsigned char *)buffer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <destination_dns>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in destAddr;
    struct sockaddr_in recvAddr;
    socklen_t addrLen = sizeof(recvAddr);
    int sockfd;
    char packet[PACKET_SIZE];
    char recvBuffer[PACKET_SIZE];
    int ttl = 1;
    int maxHops = MAX_HOPS;
    int seq = 0;

    // Resolve destination DNS to IP address
    struct hostent *host = gethostbyname(argv[1]);
    if (host == NULL)
    {
        printf("Failed to resolve destination DNS.\n");
        return 1;
    }
    char resolvedIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, host->h_addr_list[0], resolvedIP, INET_ADDRSTRLEN);

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Set destination address
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    if (inet_aton(resolvedIP, &destAddr.sin_addr) == 0)
    {
        printf("Invalid destination IP address.\n");
        return 1;
    }

    printf("Traceroute to %s (%s)\n", argv[1], resolvedIP);

    while (1)
    {
        // Set TTL in IP header
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        {
            perror("setsockopt");
            return 1;
        }

        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            perror("Failed to set socket options");
            exit(1);
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Initialize ICMP packet
        struct icmp *icmpHeader = (struct icmp *)packet;
        memset(packet, 0, PACKET_SIZE);
        icmpHeader->icmp_type = ICMP_ECHO;
        icmpHeader->icmp_code = 0;
        icmpHeader->icmp_id = getpid() & 0xFFFF;
        icmpHeader->icmp_seq = seq++;
        icmpHeader->icmp_cksum = 0;
        icmpHeader->icmp_cksum = calculateChecksum((unsigned short *)icmpHeader, PACKET_SIZE);

        // Send ICMP packet
        if (sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) == -1)
        {
            perror("sendto");
            return 1;
        }

        
        int select_retval;
        select_retval = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (select_retval == -1)
        {
            perror("Failed to select");
            return 1;
        }
        else if (select_retval == 0)
        {
            printf("%d. *\n", ttl);
            ttl++;
            if (ttl >= maxHops)
            {
                printf("Maximum hops reached\n");
                break;
            }
            continue;
        }

        // Wait for ICMP Echo Reply
        if (recvfrom(sockfd, recvBuffer, PACKET_SIZE, 0, (struct sockaddr *)&recvAddr, &addrLen) == -1)
        {
            perror("recvfrom");
            return 1;
        }

        // Extract IP header and ICMP header from received packet
        struct ip *ipHeader = (struct ip *)recvBuffer;
        struct icmp *recvIcmpHeader = (struct icmp *)(recvBuffer + (ipHeader->ip_hl << 2));

        // Extract source IP address from received packet
        char srcAddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(recvAddr.sin_addr), srcAddr, INET_ADDRSTRLEN);

        // Print hop information
        printf("%d. %s\n", ttl, srcAddr);

        // Check if the received packet is an ICMP Echo Reply
        if (recvIcmpHeader->icmp_type == ICMP_ECHOREPLY)
        {
            printf("Reached destination\n");
            break;
        }
        // Check if maximum hops reached
        if (ttl >= maxHops)
        {
            printf("Maximum hops reached\n");
            break;
        }

        // Increment TTL for next hop
        ttl++;
    }

    // Close socket
    close(sockfd);

    return 0;
}