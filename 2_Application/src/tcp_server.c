/**
  ******************************************************************************
  * @file    rbuffer.c
  * @author  YORK
  * @version V0.1.0
  * @date    01-03-2021
  * @brief   Round buffer stuffs, used for telnet.
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

/* Includes ------------------------------------------------------------------*/
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
#include <stdarg.h>

#include "app_debug.h"
#include "cmd_msg.h"
#include "list.h"
#include "platform.h"
#include "sys_config.h"
#include "rbuffer.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct tcp_rx_rbuf{
    int32_t sock_fd;
    RBUF     *rxbuf;
    struct list_head list;
} TCP_SOCK_BUFS;

/* Private define ------------------------------------------------------------*/
#define TCP_DEBUG                       APP_DBG_ON

#define TCP_RX_BUFFER_SIZE_DEF      4096
#define TCP_RX_BUFFER_SIZE_MAX_DEF  65536

#define SYMBOL_TAILED     "\r\n"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const int PORT = SERVER_PORT_DEF;
TCP_SOCK_BUFS rx_buf_list_root;

/* Private function prototypes -----------------------------------------------*/
LOCAL void listen_loop(uint32_t sock_fd);
LOCAL void accept_conn(uint32_t sock_fd, uint32_t epollfd);
LOCAL int32_t recv_message(uint32_t sock_fd);
LOCAL int32_t send_message(uint32_t dest_fd, const char *fmt, ...);
LOCAL uint32_t init_rx_buffer(uint32_t accept_fd);
LOCAL TCP_SOCK_BUFS *fd2rxbuf(uint32_t sock_fd);
LOCAL void close_sock(uint32_t sock_fd);

/* Public functions ---------------------------------------------------------*/
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


/* Private functions ---------------------------------------------------------*/

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
    char recv_buf[128];

    accept_fd = accept(sock_fd, (struct sockaddr*)&clientaddr, &len);

    if (accept_fd <= 0) {
        perror("accept error");
        return;
    }
    recv(accept_fd, recv_buf, 128, 0);
    APP_DEBUGF(TCP_DEBUG | APP_DBG_TRACE, ("accept recv: %s", recv_buf));

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

#define CMD_PRINT_SIZE   2048
LOCAL int32_t send_message(uint32_t dest_fd, const char *fmt, ...)
{
   	va_list args;
    char cmd_buf[CMD_PRINT_SIZE];
    int len=0;

   	va_start(args, fmt);
	len = vsnprintf(cmd_buf, CMD_PRINT_SIZE, fmt, args);
	va_end(args);

    return send(dest_fd, cmd_buf, len, 0);

}

LOCAL int32_t check_buf_and_send_msg(TCP_SOCK_BUFS *tcp_rx_buf)
{
    uint32_t rx_size = 0;
    char *line = NULL;
    line = rb_getline(tcp_rx_buf->rxbuf, SYMBOL_TAILED, sizeof(SYMBOL_TAILED)-1, &rx_size);

    if(line == NULL)
        return RET_OK;
    APP_DEBUGF(TCP_DEBUG | APP_DBG_TRACE, ("get line: %s", line));
    send_cmd_msg(tcp_rx_buf->sock_fd, line, send_message);
    return RET_OK;
}

LOCAL int32_t recv_message(uint32_t sock_fd) {
#define RX_TEMP_LEN  2048
    int32_t read_len = 0;
    TCP_SOCK_BUFS *tcp_rx_buf;
    uint32_t put_size = 0;
    uint8_t recv_buf[RX_TEMP_LEN];
    APP_DEBUGF(TCP_DEBUG | APP_DBG_TRACE, ("sock_fd = %d\n",sock_fd));

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

    check_buf_and_send_msg(tcp_rx_buf);
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
        APP_DEBUGF(TCP_DEBUG | APP_DBG_LEVEL_SEVERE, ("register epoll event err !\n"));
        return;
    }

    APP_DEBUGF(TCP_DEBUG | APP_DBG_TRACE , ("start epoll ...\r\n"));
    while (1) {
        ret = epoll_wait(epollfd, eventList, CLI_NUM_MAX, timeout);

        if (ret < 0) {
            APP_DEBUGF(TCP_DEBUG | APP_DBG_LEVEL_SEVERE, ("epoll event err!\n"));
            continue;//break;
        } else if (ret == 0) {
            continue;
        }

        for (i = 0; i < ret; i++) {
            if ((eventList[i].events & EPOLLERR) || (eventList[i].events & EPOLLHUP) || !(eventList[i].events & EPOLLIN)) {
                APP_DEBUGF(TCP_DEBUG | APP_DBG_LEVEL_SEVERE, ("epoll error\n"));
                continue;
                //close_sock(eventList[i].data.fd);
                //exit(-1);
            }

            if (eventList[i].events & EPOLLRDHUP) {
                APP_DEBUGF(TCP_DEBUG | APP_DBG_TRACE, ("one client close the conne.\n"));
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
