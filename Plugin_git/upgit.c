#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int i=atoi(argv[1]);
    char str[30];
    char str1[20];

    if(i==1)
    {
        system("git add -A");
        system("git commit -m 'joao'");
        system("git push");
        printf("Enviado");
    }
    if(i==2)
    {
        system("git pull");
        printf("recebido");
    }
    return 0;

}
