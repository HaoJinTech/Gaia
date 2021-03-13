#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "bus_prototype.h"
#include "app_debug.h"

#define BUS_UART_DEBUG        APP_DBG_ON

#define MAXNAMELENGHT           1024
#define DEFAULTBAUDRATE         115200
#define UART                    "dev/ttyeAMA1"
LOCAL char *uart_Name;
LOCAL uint32_t uart_baudrate;
LOCAL int fd;

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
if (tcgetattr( fd,&oldtio)  !=  0) {
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch( nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch( nSpeed )
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
    case 921600:
		cfsetispeed(&newtio, B921600);
		cfsetospeed(&newtio, B921600);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	return 0;
}

int32_t bus_uart_init(uint32_t port, uint32_t baudrate, void *other)
{
    uart_Name = UART;

    if(baudrate > 0)
        uart_baudrate = baudrate;
    else    
        uart_baudrate = DEFAULTBAUDRATE;
    return RET_OK;
}

int32_t bus_uart_open(void)
{
    int result = 0;
	fd = open(uart_Name, O_RDWR|O_NOCTTY|O_NDELAY, 0777);
	if(fd < 0)
	{
		printf("ret:2,failed to open UART:%s\n", uart_Name);
        return RET_ERROR;
	}
	else
	{
		result = set_opt(fd, uart_baudrate, 8, 'N', 1);
	}
	if(result < 0)
	{
		printf("%s>>com set error", uart_Name);
		return RET_ERROR;
	}
	printf("Success open UART:%s\n", uart_Name);
	return RET_OK;
}

int32_t bus_uart_write(const char *sendmsg, uint32_t length)
{
    ssize_t sendret = 0;
	APP_DEBUGF_HEX(BUS_UART_DEBUG | APP_DBG_TRACE, sendmsg, length);
    sendret = write(fd, sendmsg, length);
    if(sendret == length)
    {
        return sendret;
    }
    else
    {
        printf("Write device error\n");
        tcflush(fd, TCOFLUSH);
        return RET_ERROR;
    }
}

int32_t bus_uart_read(char *buf, uint32_t bufsize)
{
    ssize_t len;
    len = read(fd, buf, bufsize);
    return len;
}

int32_t bus_uart_ioctrl(BUS_CTRL_MSG *msg)
{
	return RET_OK;
}

int32_t bus_uart_close(void *param)
{
	close(fd);
	return RET_OK;
}


#define BUS_UART  {BUS_ID_UART,   \
				  NULL,			\
                  bus_uart_init,  \
                  bus_uart_open,  \
                  bus_uart_write, \
                  bus_uart_read,  \
                  bus_uart_ioctrl,\
                  bus_uart_close}
