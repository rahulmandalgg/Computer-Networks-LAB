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

void log_access(char *client_ip, int client_port, char *method, char *url)
{
    char log_entry[100];
    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    sprintf(log_entry, "%02d%02d%02d:%02d%02d%02d:%s:%d:%s:%s\n",
            time_info->tm_mday, time_info->tm_mon + 1, time_info->tm_year % 100,
            time_info->tm_hour, time_info->tm_min, time_info->tm_sec,
            client_ip, client_port,method,url);

    FILE *fp = fopen("AccessLog.txt", "a");
    fprintf(fp, "%s", log_entry);
    fclose(fp);
}

char* ParseH(int sock)
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
    //printf("%s\n",ptr);
    // printf("bytes recieved: %d\n",bytes_received);
    return ptr;
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

    while(1)
    {
        //printf("Entering loop\n");
        int clientlength = sizeof(clientaddr);
        if ((newsockfd = accept(sockfd, (struct sockaddr *) &clientaddr, &clientlength)) < 0)
        {
			perror("Accept error\n");
			exit(0);
		}
        //printf("Connection accepted\n");
        int recsize,command_length=0;
        char* string;
        string = ParseH(newsockfd);
        char* body;
        printf("%s\n",string);

        char method[20], url[100], headers[500];
        char version[10];

        sscanf(string, "%s %s %s\r\n%[^\r\n]s", method, url, version, headers);
        
        //printf("URL: %s\n", url);
        //printf("ver: %s\n", version);
        //printf("Headers: %s\n", headers);
        //printf("Method: %s\n", method);
        char* type = getFileType(url);
        //printf("file type: %s\n",type);
        char* response;
        char date[30],header_res[500],modif[30];
        struct stat st;
        char *fil = url + 1;

        //printf("%s\n",fil);
        gettime(date,30,3);
        //printf("version:%s\n",version);



        //GET command
        if(strcmp(method,"GET") == 0)
        {
            FILE *file = fopen(fil, "rb");
            if (file == NULL)
            {
                //printf("File Not found\n");
                sprintf(header_res,"%s 404 Not Found\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
                char* content = "Not Found";
                // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 9\r\nLast-Modified: \r\n\r\n%s",header_res,type,content);
                // send(newsockfd, response, strlen(response), 0);
            }
            else
            {   
                if (!stat(fil, &st))
                {
                    //printf("File found\n");
                    sprintf(header_res,"%s 200 OK\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
                    // printf("%s\n",header_res);
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char* filecontent = malloc(fileSize + 1);
                    fread(filecontent, 1, fileSize, file);
                    //printf("%s\n",filecontent);
                    strftime(modif, 100, "%d/%m/%Y %H:%M:%S", localtime( &st.st_mtime));
                    //printf("\nLast modified date and time = %s\n", modif);
                }
                else
                {
                    sprintf(header_res,"%s 403 Forbidden\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
                    // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 9\r\nLast-Modified: \r\n\n%s",header_res,type,content);
                    // send(newsockfd, response, strlen(response), 0);
                }
                // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length:%ld\r\nLast-Modified: %s\r\n\r\n%s",header_res,type,fileSize,modif,filecontent);
                // printf("%s\n",response);
                // send(newsockfd, response, strlen(response), 0);
            }
        }

        // PUT
        else if(strcmp(method,"PUT") == 0)
        {
            FILE *file = fopen(fil, "wb");
            if (file == NULL)
            {
                //printf("File Not found\n");
                sprintf(header_res,"%s 400 Bad Request\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
                char* content = "Bad Request";
                // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 11\r\nLast-Modified: \r\n\r\n%s",header_res,type,content);
                // send(newsockfd, response, strlen(response), 0);
            }
            else
            {   
                //printf("File found\n");
                body = ParseH(newsockfd);
                
                if (!stat(fil, &st))
                {   
                    fwrite(string,sizeof(char),sizeof(string),file);
                    sprintf(header_res,"%s 200 OK\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
                    // printf("%s\n",header_res);
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char* filecontent = malloc(fileSize + 1);
                    fread(filecontent, 1, fileSize, file);
                    //printf("%s\n",filecontent);
                    strftime(modif, 100, "%d/%m/%Y %H:%M:%S", localtime( &st.st_mtime));
                    //printf("\nLast modified date and time = %s\n", modif);
                }
                else
                {

                }
                // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length:%ld\r\nLast-Modified: %s\r\n\r\n%s",header_res,type,fileSize,modif,filecontent);
                // printf("%s\n",response);
                // send(newsockfd, response, strlen(response), 0);
            }
        }

        // other commands
        else
        {
            sprintf(header_res,"%s 400 Bad Request\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-Language: en-US\r\n",version,date);
            char* content = "Bad Request";
                if (!stat(fil, &st))
                {
                    strftime(modif, 100, "%d/%m/%Y %H:%M:%S", localtime( &st.st_mtime));
                    //printf("\nLast modified date and time = %s\n", modif);
                }
            // sprintf(response,"%s\r\nContent-Type: %s\r\nContent-Length: 9\r\nLast-Modified: %s\r\n\n%s",header_res,type,modif,content);
            // send(newsockfd, response, strlen(response), 0);
        }
            log_access(inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),method,url);
        
        close(newsockfd);
    }
}
