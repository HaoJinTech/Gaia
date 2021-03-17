/**
  ******************************************************************************
  * @file    spi_bus.c
  * @author  JACK.Chen
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus_prototype.h"
#include "app_debug.h"
#include "CRC.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
/* Private typedef -----------------------------------------------------------*/
#define SPI_DEBUG  APP_DBG_OFF
/* Private define ------------------------------------------------------------*/
#define ARRAY_SIZE(a) 				(sizeof(a) / sizeof((a)[0]))

#define Empty_Msg_BufferLength 		9
#define MSG_HEAD 					0x02
#define MSG_END 					0x04
#define MSG_NOCMD					0x00
#define MSG_SENDLENGTH				0x01
#define MSG_QUERYPACK				0x02
#define RECV_ACK					0x06
#define RECV_NCK					0x15
#define MAX_PACKLENGTH 				253
#define MAX_PACK					20
#define PACK_GATE					10
#define MaxRetry					20
#define DEVICE_NAME_LENTH  64
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char device[DEVICE_NAME_LENTH] = "/dev/spidev0.0";
static uint32_t last_sendpack_length;
static uint32_t last_last_sendpack_length;
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 21000000;
static uint16_t delay;
static uint8_t MsgId = 0;
static uint8_t empty[Empty_Msg_BufferLength];
//static time_t time_op, time_ed;
//test
// static int32_t Sent_packs = 0;
// static int32_t error_packs = 0;
static int32_t tryed_resend = 0;
static int RemainPack = 0;
static uint8_t FullsizePack[MAX_PACKLENGTH + 6];
//ACK包格式 0X02开头 0x04结尾
static uint8_t ResevedMsg[MAX_PACKLENGTH + 6];
static int ret = 0;
static uint8_t bus_spi_RecvCRC = 0;
static int spi_fd;
static uint8_t re_readcount = 0;
/* Private function prototypes -----------------------------------------------*/
static uint8_t GetMsgID()
{
	MsgId++;
	return MsgId;
}
static uint8_t MsgIDCountDown()
{
	MsgId--;
	return MsgId;
}
//打印收发数据包的hex值和ascii值
static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	unsigned char c;
 
	printf("%s\t | ", prefix);
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

static void pabort(const char *s)
{
	perror(s);
	abort();
}
//单包收发处理
static void transfer(int fd, const uint8_t *tx, uint8_t *rx, size_t len)
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
		pabort("can't send spi message");
}

static int32_t io_spi_write(uint8_t *data, uint32_t len)
{
	uint8_t crccode;
	uint8_t packmsg[len + 4];
	uint8_t readmsg[len + 4];
	memset(packmsg, 0, len + 4);
	memset(readmsg, 0, len + 4);
	packmsg[0] = MSG_HEAD;
	packmsg[1]= GetMsgID();
	memcpy(packmsg + 2, data, len);
	packmsg[len + 2] = MSG_END;
	crccode = crc_high_first(packmsg, len + 3);
	packmsg[len + 3] = crccode;
	memset(ResevedMsg, 0, len + 4);
	memcpy(ResevedMsg, packmsg, len + 4);
	transfer(spi_fd, packmsg, readmsg, len + 4);
	last_last_sendpack_length = last_sendpack_length;
	last_sendpack_length = len + 4;
	return 0;
}
static int32_t io_spi_read(uint8_t *buff, int len)
{
	re_readcount =0;
Reget:
	transfer(spi_fd, empty, buff, len);

	//是否有全一样的包，有就说明副板没准备好，再收一次
	char temp = buff[0];
	int resend = 1;
	int i = 0;
	while(i < len - 1)
	{
		i++;
		if(temp != buff[i])
		{
			resend = 0;
			break;
		}
	}
	if(resend == 1)
	{
		usleep(100000);
		re_readcount++;
		if(re_readcount >= 5)
		{
			//printf("Re-receive pack error, 20 times trying.\n");
			//pabort("Re-receive pack error, 100 times trying.\n")
			printf("Last Last:%d,Last:%d.\n", last_last_sendpack_length, last_sendpack_length);
			return RET_ERROR;
		}
		else
			goto Reget;
	}	
	//再收一次逻辑结束

	if(crc_high_first(buff, len - 1) == buff[len - 1])
	{
		bus_spi_RecvCRC = 1;
	}
	else
	{
		bus_spi_RecvCRC = 0;
	}
	return RET_OK;
}

