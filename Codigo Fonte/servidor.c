#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
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
    char buffer[1024] = "Ola cliente";
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

//Uncompress
        ulong message_size;
        ulong message_byte_size;
        
        recv(client, &message_size, sizeof(ulong), 0);
        recv(client, &message_byte_size, sizeof(ulong), 0);

        char* buff = (char*) malloc(message_byte_size * sizeof(char));

        recv(client, buff, message_byte_size, 0);
        //printf("msg recebida comprimida %s\n", buff);

        char* buffer_uncompress = uncompress_buffer(buff, message_size, message_byte_size);
        //printf("msg recebida descomprimida %s\n", buffer_uncompress);

//Compress
        ulong buffer_size = strlen(buffer) * sizeof(char) + 1;
        ulong buffer_byte_size = compressBound(buffer_size);

        //printf("msg descomprimida enviada %s\n", buffer);           
        char* buffer_compress = compress_buffer(buffer);             //chamando função para comprimir/compactar mensagem do buffer
        //printf("msg enviada comprimida %s\n", buffer_compress);

        send(client, &buffer_size, sizeof(ulong), 0);
        send(client, &buffer_byte_size, sizeof(ulong), 0);
        send(client, buffer_compress, buffer_byte_size, 0);

        fflush(stdout);

        close(client);
    }

    return 0;
}