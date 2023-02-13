#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 80
#define MAX_REQUEST_LEN 2000
#define BUFSIZE 2048

void send_request(const char *method, const char *url);
char *getFileType(char *file);
int parseS(int sock);
int ParseH(int sock);
void gettime(char *buff, int len, int ofset);

int main()
{

    // char *putheader = "Host: www.ag.com\r\nConnection: Keep-Alive\r\nAccept: text/plain\r\nAccept-Language: en-US\r\nContent-Type: text/plain\r\n";
    char *mthd = (char *)malloc(sizeof(char) * 3);
    char *temp;
    int i = 0;
    while (1)
    {

        char reql[200];
        printf("MyOwnBrowser> ");

        scanf("%[^\n]s", reql);
        getchar();

        // printf("%s\n", reql);

        char *req = reql;

        // printf("%s\n", req);

        for (int i = 0; i < 3; i++)
        {
            mthd[i] = req[i];
        }
        // printf("%s\n", mthd);

        if ((temp = strstr(req, "http://")) != NULL)
        {
            req = temp;
            req += 7;
        }

        // printf("%d\n",port);
        // printf("%s\n",req);

        if (strcmp(mthd, "GET") == 0)
        {
            // printf("ITS GET\n");
            send_request("GET", req);
        }
        else if (strcmp(mthd, "PUT") == 0)
        {
            // printf("ITS PUT\n");
            send_request("PUT", req);
        }
        else
        {
            printf("Invalid request\n");
        }
        bzero(reql, 200);
    }

    return 0;
}

char *getFileType(char *file)
{
    char *temp;
    if ((temp = strstr(file, ".html")) != NULL)
    {
        return "text/html";
    }
    else if ((temp = strstr(file, ".pdf")) != NULL)
    {
        return "application/pdf";
    }
    else if ((temp = strstr(file, ".jpg")) != NULL)
    {
        return "image/jpeg";
    }
    else
        return "text/*";
}

