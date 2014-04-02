/* 
 * File:   main.cpp
 * Author: zhe
 *
 * Created on 2014年3月31日, 下午5:53
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "pthread.h"
#include "sys/socket.h"
#include "sys/fcntl.h"
#include "arpa/inet.h"
using namespace std;

void* pthread_func(void *);

void Usage(char **);

int main(int argc, char** argv) {
    if (argc < 3) {
        Usage(argv);
        exit(1);
    }
    char *ip = argv[1];
    int port = atoi((char *)argv[2]);
    printf("%s %s\n", argv[1], argv[2]);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    pthread_func(&server_addr);
    puts("ALL DONE");
    return 0;
}

void *pthread_func(void *arg) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Create socket failed.");
    }
    if (connect(client_fd, (struct sockaddr *)arg, sizeof(struct sockaddr)) == -1) {
        perror("Can't connect server.");
    }
    for (int i = 0;i < 1000; ++i) {
        char buffer[1024];
        const char msg[] = "Thisisatest";
        strcpy(buffer, msg);
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("Write failed.");
        }
        puts("SEND OK");
        if (read(client_fd, buffer, sizeof(buffer)) == -1) {
            perror("Read failed.");
        }    
        puts("RECV OK");
    }
    close(client_fd);
}

void Usage(char **argv) {
    printf("Usage : %s ip port\n", argv[0]);
}

