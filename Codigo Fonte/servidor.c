#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

#define return_size 1024


void error_output(char *error_message)
{
    fprintf(stderr, "\033[31m[+]%s.\n", error_message);
    exit(1);
}

char *compress_buffer(char *buffer)
{
    ulong buffer_size = strlen(buffer) * sizeof(char) + 1;
    ulong destLen = compressBound(buffer_size);

    char *output = (char *)malloc(destLen * sizeof(char));

    int test = compress(output, &destLen, buffer, buffer_size);

    if (test == Z_OK)
        return output;
    else if (test == Z_BUF_ERROR)
        error_output("Could Not Compress Because Buffer Is Too Small");
    else if (test == Z_MEM_ERROR)
        error_output("Could Not Compress Because There Was Not Enough Memory");
    else
        error_output("Could Not Compress");
}

char *uncompress_buffer(char *buffer, ulong original_size, ulong compressed_buffer_size)
{
    char *output = (char *)malloc(original_size * sizeof(char));

    ulong destLen = compressed_buffer_size;

    int test = uncompress(output, &original_size, buffer, compressed_buffer_size);
    if (test == Z_OK)
        return output;
    else if (test == Z_BUF_ERROR)
        error_output("Could Not Uncompress Because Buffer Is Too Small");
    else if (test == Z_DATA_ERROR)
        error_output("Could Not Uncompress Because Data Is Incomplete Or Corrupted");
    else
        error_output("Could Not Uncompress");

    return output;
}

void run(char *command, char *output, int output_size)
{
    printf("%s\n", command);

    FILE *file = popen(command, "r");

    if (file == NULL)
        printf("Unable to Open file");

    int new_size = fread(output, sizeof(char), output_size, file);

    if (ferror(file) != 0)
        printf("Error reading file");
    else
            output[new_size++] = '\0';

        pclose(file);

        if (strcmp(output, "") == 0){
            printf("command doesn't exist.\n");
            bzero(output, output_size);
            strcpy(output, "command doesn't exist.");
        }
}

int main(int argc, char **argv)
{
    char result[return_size];
    char optc = 0;
    int port;
    float version = 0.4;
    int server = 0;
    
    struct option OpcoesLongas[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", no_argument, NULL, 'l'},
        {"versao", no_argument, NULL, 'v'},
        {0, 0, 0, 0}};

    if (argc == 1){ // Sem argumentos
        printf("Parametros faltando\n");
        exit(0);
    }

    while ((optc = getopt_long(argc, argv, "v:p:l", OpcoesLongas, NULL)) != -1)
    {
        switch (optc){
            case 'v': // Ajuda
                printf("Versão %f\n", version);
                exit(0);
            case 'p': // port
                port = atoi(optarg);
                printf("port: %d\n",port);
                break;
            case 'l': // log
                printf("log:\n");
                break;
            default: // Qualquer parametro nao tratado
                printf("Parametros incorretos.\n");
                exit(1);
            }
    }
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

            // Uncompress
            ulong message_size;
            ulong message_byte_size;

            recv(client, &message_size, sizeof(ulong), 0);
            recv(client, &message_byte_size, sizeof(ulong), 0);

            char *buff = (char *)malloc(message_byte_size * sizeof(char));

            recv(client, buff, message_byte_size, 0);
            //printf("msg recebida comprimida %s\n", buff);

            char *buffer_uncompress = uncompress_buffer(buff, message_size, message_byte_size);
            //printf("msg recebida descomprimida %s\n", buffer_uncompress);
            
            run(buffer_uncompress, result, return_size);

            // Compress
            ulong buffer_size = strlen(result) * sizeof(char) + 1;
            ulong buffer_byte_size = compressBound(buffer_size);

            // printf("msg descomprimida enviada %s\n", buffer);
            char *buffer_compress = compress_buffer(result); // chamando função para comprimir/compactar mensagem do buffer
            // printf("msg enviada comprimida %s\n", buffer_compress);

            send(client, &buffer_size, sizeof(ulong), 0);
            send(client, &buffer_byte_size, sizeof(ulong), 0);
            send(client, buffer_compress, buffer_byte_size, 0);

            fflush(stdout);
            //close(client);
        }

        return 0;
    }