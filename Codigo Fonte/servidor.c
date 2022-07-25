
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include "zlib.h"

char *run(char comand[])
{
    FILE *result;
    int i;
    char data, saida[1024];

    result = popen(comand, "r");

    if (result == NULL)
    {
        puts("Unable to open process");
        return ("Unable to open process");
    }

    data = fgetc(result);
    while (data != EOF)
    {
        saida[i] = data;
        i++;
        data = fgetc(result);
    }

    fclose(result);
    return (data);
}

int main()
{
    char buffer[225];
    int port = 20;
    int server = 0;

    // dados do servidor
    int client, valread;

    struct sockaddr_in caddr;
    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)};
    int csize = sizeof caddr;

    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    if (bind(server, (struct sockaddr *)&saddr, sizeof saddr) < 0)
    {
        perror("bind failed");
        return -1;
    }

    if (listen(server, 5) < 0)
    {
        perror("listen");
        return -1;
    }

    while (1)
    {
        client = accept(server, (struct sockaddr *)&caddr, &csize);

        valread = recv(client, buffer, sizeof buffer, 0);

        // send(client, mensagem, tamanho da mesagem, 0)
        // int = recv(client, mensagem, tamanho da mensagem, 0)

        send(client, buffer, valread, 0);

        puts(buffer);
        printf("%s\n", buffer);

        fflush(stdout);

        close(client);
    }

    return 0;
}