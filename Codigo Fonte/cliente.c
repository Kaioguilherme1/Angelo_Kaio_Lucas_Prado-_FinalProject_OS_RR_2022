
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{

    
    int port = 20;
    char ip[16] = "192.168.1.2";
    char buffer[1024] = {0};
    char message[1024] = "Ola server";


    //dados do cliente é ips
    int client = 0, valread, client_fd;
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)};

    
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    if ((client_fd = connect(client, (struct sockaddr *)&serv_addr,
                             sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // send(client, mensagem, tamanho da mesagem, 0)
    // int = recv(client, mensagem, tamanho da mensagem, 0)

    send(client, message, strlen(message), 0);
    printf("Hello message sent\n");

    valread = read(client, buffer, 1024);
    printf("%s| %d\n", buffer, valread);

    // closing the connected socket
    close(client_fd);
    return 0;
}