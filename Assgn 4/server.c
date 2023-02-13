#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


#define PORT 20000

void separate_request(const char *request, char **method, char **url, char **headers, char **version) {
  char copy[strlen(request) + 1];
  strcpy(copy, request);

  char *token = strtok(copy, " ");
  *method = malloc(strlen(token) + 1);
  strcpy(*method, token);

  token = strtok(NULL, " ");
  *url = malloc(strlen(token) + 1);
  strcpy(*url, token);

  token = strtok(NULL, "\r\n");
  *version = malloc(strlen(token) + 1);
  strcpy(*version, token);

  token = strtok(NULL, "\r");
  *headers = malloc(strlen(token) + 1);
  strcpy(*headers, token);
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

void log_access(char *client_ip, int client_port)
{
    char log_entry[100];
    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    sprintf(log_entry, "%02d%02d%02d:%02d%02d%02d:%s:%d\n",
            time_info->tm_mday, time_info->tm_mon + 1, time_info->tm_year % 100,
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec,
            client_ip, client_port);

    FILE *fp = fopen("AccessLog.txt", "a");
    fprintf(fp, "%s", log_entry);
    fclose(fp);
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

int main()
{
    int sockfd,newsockfd;
    struct sockaddr_in serveraddr,clientaddr;
    char buff[100];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        printf("Socket creation(Server): Failed");
        exit(0);
    }
    printf("Socket creation(Server): Success\n");

    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_port         = htons(PORT);
    serveraddr.sin_addr.s_addr  = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
		printf("Unable to bind local address\n");
		exit(0);
	}
    printf("Binding: Success\n");

    listen(sockfd,5);

    int clientlength = sizeof(clientaddr);
    newsockfd = accept(sockfd, (struct sockaddr *) &clientaddr, &clientlength);
    printf("Connection accepted\n");
    int recsize,command_length=0;
    char* string;
    // strcpy(string,"");
    ParseH(newsockfd);
	// while(recsize = recv(newsockfd, buff, 100, 0))
    // {
        
    //     command_length = command_length + recsize;
    //     strcat(string,buff);
    //     if(buff[recsize-1] == '\0')
    //     {
    //         break;
    //     }
    //     bzero(buff,100);
    // }
    

    char *method, *url, *headers, *version;

    separate_request(string, &method, &url, &headers, &version);
    headers = headers + 1;

    char *temp1 = strrchr(url,'/');
    char *temp2 = strrchr(url,':');
    char *fil;
    strncpy(fil, temp1+1,temp2-temp1);
    char* type = getFileType(url);
    char* response,modif;
    char date[30],header_res[500];
    struct stat st;
    gettime(date,30,3);


    // //GET command
    // if(strcmp(method,"GET") == 0)
    // {
    //     FILE *file = fopen(url, "rb");
    //     if (file == NULL)
    //     {
    //         sprintf(header_res,"%s 404 Not Found\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
    //         char* content = "Not Found";
    //         if (stat(fil, &st) == -1) {
    //             perror("stat");
    //             return 1;
    //         }
    //         sprintf(modif, "%s", ctime(&st.st_mtime));
    //         sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 9\nLast-Modified: %s\r\n\r\n%s",header_res,type,modif,content);
    //         send(newsockfd, response, strlen(response), 0);
    //     }
    //     else
    //     {   
    //         sprintf(header_res,"%s 200 OK\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
    //         fseek(file, 0, SEEK_END);
    //         long fileSize = ftell(file);
    //         fseek(file, 0, SEEK_SET);
    //         char* filecontent = malloc(fileSize + 1);
    //         fread(filecontent, 1, fileSize, file);
    //         if (stat(fil, &st) == -1) {
    //             perror("stat");
    //             return 1;
    //         }
    //         sprintf(modif, "%s", ctime(&st.st_mtime));
    //         sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length:%ld\r\nLast-Modified: %s\r\n\r\n%s",header_res,type,fileSize,modif,filecontent);
    //         send(newsockfd, response, strlen(response), 0);
    //     }
    // }

    // // PUT
    // else if(strcmp(method,"PUT") == 0)
    // {

    // }

    // // other commands
    // else
    // {
    //     sprintf(header_res,"%s 400 Not Bad Request\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
    //     char* content = "Bad Request";
    //     if (stat(fil, &st) == -1)
    //     {
    //         perror("stat");
    //         return 1;
    //     }
    //     sprintf(modif, "%s", ctime(&st.st_mtime));
    //     sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 9\r\nLast-Modified: %s\r\n\n%s",header_res,type,modif,content);
    //     send(newsockfd, response, strlen(response), 0);
    // }
    // log_access(inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    
    close(newsockfd);
}
