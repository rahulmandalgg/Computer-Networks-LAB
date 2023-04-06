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

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct icmp icmp_pkt;

    // create raw socket for ICMP packets
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // zero out the server_addr struct
    memset(&server_addr, 0, sizeof(server_addr));
    
    // set server_addr to IP address of target host
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton() failed");
        exit(EXIT_FAILURE);
    }

    // set up the ICMP packet
    memset(&icmp_pkt, 0, sizeof(icmp_pkt));
    icmp_pkt.icmp_type = ICMP_ECHO; // type of packet we want to send
    icmp_pkt.icmp_code = 0; // code for echo request
    icmp_pkt.icmp_id = htons(getpid() & 0xFFFF); // identifier for this process
    icmp_pkt.icmp_seq = htons(1); // sequence number of packet

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *((struct in_addr *) ip->h_addr);

    // Use traceroute like technique to find the path
    int ttl = 1;
    char hostname[NI_MAXHOST];
    struct sockaddr_in addr;
    char ip_address[INET_ADDRSTRLEN];
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    printf("\nTracing route to %s [%s]\n", hostname, inet_ntoa(dest_addr.sin_addr));
    while (ttl <= MAX_TTL)
    {
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip_address);

        // Send ICMP Echo Request
        icmp_req icmp_request;
        bzero(&icmp_request, sizeof(icmp_req));
        icmp_request.icmp_type = ICMP_ECHO;
        icmp_request.icmp_code = 0;
        icmp_request.icmp_hun.ih_idseq.icd_id = getpid() & 0xFFFF;
        icmp_request.icmp_hun.ih_idseq.icd_seq = ttl & 0xFFFF;
        icmp_request.icmp_cksum = checksum(&icmp_request, sizeof(icmp_req));
        sendto(sockfd, &icmp_request, sizeof(icmp_request), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

        // Receive ICMP Echo Reply or Time Exceeded
        char buffer[IP_MAXPACKET];
        bzero(buffer, IP_MAXPACKET);
        socklen_t len = sizeof(addr);
        int bytes_received = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *) &addr, &len);
        if (bytes_received > 0)
        {
            struct iphdr *ip = (struct iphdr *) buffer;
            inet_ntop(AF_INET, &(ip->saddr), ip_address, INET_ADDRSTRLEN);
            if (strcmp(ip_address, hostname) == 0)
            {
                // Destination reached, print statistics and exit
                print_statistics();
                exit(0);
            }
            else
            {
                // Intermediate node reached, print statistics
                struct timeval now;
                gettimeofday(&now, NULL);
                double rtt = (double) (now.tv_sec - start.tv_sec) * 1000.0 + (double) (now.tv_usec - start.tv_usec) / 1000.0;
                printf("%d  %s (%s)  %.3f ms\n", ttl, hostname, ip_address, rtt);
            }
        }
        else
        {
            printf("%d  *\n", ttl);
        }
        ttl++;
    }
    printf("\nTrace complete.\n");

    close(sockfd);
    return 0;
}
