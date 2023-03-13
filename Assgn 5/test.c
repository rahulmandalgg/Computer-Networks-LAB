#include <stdio.h>
#include <string.h>


int main()
{

    char msglen[1000];
    sprintf(msglen,"%d\r\n",60);
    printf("%s\n",msglen);
    msglen[strlen(msglen)] = '\0';
    // printf("%c",msglen[strlen(msglen)+1]);

}