#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
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

// delay function
void delay(int milliseconds)
{
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <destination_dns> <no_of_times_to_probe> <time_between_2_probes>\n", argv[0]);
        return 1;
    }

    // Resolve destination DNS to IP address
    struct hostent *host = gethostbyname(argv[1]);
    if (host == NULL)
    {
        printf("Failed to resolve destination DNS.\n");
        return 1;
    }

    // int n = atoi(argv[2]);
    // int t = atoi(argv[3]);

    struct sockaddr_in destAddr;
    struct sockaddr_in recvAddr;
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(recvAddr);
    int sockfd;
    char packet[PACKET_SIZE];
    char data[100];
    char recvBuffer[PACKET_SIZE];
    int ttl = 1;
    int maxHops = MAX_HOPS;
    int seq = 0;
    int typ;
    srand(time(NULL));

    char resolvedIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, host->h_addr_list[0], resolvedIP, INET_ADDRSTRLEN);

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Set source address
    memset(&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = INADDR_ANY;

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
        //-------
        // // Set TTL in IP header
        // if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        // {
        //     perror("setsockopt");
        //     return 1;
        // }

        int ON = 1;
        if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &ON, sizeof(ON)) < 0)
        {
            perror("Error: could not set IP_HDRINCL");
            exit(EXIT_FAILURE);
        }
        //-------

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
        // struct icmp *icmpHeader = (struct icmp *)packet;
        memset(packet, 0, PACKET_SIZE);
        struct iphdr *ipHeader = (struct iphdr *)packet;
        struct icmphdr *icmp_Header = (struct icmphdr *)(packet + sizeof(struct iphdr));

        memset(data, 0, sizeof(data));
        strcpy(data, "Hello World");
        memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, sizeof(data));

        // Set IP header
        ipHeader->ihl = 5;
        ipHeader->version = 4;
        ipHeader->tos = 0;
        ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(data));
        // ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
        ipHeader->id = rand() % 1000;
        ipHeader->frag_off = 0;
        ipHeader->ttl = ttl;
        ipHeader->protocol = IPPROTO_ICMP;
        ipHeader->check = 0;
        ipHeader->saddr = srcAddr.sin_addr.s_addr;
        ipHeader->daddr = destAddr.sin_addr.s_addr;

        ipHeader->check = calculateChecksum((unsigned short *)ipHeader, sizeof(struct iphdr));

        // Set ICMP header
        icmp_Header->type = ICMP_ECHO;
        icmp_Header->code = 0;
        icmp_Header->un.echo.id = 0;
        icmp_Header->un.echo.sequence = seq++;
        icmp_Header->checksum = 0;
        icmp_Header->checksum = calculateChecksum((unsigned short *)icmp_Header, sizeof(struct icmphdr) + sizeof(data));
        // icmp_Header->checksum = calculateChecksum((unsigned short *)icmp_Header, sizeof(struct icmphdr));

        struct timeval start, end;
        gettimeofday(&start, NULL);

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

        gettimeofday(&end, NULL);

        // Extract IP header and ICMP header from received packet
        struct iphdr *ip_recv = (struct iphdr *)recvBuffer;
        struct icmphdr *icmp_recv = (struct icmphdr *)(recvBuffer + 20);

        // Extract source IP address from received packet
        char srcAddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(recvAddr.sin_addr), srcAddr, INET_ADDRSTRLEN);

        // Print hop information
        printf("TTL: %d. IP: %s, ", ttl, srcAddr);
        printf("Type: %d, Checksum: %d\n", icmp_recv->type, icmp_recv->checksum);

        typ = icmp_recv->type;

        // Check if the received packet is an ICMP Echo Reply
        if (typ == ICMP_ECHOREPLY)
        {
            printf("Data: %s\n", recvBuffer + +sizeof(struct iphdr) + sizeof(struct icmphdr));

            break;
        }
        else if (typ == ICMP_TIME_EXCEEDED)
        {
            // do nothing
        }
        else if (typ == ICMP_DEST_UNREACH)
        {
            // do nothing
        }
        else
        {
            // check for data in the packet
            char *ptr = recvBuffer + 20 + sizeof(struct icmphdr);
            if (ptr != NULL)
            {
                if (ip_recv->protocol == IPPROTO_TCP)
                {
                    printf("TCP packet\n");
                }
                else if (ip_recv->protocol == IPPROTO_UDP)
                {
                    printf("UDP packet\n");
                }
                else
                {
                    printf("Unknown Protocol\n");
                }
                printf("Data: %s", ptr);
            }
        }
        printf("\n");

        if (typ == ICMP_ECHOREPLY)
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






