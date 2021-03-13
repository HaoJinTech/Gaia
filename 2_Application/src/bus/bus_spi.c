/**
  ******************************************************************************
  * @file    spi_bus.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus_prototype.h"
#include <stdint.h>
#include <unistd.h>
#include "app_debug.h"
#include <stdio.h>
#include <list.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "CRC.h"
/* Private typedef -----------------------------------------------------------*/
//需要处理的消息队列
typedef struct Message_Type{
	uint16_t MsgId;
	char* MessageInfo;
	struct list_head Node;
};
#define SPI_DEBUG  APP_DBG_ON
/* Private define ------------------------------------------------------------*/
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define Empty_Msg_BufferLength 8
#define ENDLINE {255, 255, 0}
#define MAXPACKLENGTH 1024
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define DEVICE_NAME_LENTH  64
static const char device[DEVICE_NAME_LENTH] = "/dev/spidev0.0";
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 10500000;
static uint16_t delay;
static int verbose;
static char empty[Empty_Msg_BufferLength];

struct list_head MessageQueueList;

char *input_tx;
int ret = 0;
int fd;
uint8_t *tx;
uint8_t *rx;
int size;
/* Private function prototypes -----------------------------------------------*/


//打印收发数据包的hex值和ascii值
static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	unsigned char c;
 
	printf("%s | ", prefix);
	while (length-- > 0) {
		printf("%02X ", *address++);
		if (!(++i % line_size) || (length == 0 && i % line_size)) {
			if (length == 0) {
				while (i++ % line_size)
					printf("__ ");
			}
			printf(" | ");  /* right close */
			while (line < address) {
				c = *line++;
				printf("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			printf("\n");
			if (length > 0)
				printf("%s | ", prefix);
		}
	}
}
/*
 *  Unescape - process hexadecimal escape character
 *      converts shell input "x23" -> 0x23
 */
static int unescape(char *_dst, char *_src, size_t len)
{
	int ret = 0;
	char *src = _src;
	char *dst = _dst;
	unsigned int ch;
 
	while (*src) {
		if (*src == NULL && *(src+1) == 'x') {
			sscanf(src + 2, "%2x", &ch);
			src += 4;
			*dst++ = (unsigned char)ch;
		} else {
			*dst++ = *src++;
		}
		ret++;
	}
	return ret;
}
//单包收发处理
static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
 
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
 
	if (mode & SPI_TX_QUAD)
		tr.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr.tx_buf = 0;
	}
 
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		;//pabort("can't send spi message");
 
	if (verbose)
		hex_dump(tx, len, 32, "TX");
	hex_dump(rx, len, 32, "RX");
}
//打印帮助信息
static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev0.0)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
	     "  -v --verbose  Verbose (show tx buffer)\n"
	     "  -p            Send data (e.g. \"1234xdexad\")\n"
	     "  -N --no-cs    no chip select\n"
	     "  -R --ready    slave pulls low to pause\n"
	     "  -2 --dual     dual transfer\n"
	     "  -4 --quad     quad transfer\n");
	exit(1);
}
//根据参数配置
#if 0
static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "dual",    0, 0, '2' },
			{ "verbose", 0, 0, 'v' },
			{ "quad",    0, 0, '4' },
			{ NULL, 0, 0, 0 },
		};
		int c;
 
		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR24p:v", lopts, NULL);
 
		if (c == -1)
			break;
 
		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case 'p':
			input_tx = optarg;
			break;
		case '2':
			mode |= SPI_TX_DUAL;
			break;
		case '4':
			mode |= SPI_TX_QUAD;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
	if (mode & SPI_LOOP) {
		if (mode & SPI_TX_DUAL)
			mode |= SPI_RX_DUAL;
		if (mode & SPI_TX_QUAD)
			mode |= SPI_RX_QUAD;
	}
}
#endif
/* Public functions ----------------------------------------------------------*/
int32_t bus_spi_init(uint32_t port, uint32_t freq, void *other)
{
	snprintf( device, DEVICE_NAME_LENTH, "/dev/spidev0.%d", port);
	speed = freq;
	memset(empty, 0, Empty_Msg_BufferLength);
  	return RET_OK;
}
/* int32_t bus_spi_init(int argc, char *argv[])
{
  parse_opts(argc, argv);
  INIT_LIST_HEAD(&MessageQueueList);
  return RET_OK;
} */

