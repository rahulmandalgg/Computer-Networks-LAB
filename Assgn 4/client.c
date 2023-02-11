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

void send_request(const char *method, const char *url);
char *get_inpt();
char *getFileType(char *file);

int main()
{
    char *temp;

    // char *putheader = "Host: www.ag.com\r\nConnection: Keep-Alive\r\nAccept: text/plain\r\nAccept-Language: en-US\r\nContent-Type: text/plain\r\n";
    char *mthd = (char *)malloc(sizeof(char) * 3);

    int i = 0;
    while (1)
    {
    
        char reql[200];
        printf("MyBrowser> ");

        scanf("%[^\n]s", reql);
        getchar();
        
        //printf("%s\n", reql);
        
        char *req = reql;

        //printf("%s\n", req);

        for (int i = 0; i < 3; i++)
        {
            mthd[i] = req[i];
        }
        //printf("%s\n", mthd);

        if ((temp = strstr(req, "http://")) != NULL)
        {
            req = temp;
            req += 7;
        }

        //printf("%s\n",req);

        if (strcmp(mthd, "GET") == 0)
        {
            // printf("ITS GET\n");
            send_request("GET", req);
        }
        else if (strcmp(mthd, "PUT") == 0)
        {
            // printf("ITS PUT\n");
            // send_request("PUT", req , putheader, 13, "text/plain", "Hello, World!");
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
    else if ((temp = strstr(file, ".jpeg")) != NULL)
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

        //printf("%s\n",headers);
        //printf("Requesting %s from %s\n", url, ip);
        sprintf(request, "%s %s HTTP/1.1\r\n%s\r\n", method, url, headers);
        printf("%s", request);
    }
    else if (strcmp(method, "PUT") == 0)
    {
        // need to implement
    }
    int i = 0;
    char *filename = type;
    while (filename[i] != '/')
        filename--;
    filename++;
    // printf("%s\n",filename);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(ip);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error connecting to server\n");
        return;
    }

    printf("Connected to server\n");

    send(client_socket, request, strlen(request), 0);

    char response[100];
    int flag = 0;
    int k = 0;

    got = (char *)malloc(sizeof(char) * 100);

    while (1)
    {
        bzero(response, 100);
        int bytes_received = recv(client_socket, response, 100, 0);

        for (int i = 0; i < 100; i++)
        {

            got[k] = response[i];
            k++;
            if (response[i] == '\r' && response[i + 1] == '\n' && response[i + 2] == '\r' && response[i + 3] == '\n')
            {
                flag = 1;
                break;
            }
        }
        if (flag == 1)
            break;
        else
            got = (char *)realloc(got, sizeof(char) * (k + 100));
    }

    //printf("%s\n", got);
    while (response[i - 3] != '\r' && response[i - 2] != '\n' && response[i - 1] != '\r' && response[i] != '\n')
        i++;
    // printf("%s",(response+i+3));

    FILE *fp = fopen(filename, "w");

    if (fp)
    {
        fprintf(fp, "%s", (response + i + 3));
    }

    while (1)
    {
        bzero(response, 100);
        int bytes_received = recv(client_socket, response, 100, 0);
        if (bytes_received <= 0)
            break;
        fprintf(fp, "%s", response);
        // printf("%s",response);
    }
    fclose(fp);
    // printf("---%d---\n", strlen(got));

    close(client_socket);
}
