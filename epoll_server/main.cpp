/* 
 * File:   main.cpp
 * Author: zhe
 *
 * Created on 2014年3月31日, 下午2:54
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <map>

#include "sys/fcntl.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "sys/signal.h"
#include "sys/epoll.h"

using namespace std;

#define MESSAGE_LENGTH 1024

volatile bool is_running;

void Usage(char **);

void setnonblock(int fd);

int main(int argc, char** argv) {
    is_running = true;
    int port;
    if (argc < 2) {
        Usage(argv);
        exit(1);
    }
    port = atoi(argv[1]);
    struct sockaddr_in server_addr;
    int server_sock;
    map<int, int> context;
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Create socket failed.");
        exit(1);
    }
    setnonblock(server_sock);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Bind socket failed.");
        exit(1);
    }
    
    if (listen(server_sock, 10) == -1) {
        perror("Listen socket failed.");
        exit(1);
    }
    
    struct epoll_event ev, events[1024];
    ev.events = EPOLLIN | EPOLLET;

    int epoll_fd = epoll_create(10240);
    ev.data.fd = server_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &ev) == -1) {
        exit(1);
    }
    int loop = 0;
    while (is_running) {
        fprintf(stderr, "Wait for event %d\n", loop ++);
        int epoll_event_num = epoll_wait(epoll_fd, events, 1024, -1);
        fprintf(stderr, "%d events\n", epoll_event_num);
        for (int i = 0;i < epoll_event_num; ++i) {
            if (events[i].data.fd == server_sock) {
                struct sockaddr_in client_addr;
                uint client_length = sizeof(client_addr);
                int client_fd = accept(server_sock, (sockaddr *)&client_addr, &client_length);
                if (client_fd == -1) {
                    perror("Cant establish connect");
                    continue;
                }
                setnonblock(client_fd);
                fprintf(stderr, "Accept socket with fd : %d\n", client_fd);
                context[client_fd] = 0;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            } else if (events[i].events & EPOLLIN) {
                char buffer[MESSAGE_LENGTH];
                int msg_total = context[events[i].data.fd];
                int msg_len = 0;
                if ((msg_len = read(events[i].data.fd, buffer, sizeof(buffer))) == -1) {
                    perror("Read data failed.");
                    continue;
                }
                if (msg_len == 0) {
                    close(events[i].data.fd);
                    context.erase(events[i].data.fd);
                    fprintf(stderr, "Close client fd : %d\n", events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                    continue;
                }
                msg_total += msg_len;
                if (write(events[i].data.fd, buffer, msg_len) == -1) {
                    perror("Send data failed");
                }    

                context[events[i].data.fd] = msg_total;
            }
            
        }
        
    }
    
    
    close(epoll_fd);
    close(server_sock);   
    return 0;
}

void System_gc(int signal) {
}

void Usage(char **argv) { 
    fprintf(stderr, "Usage : %s port\n", argv[0]);
}

void setnonblock(int fd) {
    int flag;
    if ((flag = fcntl(fd, F_GETFL)) == -1) {
        perror("Get flag failed.");
        return;
    }
    
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) {
        perror("Set flag failed.");
        return;
    }
}