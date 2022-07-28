#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "zlib.h"

char *log_filename;
int logfd;
int sockfd;
int logflag = 0;
int cprflag = 0;

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

int main(int argc, char **argv)
{

    char *ip;
    char buffer[1024];
    char message[1024];
    int opt;
    int port;
    int pflag = 0;
    int lflag = 0;
    int stop = 1;
    char *hostname = "localhost";

    struct option longopts[] = {
        {"ip", required_argument, NULL, 'i'},
        {"port", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
        {"hostname", required_argument, NULL, 'h'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}};

    if (argc == 1)
    { // Sem argumentos
        printf("Parametros faltando\n");
        exit(0);
    }

    while ((opt = getopt_long(argc, argv, "i:p:l:h:c", longopts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip = optarg;
            printf("host: %s\n", ip);
            break;
        case 'p':
            port = atoi(optarg);
            printf("port: %d\n", port);
            break;
        case 'l':
            log_filename = optarg;
            logfd = creat(log_filename, S_IRWXU);
            logflag = 1;
            if (logfd < 1)
            {
                fprintf(stderr, "can't open file %s\n", log_filename);
                exit(1);
            }
            lflag = 1;
            break;
        case 'h':
            hostname = optarg;
            break;
        case 'c':
            cprflag = 1;
            break;
        default:
        {
            fprintf(stderr, "unrecognized argument");
            exit(1);
        }
        }
    }
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
    // if (cprflag){
    // send(client, "1", 1, 0);
    while (stop)
    {
        printf("user@%s ~# ", ip);
        fgets(message, 1024, stdin);

        // if(strcmp(message, "exit()")){
        //   printf("exit...");
        //  closing the connected socket
        // close(client_fd);
        //}
        // Compress
        ulong message_size = strlen(message) * sizeof(char) + 1;
        ulong message_byte_size = compressBound(message_size);
        // printf("msg descomprimida enviada %s\n", message);

        char *message_compress = compress_buffer(message); // chamando função para comprimir/compactar mensagem do buffer
        // printf("msg enviada comprimida %s\n", message_compress);

        send(client, &message_size, sizeof(ulong), 0);
        send(client, &message_byte_size, sizeof(ulong), 0);
        send(client, message_compress, message_byte_size, 0);

        if (strncmp(message, "exit()", 6) == 0)
        {
            printf("exit");
            stop = 0;
            close(client_fd);
        }
        else
        {

            // Uncompress
            ulong buffer_size;
            ulong buffer_byte_size;

            recv(client, &buffer_size, sizeof(ulong), 0);
            recv(client, &buffer_byte_size, sizeof(ulong), 0);

            char *tmp = (char *)malloc(buffer_byte_size * sizeof(char));

            recv(client, tmp, buffer_byte_size, 0);
            // printf("msg recebida comprimida %s\n", tmp);

            char *message_uncompress = uncompress_buffer(tmp, buffer_size, buffer_byte_size);
            printf("%s\n", message_uncompress);
        }
    }
    return 0;
    /*}else{

        send(client, message, sizeof(message), 0);
        recv(client, buffer, sizeof(1024), 0);
        printf("%s \n", buffer);
    }*/
}