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

#define MAX_THREAD 1

void* pthread_func(void *);

void Usage(char **);

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

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
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    pthread_t threads[MAX_THREAD];
    for (int i = 0;i < MAX_THREAD; ++i) {
        if (pthread_create(threads + i, NULL, pthread_func, &server_addr) == -1) {
            perror("Can't create thread");
        }
        
    }
    
    
    for (int i = 0;i < MAX_THREAD; ++i) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

void *pthread_func(void *arg) {
    //sleep(rand() % 100);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Create socket failed.");
        pthread_exit(NULL);
    }
    if (connect(client_fd, (struct sockaddr *)arg, sizeof(struct sockaddr)) == -1) {
        perror("Can't connect server.");
        pthread_exit(NULL);
    }
    for (int i = 0;i < 1000; ++i) {
        char buffer[1024];
        const char msg[] = "Thisisatest";
        strcpy(buffer, msg);
        //sleep(rand() % 100);
        pthread_mutex_lock(&global_lock);
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("Write failed.");
        }
        if (read(client_fd, buffer, sizeof(buffer)) == -1) {
            perror("Read failed.");
        }
        pthread_mutex_unlock(&global_lock);
        
    }
    pthread_exit(NULL);
}

void Usage(char **argv) {
    printf("Usage : %s ip port\n", argv[0]);
}

