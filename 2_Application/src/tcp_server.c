/*
 * File      : tcp_server.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-28     YaoWang      first implementation
 */

#include <stdio.h>
#include <netinet/in.h>   //for souockaddr_in
#include <sys/types.h>     
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h> // open function
#include <unistd.h> // close function
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>

#include "list.h"
#include "platform.h"
#include "sys_config.h"
#include "rbuffer.h"

typedef struct tcp_cmd_msg{
    char *recv_buf;
    char *send_buf;
    uint32_t sock_fd;
} TCP_CMD_MSG;

typedef struct tcp_rx_rbuf{
    int32_t sock_fd;
    RBUF     *rxbuf;
    struct list_head list;
} TCP_SOCK_BUFS;

TCP_SOCK_BUFS rx_buf_list_root;

const int PORT = SERVER_PORT_DEF;

LOCAL void listen_loop();
LOCAL void accept_conn(uint32_t sock_fd, uint32_t epollfd);
LOCAL int32_t recv_message(uint32_t sock_fd);
LOCAL uint32_t init_rx_buffer(uint32_t accept_fd);
LOCAL TCP_SOCK_BUFS *fd2rxbuf(uint32_t sock_fd);
LOCAL void close_sock(uint32_t sock_fd);

int  tcp_server_init(void);

int tcp_server_init(void) {
    int sock_fd;
    struct sockaddr_in server_addr;

    rx_buf_list_root.sock_fd = -1;
    rx_buf_list_root.rxbuf = NULL;
    INIT_LIST_HEAD(&rx_buf_list_root.list);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket:");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 0;
    }
    if (listen(sock_fd, 10) < 0) {
        perror("listen");
        return 0;
    }

    listen_loop(sock_fd);

    return 0;
}

LOCAL uint32_t init_rx_buffer(uint32_t accept_fd)
{
#define RB_NAME_SIZE 16
    char rb_name[RB_NAME_SIZE];
    TCP_SOCK_BUFS *tcp_rx_buf = malloc(sizeof(TCP_SOCK_BUFS));

    if(NULL == tcp_rx_buf){
        perror("malloc");
        return RET_ERROR;
    }
    snprintf(rb_name, RB_NAME_SIZE, "tcprx%d", accept_fd);
    tcp_rx_buf->sock_fd = accept_fd;
    tcp_rx_buf->rxbuf = rb_malloc(rb_name, TCP_RX_BUFFER_SIZE_DEF, TCP_RX_BUFFER_SIZE_MAX_DEF);
    INIT_LIST_HEAD(&tcp_rx_buf->list);

    list_add(&tcp_rx_buf->list, &rx_buf_list_root.list);

    return RET_OK;
}

LOCAL void accept_conn(uint32_t sock_fd, uint32_t epollfd) {
    struct sockaddr_in clientaddr;
    struct epoll_event event;
    socklen_t len = sizeof(struct sockaddr);
    uint32_t accept_fd = 0;

    accept_fd = accept(sock_fd, (struct sockaddr*)&clientaddr, &len);

    if (accept_fd <= 0) {
        perror("accept error");
        return;
    }

    event.data.fd = accept_fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, accept_fd, &event);

    init_rx_buffer(accept_fd);
    return;
}

LOCAL TCP_SOCK_BUFS *fd2rxbuf(uint32_t sock_fd)
{
    TCP_SOCK_BUFS *iter;
    list_for_each_entry(iter, &rx_buf_list_root.list, list){
        if(iter->sock_fd == sock_fd)
            return iter;
    }
    return NULL;
}

LOCAL int32_t recv_message(uint32_t sock_fd) {
#define RX_TEMP_LEN  2048
    int32_t read_len = 0;
    TCP_SOCK_BUFS *tcp_rx_buf;
    uint32_t put_size = 0;
    uint8_t recv_buf[RX_TEMP_LEN];
    printf("[recv_message] sock_fd = %d\n",sock_fd);

    tcp_rx_buf = fd2rxbuf(sock_fd);
    if(tcp_rx_buf == NULL)
        return RET_ERROR;

    do {

        read_len = recv(sock_fd, recv_buf, RX_TEMP_LEN, 0);
        if(read_len == 0)
            return RET_ERROR;

        put_size = rb_put(tcp_rx_buf->rxbuf, recv_buf, read_len);
        if(put_size != read_len)
            return RET_ERROR;
    }while(read_len == RX_TEMP_LEN);
/*
    strcpy(send_buf, recv_buf);
    send(sock_fd, send_buf, sizeof(send_buf), 0);
*/
    return RET_OK;
}

LOCAL void close_sock(uint32_t sock_fd)
{
    TCP_SOCK_BUFS *tcp_rx_buf;
    tcp_rx_buf = fd2rxbuf(sock_fd);
    if(tcp_rx_buf != NULL){
        rb_free(tcp_rx_buf->rxbuf);
        list_del(&tcp_rx_buf->list);
        free(tcp_rx_buf);
    }

    close(sock_fd);
}

LOCAL void listen_loop(uint32_t sock_fd)
{
    int32_t epollfd, i, ret;
    int32_t timeout = 300;
    struct epoll_event event;
    struct epoll_event eventList[CLI_NUM_MAX];

    epollfd = epoll_create(CLI_NUM_MAX);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sock_fd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_fd, &event) < 0) {
        printf("register epoll event err !");
        return;
    }

    while (1) {
        ret = epoll_wait(epollfd, eventList, CLI_NUM_MAX, timeout);

        if (ret < 0) {
            printf("epoll event err!");
            break;
        } else if (ret == 0) {
            continue;
        }

        for (i = 0; i < ret; i++) {
            if ((eventList[i].events & EPOLLERR) || (eventList[i].events & EPOLLHUP) || !(eventList[i].events & EPOLLIN)) {
                printf("epoll error\n");
                close_sock(eventList[i].data.fd);
                exit(-1);
            }

            if (eventList[i].events & EPOLLRDHUP) {
                printf("//one client close the conne.//\n");
                close_sock(eventList[i].data.fd);
            }

            if (eventList[i].data.fd == sock_fd) {
                accept_conn(sock_fd, epollfd);
            } else {
                recv_message(eventList[i].data.fd);
            }
        }
    }

    close(epollfd);
    close(sock_fd);
    return;
}
