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

void send_request(const char *method, const char *url, const char *headers, int content_length, const char *content_type, const char *content);
char *get_inpt();
char *getFileType(char *file);

int main()
{
    char *temp;
    char *req = (char *)malloc(sizeof(char) * MAX_REQUEST_LEN);
    char *getheader = (char*)malloc(sizeof(char)*MAX_REQUEST_LEN);
    char *putheader = "Host: www.ag.com\r\nConnection: Keep-Alive\r\nAccept: text/plain\r\nAccept-Language: en-US\r\nContent-Type: text/plain\r\n";
    char *mthd = (char *)malloc(sizeof(char) * 3);
    // send_request("GET", "/", headers, 0, "", "");
    // send_request("GET", "/forbidden", headers, 0, "", "");
    // send_request("GET", "/not_found", headers, 0, "", "");
    // send_request("GET", "/~agupta/networks/index.html", headers, 0, "text/plain", "Hello, World!");

    int i = 0;
    while (1)
    {
        printf("MyBrowser> ");
        req = get_inpt();
        // printf("%s\n", req);

        for (int i = 0; i < 3; i++)
        {
            mthd[i] = req[i];
        }
        // printf("%s", mthd);

        if ((temp = strstr(req, "http://")) != NULL)
        {
            req = temp;
            req += 7;
        }

        //printf("%s\n",req);

        if (strcmp(mthd, "GET") == 0)
        {
            // printf("ITS GET\n");
            send_request("GET", req , getheader, 0, "", "");
        }
        else if (strcmp(mthd, "PUT") == 0)
        {
            // printf("ITS PUT\n");
            send_request("PUT", req , putheader, 13, "text/plain", "Hello, World!");
        }
        else
        {
            printf("Invalid request\n");
        }
    }

    return 0;
}

char *get_inpt()
{
    char *req = (char *)malloc(sizeof(char) * MAX_REQUEST_LEN);
    int i = 0;
    while (1)
    {
        char c = getchar();
        if (c == '\n')
            break;
        req[i] = c;
        i++;
    }

    return req;
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
  else return "text/*";
}

void send_request(const char *method, const char *url, const char *headers, int content_length, const char *content_type, const char *content)
{
    char *typ;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char *got;
    char *ip=(char *)malloc(sizeof(char)*20);
    if (client_socket < 0)
    {
        printf("Error creating client socket\n");
        return;
    }
    if(strcmp(method,"GET")==0)
    {
        int i=0;
        while(url[i] != '/')
        {
            ip[i]= url[i];
            i++;
        }
        url = url + i;
        //printf("%s\n",url);
        char *type=url + strlen(url) -5;
        typ=getFileType(type);

        sprintf(headers, "Host: %s\r\nConnection: close\r\nDate:\r\nAccept: %s\r\nAccept-Language: en-US, en;q=0.9\r\nContent-length: 0\r\n", ip,typ);

        //printf("%s\n",headers);

        char request[MAX_REQUEST_LEN];
        sprintf(request, "%s %s HTTP/1.1\r\n%s\r\n", method, url, headers);
        //printf("%s", request);
    }
    else if(strcmp(method,"PUT")==0)
    {
        //need to implement
    }

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

    char request[MAX_REQUEST_LEN];
    
    // if (content_length > 0)
    // {
    //     sprintf(request, "%sContent-Length: %d\r\n", request, content_length);
    //     sprintf(request, "%sContent-Type: %s\r\n", request, content_type);
    //     sprintf(request, "%s\r\n%s", request, content);
    // }

    printf("%s", request);
    send(client_socket, request, strlen(request), 0);

    char response[100];
    int flag = 0;
    int k=0;
    got = (char *)malloc((100) * sizeof(char));
    // recv(client_socket, got, MAX_REQUEST_LEN - 1, 0);
    // strcpy(response,got);
    // recv(client_socket, got, MAX_REQUEST_LEN - 1, 0);
    // strcat(response, got);

    while(1)
    {
        bzero(response,100);
        int bytes_received = recv(client_socket, response, 100, 0);
        if(bytes_received == 0) break;
        else 
        {
            got=(char *)realloc(got,(k+100)*sizeof(char));
        }
        // printf("%d",bytes_received);
        
        for (int i = 0; i < 100; i++)
        {
            got[k] = response[i];
            k++;
        }
        
    }

    got[k] = '\0';
    printf("%s\n", got);

    close(client_socket);
}