static int32_t MSG_DealWith(uint8_t *sendmsg, uint32_t len, uint8_t *readmsg, const uint8_t MSGRETURN)
{
	tryed_resend = 0;
	//发送指令信息	
Resend:
	if(tryed_resend >= MaxRetry)
	{	
		tryed_resend = 0;
		return RET_ERROR;
	}
	io_spi_write(sendmsg, len);
	//接收指令的回复
	//usleep(20);
	ret = io_spi_read(readmsg, Empty_Msg_BufferLength);
	//Sent_packs++;
	if(ret < 0)
	{
		//printf("Read ACK Error, resend data.\n");
		usleep(1000);
		MsgIDCountDown();
		tryed_resend++;
		goto Resend;
	}
	if(bus_spi_RecvCRC == 0)
	{
		//hex_dump(ResevedMsg, len + 4, 32,"CRCSEND");
		//hex_dump(readmsg, Empty_Msg_BufferLength, 32, "CRCERR");
	}
	if(readmsg[0] == MSG_HEAD && readmsg[Empty_Msg_BufferLength -2] == MSG_END)
	{
		if(readmsg[2] == MSGRETURN)
		{
			if(bus_spi_RecvCRC == 0)
			{
				//printf("CRC Error, but get correct return\n");
			}
			if(MSG_QUERYPACK == MSGRETURN)
			{
				return readmsg[3];
			}
			else
			{
				return RET_OK;
			}
		}
		else if(readmsg[2] == RECV_NCK)
		{
			//hex_dump(ResevedMsg, len + 4, 32,"NCKSEND");
			//hex_dump(readmsg, Empty_Msg_BufferLength,Empty_Msg_BufferLength, "NCKRECV");
			//error_packs++;
			if(readmsg[1] == MsgId)
			{				
				//printf("Subboard cannot deal with the pack, resend a new pack.\n");
				tryed_resend++;
				usleep(1000);
				goto Resend;
			}
			else if(readmsg[1] == MsgId - 1)
			{
				//printf("Can Resend the last pack.\n");
				usleep(1000);
				MsgIDCountDown();
				goto Resend;
			}
			else
			{
				//printf("MsgId is irelevant, but still resend the last pack.\n");
				usleep(1000);
				MsgIDCountDown();
				tryed_resend++;
				goto Resend;
			}
		}
		else
		{
			if(readmsg[1] == MsgId)
			{
				//printf("The pack is wrong, send a new pack.\n");
				tryed_resend++;
				usleep(1000);
				goto Resend;
			}
		}
	}
	//error_packs++;
	//hex_dump(readmsg, Empty_Msg_BufferLength,Empty_Msg_BufferLength, "ERR");
	//printf("Can not receive a correct pack, resend the last message.\n");
	tryed_resend++;
	usleep(1000);
	MsgIDCountDown();
	goto Resend;
}

//应该交由上层管理控制
static int32_t MSG_SendData(uint8_t* packdata,uint32_t len)
{
	uint8_t readbuff[Empty_Msg_BufferLength];
	memset(readbuff, 0, Empty_Msg_BufferLength);
	return MSG_DealWith(packdata, len, readbuff, RECV_ACK);
}

