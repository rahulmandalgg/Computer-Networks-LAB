#include <stdio.h>
#include <string.h>


int main()
{
    char c[1000] = "HELLO";
    char l[1002];
    sprintf(l,"NO%s",c);
    printf("%s",l);
}