int32_t bus_spi_open(void)
{
	fd = open(device, O_RDWR);
		if (fd < 0)
			;//pabort("can't open device");
  	return RET_OK;
}

int32_t bus_spi_write(char *data, uint32_t len)
{
//<<<<<<< HEAD
	//判定包长
	uint32_t sendpacklength = MAXPACKLENGTH;
	uint16_t packages = (sizeof(&data) - 1) / MAXPACKLENGTH + 1;
	if(packages <= 1)
	{
		sendpacklength = sizeof(&data);
	}
	int32_t ret = bus_spi_sendLength(sendpacklength);
	if(ret < 0)
	{
		//报错信息
	}
	char packmsg[sendpacklength + 2];
	memset(packmsg, 0, sendpacklength);
	uint32_t index = 0;
	char *readbuff;
	//从第一个包开始直到最后第二个包结束，进行发包
	for(int i = 0;i < packages - 1;i++)
	{
		strncpy(packmsg, data + index, sendpacklength);
		index += sendpacklength;
		//加入CRC
		packmsg[sendpacklength] = 0x0d;
		packmsg[sendpacklength + 1] = 0x0a;
		transfer(fd, packmsg, readbuff, sendpacklength);
		//每发送指定数量的包以后进行询问，看还积了多少包没有发，再往下发指定数量-还剩的值，这个差值可以做一个门限，不需要一个个发
	}
	//处理发完剩下的一包，剩下一包理论上是小于等于指定包长度的
	if(sizeof(&data) - 1 > index)
	{
		memset(packmsg, 0, sendpacklength + 2);
		strncpy(packmsg, data + index, sizeof(&data) - 1 - index);
		index = 0;
		//加入CRC
		packmsg[sendpacklength] = 0x0d;
		packmsg[sendpacklength + 1] = 0x0a;
		transfer(fd, packmsg, readbuff, sizeof(packmsg));
	}
	return 0;
}

int32_t bus_spi_sendLength(uint32_t len)
{
	//制作长度包下发
	char data[8];


	char *readbuff;
	transfer(fd, data, readbuff, sizeof(data));
	return 0;
}

/* int32_t bus_spi_write(struct Message_Type *msg, uint32_t len)
{
	list_add_tail(&msg->Node, &MessageQueueList);
	char *readbuff;
	transfer(fd, &msg->MessageInfo, readbuff, sizeof(&msg->MessageInfo));
	//处理readbuff

  	return 0;
} */

/* char* bus_spi_read(char * msg)
{
	char *readbf = msg;
	char *longread = "";
	char Emptystr[Empty_Msg_BufferLength];
	memset(Emptystr,0,Empty_Msg_BufferLength);
	//判断是否包含结束符
	char *ret = strpbrk(readbf,(char *)ENDLINE);
	strcat(longread, readbf);
	if(strlen(ret)<3)//不包含完整结束符的话再读一次，并且把读到的消息放入longread队尾
	{
		transfer(fd, Emptystr, readbf, Empty_Msg_BufferLength);
		ret = strpbrk(readbf,(char *)ENDLINE);
		strcat(longread, readbf);
	}
	return longread;
//=======
 	APP_DEBUGF(SPI_DEBUG | APP_DBG_TRACE, ("write data lenth: %d\r\n", len));
  APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, data, len);
  return 0;
//>>>>>>> a797dff93ddb1dedd994bd30ca253e4fbbf76f80
} */

int32_t bus_spi_read(char *buff, int len)
{
	transfer(fd, empty, buff, len);
	return buff;
}

int32_t bus_spi_ioctrl(BUS_CTRL_MSG *msg)
{
	
	return RET_OK;
}

int32_t bus_spi_close(void *param)
{
	close(fd);
	return RET_OK;
}

#define BUS_SPI   {BUS_ID_SPI,   \
                  bus_spi_init,  \
                  bus_spi_open,  \
                  bus_spi_write, \
                  bus_spi_read,  \
                  bus_spi_ioctrl,\
                  bus_spi_close}


//建立消息List，发送一个消息以后给这个消息列表加一个msgId

//发送消息与收包消息同时进行，收到的消息包不是以255 255 0结尾就再发一个指定长度(EMTPY_MSG_LENGTH)的空包，直到收到包含255 255 0结尾的包作为接收结束
//对于接收的消息进行