static int32_t MSG_SendLength(uint32_t len, uint32_t pack)
{
	//制作长度包下发
	uint8_t data[Empty_Msg_BufferLength - 4];
	memset(data, 0, Empty_Msg_BufferLength - 4);
	data[0] = MSG_SENDLENGTH;
	data[1] = len >> 8;
	data[2] = len & 0xFF;
	if(pack > 0)
	{
		data[3] = pack >> 8;
		data[4] = pack & 0xFF;
	}
	uint8_t readbuff[Empty_Msg_BufferLength];
	memset(readbuff, 0, Empty_Msg_BufferLength);
	return MSG_DealWith(data, Empty_Msg_BufferLength - 4, readbuff, RECV_ACK);
}

//应该交由上层管理控制
static int32_t MSG_SendQUERYPACK(void)
{
	//制作长度包下发
	uint8_t data[Empty_Msg_BufferLength - 4];
	memset(data, 0, Empty_Msg_BufferLength - 4);
	data[0] = MSG_QUERYPACK;
	uint8_t readbuff[Empty_Msg_BufferLength];
	memset(readbuff, 0, Empty_Msg_BufferLength);
	return MSG_DealWith(data, Empty_Msg_BufferLength - 4, readbuff, MSG_QUERYPACK);
}

/* Public functions ----------------------------------------------------------*/
int32_t bus_spi_init(uint32_t port, uint32_t freq, void *other)
{
	snprintf(device, DEVICE_NAME_LENTH, "/dev/spidev0.%d", port);
	speed = freq;
	memset(empty, 0, Empty_Msg_BufferLength);
  	return RET_OK;
}