void send_request(const char *method, const char *url)
{
    char *typ;
    char *type;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char *got;
    char headers[500];
    int port = 0;
    char date[30];
    char ifdate[30];
    char ip[20];
    char request[MAX_REQUEST_LEN];
    char *temp, *temp1;
    bzero(request, MAX_REQUEST_LEN);
    bzero(headers, 500);
    bzero(ip, 20);
    if (client_socket < 0)
    {
        printf("Error creating client socket\n");
        return;
    }

    if (strcmp(method, "GET") == 0)
    {
        if ((temp1 = strstr(url, ":")) != NULL)
        {
            // printf("ITS PORT\n");
            // printf("%s\n",temp1);
            *temp1 = NULL;
            temp1 = temp1 + 1;
            // printf("%s\n",temp1);
            port = atoi(temp1);
        }

        int i = 0;
        while (url[i] != '/')
        {
            ip[i] = url[i];
            i++;
        }
        url = url + i;
        // printf("%s\n",url);
        // printf("%s\n",ip);
        // printf("%s\n",headers);

        // printf("%s\n",url);

        type = url + strlen(url) - 5;
        typ = getFileType(type);
        // printf("%s\n",type);
        gettime(date, 30, 0);
        gettime(ifdate, 30, -2);
        // sprintf(headers, "Host: %s\r\nConnection: close\r\nDate: %s\r\nAccept: %s\r\nAccept-Language: en-US, en;q=0.9\r\nIf-Modified-Since: %s\r\n", ip, date, typ, ifdate);
        sprintf(headers, "Host: %s\r\nConnection: close\r\nDate: \r\nAccept: %s\r\nAccept-Language: en-US, en;q=0.9\r\nIf-Modified-Since: \r\n", ip, typ);
        // printf("%s\n",headers);
        // printf("Requesting %s from %s\n", url, ip);
        sprintf(request, "%s %s HTTP/1.1\r\n%s\r\n", method, url, headers);
        // printf("%s", request);

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        if (port == 0)
            server_address.sin_port = htons(PORT);
        else
            server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = inet_addr(ip);

        // printf("PORT: %d\n",htons(server_address.sin_port));

        struct pollfd fdset[1]; // poll
        int timeout = 3000, ret;
        fdset[0].fd = client_socket;
        fdset[0].events = POLLIN;

        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Error connecting to server\n");
            return;
        }

        printf("Connected to server\n");

        i = 0;
        char *filename = type;
        while (filename[i] != '/')
            filename--;
        filename++;
        // printf("%s\n", filename);
        send(client_socket, request, strlen(request), 0);

        char response[100];
        int flag = 0;
        int k = 0;
        int contentlength = 0;
        int bytes_received = 0;

        int polval = poll(fdset,1,timeout);                                                                                    //polling
        if(polval > 0) 
        {
            if (parseS(client_socket) && (contentlength = ParseH(client_socket)))
            {

                int bytes = 0;
                FILE *fd = fopen(filename, "wb");

                while (bytes_received = recv(client_socket, response, 100, 0))
                {
                    if (bytes_received == -1)
                    {
                        perror("recieve");
                        exit(1);
                    }

                    fwrite(response, 1, bytes_received, fd);
                    bytes += bytes_received;
                    if (bytes == contentlength)
                        break;
                }
                fclose(fd);
            }
            if (fork() == 0)
            {
                execlp("xdg-open", "xdg-open", filename, NULL);
                exit(1);
            }
        }
        else
        {
            printf("Timeout\n");
        }
    }
    else if (strcmp(method, "PUT") == 0)
    {
        char *port1;
        int i = 0;
        char *ptr = url;
        char *filename;
        while (url[i] != '/')
        {
            ip[i] = url[i];
            i++;
        }
        url = url + i;
        i = 0;
        if ((temp = strstr(url, ":")) != NULL)
        {
            port1 = temp + 1;
            *temp = NULL;
            while (temp[i] != ' ')
                i++;
            filename = temp + i + 1;
            temp[i] = '\0';
            temp++;
            port = atoi(temp);
            // printf("%s\n",temp);
            char *tmp = url;
            // printf("%s\n", filename);
            strcat(tmp, "/");
            tmp[strlen(tmp)] = '\0';
            strcat(tmp, filename);
            tmp[strlen(tmp)] = '\0';
            filename = filename - 5;
        }
        else
        {
            while (ptr[i] != ' ')
                i++;
            filename = ptr + i + 1;
            ptr[i] = '/';
        }

        type = url + strlen(url) - 5;
        typ = getFileType(type);
        // printf("%s\n",typ);
        // printf("%d\n",port);
        // printf("%s\n", filename);
        // printf("%s\n", url);

        // printf("%s\n",url);
        // printf("%s\n",ip);
        i = 0;
        // printf("%s\n",url);

        // printf("%s\n", url);

        FILE *file;

        file = fopen(filename, "rb");
        if (file == NULL)
        {
            printf("Error opening file\n");
            return 1;
        }

        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        rewind(file);
        // printf("%d", file_size);

        gettime(date, 30, 0);
        // printf("%s\n", date);
        sprintf(headers, "Host: %s\r\nAccept: %s\r\nConnection: close\r\nDate: %s\r\nContent-length: %d\r\nContent-type: %s\r\nContent-language: en-US\r\n\r\n", ip, typ, date, file_size, typ);

        // printf("%s\n",headers);
        // printf("Requesting %s from %s\n", url, ip);
        sprintf(request, "%s %s HTTP/1.1\r\n%s\r\n", method, url, headers);
        // printf("%s", request);

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        if (port == 0)
            server_address.sin_port = htons(PORT);
        else
            server_address.sin_port = htons(port1);
        server_address.sin_addr.s_addr = inet_addr(ip);

        // printf("PORT: %d\n",htons(server_address.sin_port));

        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Error connecting to server\n");
            return;
        }

        printf("Connected to server\n");

        send(client_socket, request, strlen(request), 0);

        int remaining = file_size;
        char buffer[BUFSIZE];
        while (remaining > 0)
        {
            bzero(buffer, BUFSIZE);
            int to_send = (remaining > BUFSIZE) ? BUFSIZE : remaining;

            fread(buffer, to_send, 1, file);
            // printf("%s\n", buffer);
            if (send(client_socket, buffer, to_send, 0) < 0)
            {
                printf("Send failed");
                return 1;
            }
            remaining -= to_send;
        }

        parseS(client_socket);
        ParseH(client_socket);
    }

    close(client_socket);
}

int parseS(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 1;
    int bytes_received, status;

    while (bytes_received = recv(sock, ptr, 1, 0))
    {
        if (bytes_received == -1)
        {
            perror("Error in Reading HTTP Status");
            exit(1);
        }

        if ((ptr[-1] == '\r') && (*ptr == '\n'))
            break;
        ptr++;
    }
    *ptr = 0;
    ptr = buff + 1;

    sscanf(ptr, "%*s %d ", &status);

    printf("%s\n", ptr);

    return (bytes_received > 0) ? status : 0;
}

int ParseH(int sock)
{
    char c;
    char buff[1024] = "", *ptr = buff + 4;
    int bytes_received, status;

    while (bytes_received = recv(sock, ptr, 1, 0))
    {
        if (bytes_received == -1)
        {
            perror("Parse Header");
            exit(1);
        }

        if ((ptr[-3] == '\r') && (ptr[-2] == '\n') && (ptr[-1] == '\r') && (*ptr == '\n'))
            break;
        ptr++;
    }
    *ptr = 0;
    ptr = buff + 4;
    printf("%s\n",ptr);
    if (bytes_received)
    {
        ptr = strstr(ptr, "Content-Length:");
        if (ptr)
        {
            sscanf(ptr, "%*s %d", &bytes_received);
        }
        else
            bytes_received = -1;
    }
    // printf("bytes recieved: %d\n",bytes_received);
    return bytes_received;
}

void gettime(char *buff, int len, int ofset)
{
    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = gmtime(&current_time);

    time_info->tm_mday += ofset;

    // strftime(time_string, sizeof(time_string), "%a, %d %b %Y %T GMT", time_info);
    current_time = mktime(time_info);
    strftime(buff, len, "%a, %d %b %Y %T GMT", time_info);
}
