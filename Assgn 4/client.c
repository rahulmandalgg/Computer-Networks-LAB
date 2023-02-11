#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 80
#define MAX_REQUEST_LEN 2000

void send_request(const char *method, const char *url,int port);
char *getFileType(char *file);
int ReadHttpStatus(int sock);
int ParseHeader(int sock);

int main()
{
    char *temp,*temp1;

    // char *putheader = "Host: www.ag.com\r\nConnection: Keep-Alive\r\nAccept: text/plain\r\nAccept-Language: en-US\r\nContent-Type: text/plain\r\n";
    char *mthd = (char *)malloc(sizeof(char) * 3);
    int port=0;
    int i = 0;
    while (1)
    {

        char reql[200];
        printf("MyBrowser> ");

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
        //printf("%s\n",req);
        if((temp1 = strstr(req,":")) != NULL )
        {
            //printf("ITS PORT\n");
            //printf("%s\n",temp1);
            *temp1= NULL;
            temp1 = temp1 + 1;
            //printf("%s\n",temp1);
            port = atoi(temp1);
            
        }
        // printf("%s\n",req);
        // printf("%d\n",port);
        // printf("%s\n",req);

        if (strcmp(mthd, "GET") == 0)
        {
            // printf("ITS GET\n");
            send_request("GET", req,port);
        }
        else if (strcmp(mthd, "PUT") == 0)
        {
            // printf("ITS PUT\n");
            send_request("PUT", req,port);
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

void send_request(const char *method, const char *url,int port)
{
    char *typ;
    char *type;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char *got;
    char headers[500];
    char ip[20];
    char request[MAX_REQUEST_LEN];
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
        type = url + strlen(url) - 5;
        typ = getFileType(type);

        sprintf(headers, "Host: %s\r\nConnection: close\r\nDate:\r\nAccept: %s\r\nAccept-Language: en-US, en;q=0.9\r\nContent-length: 0\r\n", ip, typ);

        // printf("%s\n",headers);
        // printf("Requesting %s from %s\n", url, ip);
        sprintf(request, "%s %s HTTP/1.1\r\n%s\r\n", method, url, headers);
        // printf("%s", request);

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        if(port == 0) server_address.sin_port = htons(PORT);
        else server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = inet_addr(ip);

        // printf("PORT: %d\n",htons(server_address.sin_port));

        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("Error connecting to server\n");
            return;
        }

        printf("Connected to server\n");

        i = 0;
        char *filename = type;
        while (filename[i] != '/') filename--;
        filename++;
  

        send(client_socket, request, strlen(request), 0);

        char response[100];
        int flag = 0;
        int k = 0;
        int contentlength = 0;
        int bytes_received = 0;

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
        if(fork()==0)
        {
            execlp("xdg-open", "xdg-open", filename, NULL);
            exit(1);
        }

    }
    else if (strcmp(method, "PUT") == 0)
    {
        
        printf("PUT not implemented yet LOL\n");
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
    
    return bytes_received;
}