int32_t bus_spi_open(void)
{
	spi_fd = open(device, O_RDWR);
	if (spi_fd < 0)
	{
		pabort("can't open device");
		return RET_ERROR;
	}

	/*
	 * spi mode
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");
 
	ret = ioctl(spi_fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");
	
	/*
	 * bits per word
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");
 
	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");
 
	/*
	 * max speed hz
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");
 
	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");
	
	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
  	return RET_OK;
}

//应该重写，仅进行收发操作
int32_t bus_spi_write(char *data, uint32_t len)
{
	uint8_t split_pack = 0;
	//usleep(100);
	//Sent_packs = 0;
	//判定包长
 	uint16_t sendpacklength = MAX_PACKLENGTH;
	uint16_t packages = (len - 1) / MAX_PACKLENGTH + 1;
	if(packages > 1)
	split_pack = 1;
	uint16_t shortpacks = 0;
	uint32_t sendoffset = 0;
	int32_t RET;

	APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, data, len);

	while(packages > 0){
		//确认收发包有大于门限个可发送或者大于剩余可发送
		RemainPack = MSG_SendQUERYPACK();
		if(RemainPack < 0)
		{
			printf("Subboard cannot return querylength.\n");
			return RET_ERROR;
		}
		while(RemainPack < packages && RemainPack < PACK_GATE)
		{
			usleep(10);
			RemainPack = MSG_SendQUERYPACK();
			if(RemainPack < 0)
			{
				printf("Subboard cannot return querylength.\n");
				return RET_ERROR;
			}
		}
		//确认收发包有大于门限个可发送或者大于剩余可发送

		if(packages <= 1)
		{
			sendpacklength = len - sendoffset;
			shortpacks = packages;
		}
		else
		{
			//如果包大于最大收发包存量，暂时先只发最大存量的包数量
			if(packages > RemainPack)
			{
				shortpacks = RemainPack;
			}
			else
			{
				shortpacks = packages - 1;
			}
		}

		//告诉副板我要发的数据包长度与包数量，长度不大于MAX_PACKLENGTH，包数量不大于MAX_PACK, 注意》》是否包长要算上包头，MSGID，包尾，算上就要 + 3
		RET = MSG_SendLength(sendpacklength + 3, shortpacks);
		if(RET < 0)
		{
			printf("Subboard cannot set data pack length\n");
			return RET_ERROR;
		}

		//将数据分包进行发送
		for(int i = 0; i < shortpacks;i++)
		{
			memset(FullsizePack, 0, sendpacklength);
			//判断余下的数据是否够组成一包，够就完整包大小拷贝，不够就仅拷贝有数据的部分
			if(len - sendoffset > sendpacklength)
			{
				memcpy(FullsizePack, (data + sendoffset), sendpacklength);
				sendoffset += sendpacklength;
			}	
			else
			{
				memcpy(FullsizePack, (data + sendoffset), len - sendoffset);
				sendoffset += len - sendoffset;
			}
			RET = MSG_SendData(FullsizePack, sendpacklength);
			// if(split_pack)
			// 	usleep(10);
			if(RET < 0)
			{
				printf("Send data with unknown error, MsgId:%d\n", MsgId);
				return RET_ERROR;
			}
		}

		//判断是否还有需要发送的包
		packages = packages - shortpacks;
	}
	//轮询到缓存空掉
	// RemainPack = MSG_SendQUERYPACK();
	// while(RemainPack < MAX_PACK)
	// {
	// 	usleep(1000);
	// 	RemainPack = MSG_SendQUERYPACK();
	// }
	return 0;
}

void *bus_spi_read(char *buff, int len)
{
	re_readcount = 0;
Reget:
	transfer(spi_fd, empty, buff, len);
	//是否有全一样的包，有就说明副板没准备好，再收一次
	char temp = buff[0];
	int resend = 1;
	int i = 0;
	while(i < len - 1)
	{
		i++;
		if(temp != buff[i])
		{
			resend = 0;
			break;
		}
	}
	if(resend == 1)
	{
		re_readcount++;
		if(re_readcount >= 20)
		{
			printf("Re-receive pack error, 20 times trying.\n");
		}
		else
			goto Reget;
	}	
	//再收一次逻辑结束
	if(crc_high_first(buff, len - 1) == buff[len - 1])
	{
		bus_spi_RecvCRC = 1;
	}
	else
	{
		bus_spi_RecvCRC = 0;
	}
	return buff;
}

int32_t bus_spi_ioctrl(BUS_CTRL_MSG *msg)
{
	return RET_OK;
}

int32_t bus_spi_close(void *param)
{
	close(spi_fd);
	return RET_OK;
}

#define BUS_SPI   {BUS_ID_SPI,   \
				  NULL,\
                  bus_spi_init,  \
                  bus_spi_open,  \
                  bus_spi_write, \
                  bus_spi_read,  \
                  bus_spi_ioctrl,\
                  bus_spi_close}

#if 1

// static int SendPackCount = 0;

// double writedata_time()
// {
// 	return difftime(time_ed, time_op);
// }


//建立消息List，发送一个消息以后给这个消息列表加一个msgId

//发送消息与收包消息同时进行，收到的消息包不是以255 255 0结尾就再发一个指定长度(EMTPY_MSG_LENGTH)的空包，直到收到包含255 255 0结尾的包作为接收结束
//对于接收的消息进行

int MsgIDCheck(void)
{
	uint8_t read_forthis[Empty_Msg_BufferLength];
	memset(read_forthis, 0, Empty_Msg_BufferLength);
	io_spi_read(read_forthis, Empty_Msg_BufferLength);
	char temp = read_forthis[0];
	int i = 0;
	while(i < Empty_Msg_BufferLength - 1)
	{
		i++;
		if(temp != read_forthis[i])
		{
			if(read_forthis[0] == MSG_HEAD && read_forthis[Empty_Msg_BufferLength - 2] == MSG_END)
			{
				MsgId = read_forthis[1];
				printf("Get MsgID:%d\n",MsgId);
				break;
			}
		}
	}
	uint8_t data[MAX_PACKLENGTH];
	memset(data, 0, MAX_PACKLENGTH);
	uint8_t readbuff[Empty_Msg_BufferLength];
	memset(readbuff, 0, Empty_Msg_BufferLength);
	io_spi_write(data, MAX_PACKLENGTH);
	//接收指令的回复
	//usleep(20);
	transfer(spi_fd, empty, readbuff, Empty_Msg_BufferLength);
	//是否有全一样的包，有就说明副板没准备好，再收一次
	while(i < Empty_Msg_BufferLength - 1)
	{
		i++;
		if(temp != read_forthis[i])
		{
			printf("success break out.\n");
			return 0;
		}
	}	
	printf("Failed to break out.\n");
	return -1;
}

int SendCheckData(void)
{
	uint8_t testindex = 0;
	uint8_t testbuf[MAX_PACKLENGTH];
	
	uint8_t read_test_buff[Empty_Msg_BufferLength];
	memset(read_test_buff, 0 , Empty_Msg_BufferLength);

again:
	if(testindex == 0xFF)
	{
		printf("Failed to break out.\n");
		return -1;
	}
	memset(testbuf, testindex, MAX_PACKLENGTH);
	testindex++;

	io_spi_write(testbuf, MAX_PACKLENGTH);
	transfer(spi_fd, empty, read_test_buff, Empty_Msg_BufferLength);
	//是否有全一样的包，有就说明副板没准备好，再收一次
	char tempchar = read_test_buff[0];
	int soi = 0;
	while(soi < Empty_Msg_BufferLength - 1)
	{
		soi++;
		if(tempchar != read_test_buff[soi])
		{
			printf("Success break out, MsgId:%d", MsgId);
			return 0;
		}
	}
	goto again;
}
int anotherCheckData(void)
{
	uint8_t checklength = 1;
	uint8_t testbuf[MAX_PACKLENGTH];
	uint8_t read_test_buff[Empty_Msg_BufferLength];
	memset(read_test_buff, 0 , Empty_Msg_BufferLength);
	while(checklength <= MAX_PACKLENGTH)
	{
		memset(testbuf, checklength, checklength);
		io_spi_write(testbuf, checklength);
		transfer(spi_fd, empty, read_test_buff, Empty_Msg_BufferLength);
		//是否有全一样的包，有就说明副板没准备好，再收一次
		char tempchar = read_test_buff[0];
		int soi = 0;
		while(soi < Empty_Msg_BufferLength - 1)
		{
			soi++;
			if(tempchar != read_test_buff[soi])
			{
				printf("Success break out, MsgId:%d", MsgId);
				return 0;
			}
		}
		checklength++;
	}
	printf("Failed to break out.\n");
	return -1;
}
int main(void)
{
	uint32_t MessageCount = 1027;
	int sendpackCount = 5000000;
	char BufMsg[MessageCount];
	memset(BufMsg, 0, MessageCount);
	BufMsg[0] = 250;
	BufMsg[MessageCount - 2] = 0xFF;
	BufMsg[MessageCount - 1] = 0xFF;
	for(int i = 0;i < (MessageCount - 3) / 4;i++)
	{
		BufMsg[i * 4 + 1] = ((i + 1) >> 8) & 0xFF;
		BufMsg[i * 4 + 2] = ((i + 1) & 0xFF);
		BufMsg[i * 4 + 3] = (((15 * i) % 360) >> 8) & 0xFF;
		BufMsg[i * 4 + 4] = ((15 * i)% 360) & 0xFF;
	}
	ret = bus_spi_init(0, 21000000, NULL);
	ret = bus_spi_open();
	while(sendpackCount--)
	{
	 	ret = bus_spi_write(BufMsg, MessageCount);
	}

	//SendCheckData();
	// uint8_t read_forthis[Empty_Msg_BufferLength];
	// memset(read_forthis, 0, Empty_Msg_BufferLength);
	// io_spi_read(read_forthis, Empty_Msg_BufferLength);

	//anotherCheckData();
	
	MsgIDCheck();

	//printf("Sent over.\n");
	//printf("Send Data use time:%fs\n", writedata_time());
	return 0;
}
#endif


//建立消息List，发送一个消息以后给这个消息列表加一个msgId

//发送消息与收包消息同时进行，收到的消息包不是以255 255 0结尾就再发一个指定长度(EMTPY_MSG_LENGTH)的空包，直到收到包含255 255 0结尾的包作为接收结束
//对于接收的消息进行