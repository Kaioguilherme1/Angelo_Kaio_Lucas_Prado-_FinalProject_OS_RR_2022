#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "zlib.h"

void error_output(char* error_message) {
    fprintf(stderr, "\033[31m[+]%s.\n", error_message);
    exit(1);
}

char* compress_buffer(char* buffer) {
    ulong buffer_size = strlen(buffer) * sizeof(char) + 1;
    ulong destLen = compressBound(buffer_size);

    char* output = (char*)malloc(destLen * sizeof(char));

    int test = compress(output, &destLen, buffer, buffer_size);
    
    if (test == Z_OK) return output;
    else if (test == Z_BUF_ERROR) error_output("Could Not Compress Because Buffer Is Too Small");
    else if (test == Z_MEM_ERROR) error_output("Could Not Compress Because There Was Not Enough Memory");
    else error_output("Could Not Compress");
}

char* uncompress_buffer(char* buffer, ulong original_size, ulong compressed_buffer_size) {
    char* output = (char*)malloc(original_size * sizeof(char));

    ulong destLen = compressed_buffer_size;

    int test = uncompress(output, &original_size, buffer, compressed_buffer_size);

    if (test == Z_OK) return output;
    else if (test == Z_BUF_ERROR) error_output("Could Not Uncompress Because Buffer Is Too Small");
    else if (test == Z_DATA_ERROR) error_output("Could Not Uncompress Because Data Is Incomplete Or Corrupted");
    else error_output("Could Not Uncompress");

    return output;
}

int main(int argc, char const *argv[])
{

    int port = 20;
    char ip[16] = "192.168.1.3";
    char buffer[1024] = {0};
    char message[1024] = "Ola server";

    // dados do cliente é ips
    int server;
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
    };

//Compress
    ulong message_size = strlen(message) * sizeof(char) + 1;
    ulong message_byte_size = compressBound(message_size);

    //printf("msg descomprimida enviada %s\n", message);
    char* message_compress = compress_buffer(message);           //chamando função para comprimir/compactar mensagem do buffer
    //printf("msg enviada comprimida %s\n", message_compress);

    send(client, &message_size, sizeof(ulong), 0);
    send(client, &message_byte_size, sizeof(ulong), 0);
    send(client, message_compress, message_byte_size, 0);

//Uncompress
    ulong buffer_size;
    ulong buffer_byte_size;

    recv(client, &buffer_size, sizeof(ulong), 0);
    recv(client, &buffer_byte_size, sizeof(ulong), 0);

    char* tmp = (char*)malloc(buffer_byte_size * sizeof(char));

    recv(client, tmp, buffer_byte_size, 0);
    //printf("msg recebida comprimida %s\n", tmp);

    char* message_uncompress = uncompress_buffer(tmp, buffer_size, buffer_byte_size);
    //printf("msg recebida descomprimida %s\n", message_uncompress);


    // closing the connected socket
    close(client_fd);
    return 0;
}