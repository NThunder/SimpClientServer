#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PAGESIZE 8196

int server_socket;

volatile int cur_sig = 0;
volatile int break_sig = 1;

enum FDS { FDIN = 0, FDOUT = 1, FDERR = 2 };

void handle(int signum)
{
    if (cur_sig == 1) {
        close(server_socket);
        _exit(0);
    }
    break_sig = 0;
}

int main(int argc, char** argv)
{
    struct sigaction action = {.sa_handler = &handle, .sa_flags = SA_RESTART};
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in sockaddr = {
        AF_INET, htons(atoi(argv[1])), {inet_addr("127.0.0.1")}};
    int succses =
        bind(server_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (succses == -1) {
        perror("bind");
        close(server_socket);
        return -1;
    }
    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("listen");
        close(server_socket);
        return -1;
    }
    while (break_sig) {
        cur_sig = 1;
        int connfd = accept(server_socket, (struct sockaddr*)NULL, NULL);
        cur_sig = 0;
        char query[PAGESIZE] = {};
        if (read(connfd, query, PAGESIZE) <= 0) {
            close(connfd);
            continue;
        }
        char file_name[PAGESIZE] = {};
        sscanf(query, "%s", file_name);

        char full_path[PAGESIZE] = {};
        strcpy(full_path, "~");
        strcat(full_path, "/");
        strcat(full_path, file_name);
        struct stat st = {};
        stat(full_path, &st);
        if (access(full_path, F_OK) != 0) {
            char reply[PAGESIZE] = "HTTP/1.1 404 Not Found\r\n";
            write(connfd, reply, strlen(reply));
            write(connfd, "Content-Length: 0\r\n", 19);
            write(connfd, "\r\n", 2);
        } else if (access(full_path, R_OK) != 0) {
            char reply[PAGESIZE] = "HTTP/1.1 403 Forbidden\r\n";
            write(connfd, reply, strlen(reply));
            dup2(connfd, FDOUT);
            write(connfd, "Content-Length: 0\r\n", 19);
            write(connfd, "\r\n", 2);
        } else {
            ssize_t input = open(full_path, O_RDONLY);
            char reply[PAGESIZE] = "HTTP/1.1 200 OK\r\n";
            write(connfd, reply, strlen(reply));
            char len[PAGESIZE] = "Content-Length: ";
            char rep[PAGESIZE];
            sprintf(rep, "%ld", st.st_size);
            strcat(len, rep);
            write(connfd, len, strlen(len));
            write(connfd, "\r\n\r\n", 4);
            char page[PAGESIZE] = {};
            ssize_t read_size = 1;
            while (read_size > 0) {
                if ((read_size = read(input, page, PAGESIZE)) == -1) {
                    break;
                }
                write(connfd, page, read_size);
            }
            close(input);
        }
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }
    close(server_socket);
    return 0;
}