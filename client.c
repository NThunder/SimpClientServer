#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

enum FDS { FDIN = 0, FDOUT = 1, FDERR = 2 };

int main(int argc, char** argv)
{
    if (argc != 5) {
        printf("%s\n", "Please, inter the request in format: 127.0.0.1 80 GET file");
        exit(1);
    }
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in sockaddr = {
        AF_INET, htons(atoi(argv[2])), {inet_addr(argv[1])}};
    int succses =
        connect(client_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (succses == -1) {
        perror("connect");
        close(client_socket);
        return -1;
    }
    char answer[4096];
    if (!strcmp("GET", argv[3])) {
        write(client_socket, argv[4], sizeof(argv[4]));
        while (read(client_socket, answer, sizeof(answer)) > 0) {
            printf("%s\n", answer);
        }
    }
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    return 0;
}