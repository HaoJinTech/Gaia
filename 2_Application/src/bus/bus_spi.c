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
//#include <wiringPi.h>
/* Private typedef -----------------------------------------------------------*/
#define SPI_DEBUG  APP_DBG_ON
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
#define RECV_PWRUP					0xFD
#define MAX_PACKLENGTH 				253
#define MAX_PACK					20
#define PACK_GATE					10
#define MaxRetry					5
#define DEVICE_NAME_LENTH  64
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char device[DEVICE_NAME_LENTH] = "/dev/spidev0.0";
static uint32_t last_sendpack_length;
static uint32_t last_sendpack_count;
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 21000000;
static uint16_t delay;
static uint8_t MsgId = 0xFF;
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
static uint8_t ReceivedMsg[512];
static uint8_t ReceviedMsg_offset = 0;
static int ret = 0;
static uint8_t bus_spi_RecvCRC = 0;
static int spi_fd;
static uint8_t re_readcount = 0;
static enum spi_trans_state{
	STAT_CheckForReady, STAT_BeforeSendData, STAT_SendingData, STAT_SendingComplete, STAT_Unkown = 255
} SPI_BUS_STATUS;

/* Private function prototypes -----------------------------------------------*/
//static void RecoverStatus(void);

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
				printf("%s\t | ", prefix);
		}
	}
}
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
	
	//调试
	hex_dump(packmsg, len + 4, 32, "SEND");
	hex_dump(readmsg, len + 4, 32, "SENDREAD");
	//

	if(readmsg[1] == MsgId && readmsg[2] == RECV_ACK)
		return RET_OK;
	else
		return RET_ERROR;
	
	//APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, packmsg, len + 4);
	
	//APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, readmsg, len + 4);
	
	// if(ReceviedMsg_offset >= 240)
	// ReceviedMsg_offset = 0;
	// memcpy(ReceivedMsg + ReceviedMsg_offset, packmsg + 1, 6);
	// ReceviedMsg_offset += 6;
	//return 0;
}
static int32_t io_spi_read(uint8_t *buff, int len)
{
	re_readcount = 0;
	RecreateEmptyPack();
Reget:
	transfer(spi_fd, empty, buff, len);
	hex_dump(empty,Empty_Msg_BufferLength,32,"READSEND");
	hex_dump(buff,Empty_Msg_BufferLength,32,"READ");
	//APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, buff, len);
	//是否有全一样的包，有就说明副板没准备好，再收一次
	char temp = buff[0];
	int resend = 1;
	int i = 0;
	while(i < len - 1)	{
		i++;
		if(temp != buff[i])	{
			resend = 0;
			break;
		}
	}
	if(resend == 1)	{
		usleep(50000);
		re_readcount++;
		if(re_readcount >= 5)	{
			// if(SPI_BUS_STATUS == STAT_SendingData || SPI_BUS_STATUS == STAT_SendingComplete)
			// 	printf("Data transfer err, length:%d, packs:%d.\n", last_sendpack_length, last_sendpack_count);
			return RET_ERROR;
		}
		else
			goto Reget;
	}	
	//再收一次逻辑结束
	
	// if(ReceviedMsg_offset >= 240)
	// 	ReceviedMsg_offset = 0;
	// memcpy(ReceivedMsg + ReceviedMsg_offset, buff + 1, 6);
	// ReceviedMsg_offset += 6;
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
		MsgIDCountDown();
		MsgIDCountDown();
		checklength++;
	}
	printf("Failed to break out.\n");
	return -1;
}
static int32_t MSG_DealWith(uint8_t *sendmsg, uint32_t len, uint8_t *readmsg, const uint8_t MSGRETURN)
{
	tryed_resend = 0;
	//发送指令信息	
Resend:
	if(tryed_resend >= MaxRetry)
	{	
		printf("Offset:%d\n", ReceviedMsg_offset);
		hex_dump(ReceivedMsg, 240, 6, "Pair");
		tryed_resend = 0;
		return RET_ERROR;
	}
	ret = io_spi_write(sendmsg, len);
	if(ret < 0)
	{
		MsgIDCountDown();
		goto Resend;
	}
	//接收指令的回复
	//usleep(20);
	ret = io_spi_read(readmsg, Empty_Msg_BufferLength);
	//Sent_packs++;
	if(ret < 0)
	{
		MsgIDCountDown();
		MsgIDCountDown();
		tryed_resend++;
		usleep(1000);
		goto Resend;
		//return RET_ERROR;
	}
	//包头包尾等于指定值判定该消息可解包
	if(readmsg[0] == MSG_HEAD && readmsg[Empty_Msg_BufferLength -2] == MSG_END)
	{
		//该消息为此次发送的回复结果
		if(readmsg[1] == MsgId)
		{
			if(readmsg[2] == MSGRETURN)
			{
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
				//该消息无法处理，重发
				tryed_resend++;
				usleep(1000);
				goto Resend;
			}
		}
		else if(readmsg[1] == 0x00 && readmsg[2] == RECV_ACK)
		{
			printf("Get lost of msgid.\n");
			return -2;
		}
	}
	//error_packs++;
	//hex_dump(readmsg, Empty_Msg_BufferLength,Empty_Msg_BufferLength, "ERR");
	//printf("Can not receive a correct pack, resend the last message.\n");
	//不满足规则的其他包
	tryed_resend++;
	usleep(1000);
	MsgIDCountDown();
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

static int INIT_Checking(void)
{
	uint8_t count = 10;
	uint8_t checkmsg[Empty_Msg_BufferLength];
	uint8_t returnmsg[Empty_Msg_BufferLength];
	uint8_t checkrecv[Empty_Msg_BufferLength];
	memset(checkmsg, 0, Empty_Msg_BufferLength);
	memset(returnmsg, 0, Empty_Msg_BufferLength);
	memset(checkrecv, 0, Empty_Msg_BufferLength);
	checkmsg[0] = MSG_HEAD;
	checkmsg[1] = 0x00;
	checkmsg[2] = RECV_ACK;
	checkmsg[Empty_Msg_BufferLength - 2] = MSG_END;
	checkmsg[Empty_Msg_BufferLength - 1] = crc_high_first(checkmsg, Empty_Msg_BufferLength - 1);
	returnmsg[0] = MSG_HEAD;
	returnmsg[1] = 0x01;
	returnmsg[2] = RECV_ACK;
	returnmsg[Empty_Msg_BufferLength - 2] = MSG_END;
	returnmsg[Empty_Msg_BufferLength - 1] = crc_high_first(returnmsg, Empty_Msg_BufferLength - 1);

	//第一次沟通 我发 00 ACK，应该要收到FF 上电
while(1){
		transfer(spi_fd, checkmsg, checkrecv, Empty_Msg_BufferLength);
		//沟通后发现收到FF PWR 说明副板上电中，或者00 ACK说明之前已经收过00 ACK，都需要去下发01 ACK
		if((checkrecv[1] == 0xFF && checkrecv[2] == RECV_PWRUP)
		 || (checkmsg[2] == 0x00 && checkrecv[2] == RECV_ACK)
		 || (checkrecv[1] == 0xFF && checkrecv[2] == RECV_NCK))
		{
			//收到从回复，开始发01 ACK，应该收到00 ACK
			transfer(spi_fd, returnmsg, checkrecv, Empty_Msg_BufferLength);
			if(checkrecv[1] == 0x00 && checkrecv[2] == RECV_ACK)
			{
				GetMsgID();
				return RET_OK; //确实收到00 ACK 沟通完成
			}
		}
	}
	//其他情况下，比如没收到消息格式的包，或者发01 ACK没能收到 00 ACK的情况下，都重新发00 ACK，也就是去到循环开头
}
void RecreateEmptyPack()
{
	empty[1] = GetMsgID();
	empty[Empty_Msg_BufferLength - 1] = crc_high_first(empty, Empty_Msg_BufferLength);
}

/* Public functions ----------------------------------------------------------*/
int32_t bus_spi_init(uint32_t port, uint32_t freq, void *other)
{
	snprintf(device, DEVICE_NAME_LENTH, "/dev/spidev0.%d", port);
	speed = freq;
	memset(empty, 0, Empty_Msg_BufferLength);
	//empty[Empty_Msg_BufferLength - 1] = 0xAA;
  	return RET_OK;
}

int32_t bus_spi_open(void)
{
	uint8_t checktime = 3;
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
		
	SPI_BUS_STATUS = STAT_Unkown;

	// uint8_t CheckReady[5];
	// memset(CheckReady, 0, 5);
	// CheckReady[0] = 0xFF;
	// uint8_t read_buff[Empty_Msg_BufferLength];
	// memset(read_buff, 0, Empty_Msg_BufferLength);
	// while(checktime > 0)
	// {
	// 	sleep(3);
	// 	io_spi_write(CheckReady, 5);
	// 	io_spi_read(read_buff ,Empty_Msg_BufferLength);
	// 	if(read_buff[2] == RECV_ACK)
	// 	{
	// 		checktime--;
	// 	}
	// }
  	return RET_OK;
}

//应该重写，仅进行收发操作
int32_t bus_spi_write(char *data, uint32_t len)
{	
	//判定包长
 	uint16_t sendpacklength;
	uint16_t packages;
	uint16_t shortpacks;
	uint32_t sendoffset;
	int32_t RET;
RP:
	sendpacklength = MAX_PACKLENGTH;
	packages = (len - 1) / MAX_PACKLENGTH + 1;
	shortpacks = 0;
	sendoffset = 0;

	while(packages > 0){
		//确认收发包有大于门限个可发送或者大于剩余可发送
		RemainPack = MSG_SendQUERYPACK();
		if(RemainPack < 0)
		{
			printf("Subboard cannot return querylength.\n");
			if(SPI_BUS_STATUS == STAT_SendingComplete)
			{
				for(int i = 0;i < last_sendpack_count;i++)
				{
					memset(FullsizePack, 0, last_sendpack_length);
					RET = MSG_SendData(FullsizePack, last_sendpack_length);
					if(RET < 0 && i > 0)
					{
						printf("Recover complete from missed data.\n");
						goto RP;
					}
				}
			}
			return RET_ERROR;
		}
		while(RemainPack < packages && RemainPack < PACK_GATE)
		{
			usleep(1000);
			RemainPack = MSG_SendQUERYPACK();
			if(RemainPack < 0)
			{
				printf("Subboard cannot return querylength.\n");
				if(SPI_BUS_STATUS == STAT_SendingComplete)
				{
					for(int i = 0;i < last_sendpack_count;i++)
					{
						memset(FullsizePack, 0, last_sendpack_length);
						RET = MSG_SendData(FullsizePack, last_sendpack_length);
						if(RET < 0 && i>0)
						{
							printf("Recover complete from missed data.\n");
							goto RP;
						}
					}
				}
				return RET_ERROR;
			}
		}
		
		SPI_BUS_STATUS = STAT_CheckForReady;
		//确认收发包有大于门限个可发送或者大于剩余可发送
		if(packages == 1)//仅一个包需要重新判定大小
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
				shortpacks = packages;
			}
		}
		//告诉副板我要发的数据包长度与包数量，长度不大于MAX_PACKLENGTH，包数量不大于MAX_PACK, 注意》》是否包长要算上包头，MSGID，包尾，算上就要 + 3
		
		last_sendpack_length = 9;
		last_sendpack_count = 1;
		usleep(100);
		RET = MSG_SendLength(sendpacklength + 3, shortpacks);
		usleep(100);
		if(RET < 0)
		{
			printf("Subboard cannot set data pack length\n");
			memset(FullsizePack, 0, sendpacklength);
			for(int i = 0;i < sendpacklength;i++)
			{
				RET = MSG_SendData(FullsizePack, sendpacklength);
				usleep(1000);
			}
			if(RET >= 0)
			{
				printf("Recover Complete, resend the data\n");
				goto RP;
			}
			else
				return RET_ERROR;
		}
		SPI_BUS_STATUS = STAT_BeforeSendData;
		//将数据分包进行发送
		
		last_sendpack_length = sendpacklength + 3;
		last_sendpack_count = shortpacks;
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
		ReSend:
			RET = MSG_SendData(FullsizePack, sendpacklength);
			usleep(700);
			if(RET == -2)
			{
				//出现00id，但是是ack的回复，重新进行指令发送并重新发数据
				if(i == shortpacks -1)
				{
					printf("MsgId 00 is the last message.\n");
					goto RP;
				}
				else //出现该问题但不是最后一个数据包，重发数据
				{
					printf("MsgId 00 is not the last message.\n");
					MsgIDCountDown();
					MsgIDCountDown();
					goto ReSend;
				}
			}
			else if(RET < 0)
			{
				printf("Send data with unknown error, MsgId:%d\n", MsgId);
				// printf("try to check status.\n");
				// int new_ret = MSG_SendQUERYPACK();
				// if(new_ret >=0)
				// {
				// 	printf("Subboard get Msg withId:%d\n", MsgId);
				// }
				// else
				// {
				// 	MsgIDCountDown();
				// 	RET = MSG_SendData(FullsizePack, sendpacklength);
				// 	if(RET >= 0)
				// 	{
				// 	printf("Subboard get Msg withId:%d\n", MsgId);

				// 	}
				// }
					return RET_ERROR;
			}
			SPI_BUS_STATUS = STAT_SendingData;
		}

		//判断是否还有需要发送的包
		packages = packages - shortpacks;
		if(packages == 0)
		{
			SPI_BUS_STATUS = STAT_SendingComplete;
		}
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

#if 0

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




//建立消息List，发送一个消息以后给这个消息列表加一个msgId

//发送消息与收包消息同时进行，收到的消息包不是以255 255 0结尾就再发一个指定长度(EMTPY_MSG_LENGTH)的空包，直到收到包含255 255 0结尾的包作为接收结束
//对于接收的消息进行
//上电需要收发一个消息，我要发一个00 ff的包，收00 ACK或者FF NCK，发到我收到00 ACK完成上电重启,此时用于收数据的包应该是01 ACK而不是empty
//每次发送数据包的同时会收到上一个数据的msgid和包长，如果出现本应退出发数据状态缺收到0404就发对应改长度的错CRC包，
	// 64X4设置数据包测试
	
/* 	uint32_t MessageCount = 1027;
	int sendpackCount = 500000;
	uint8_t BufMsg[MessageCount];
	memset(BufMsg, 0, MessageCount);
	BufMsg[0] = 250;
	BufMsg[MessageCount - 2] = 0xFF;
	BufMsg[MessageCount - 1] = 0xFF;
	for(int i = 0;i < (MessageCount - 3) / 4 - 1;i++)
	{
		BufMsg[i * 4 + 1] = ((i + 1) >> 8) & 0xFF;
		BufMsg[i * 4 + 2] = ((i + 1) & 0xFF);
		BufMsg[i * 4 + 3] = (((15 * i) % 360) >> 8) & 0xFF;
		BufMsg[i * 4 + 4] = ((15 * i)% 360) & 0xFF;
	} */
	
	/*
	uint32_t MessageCount = 7;
	int sendpackCount = 5000000;
	char BufMsg[MessageCount];
	memset(BufMsg, 0, MessageCount);
	BufMsg[0] = 0xE6;
	BufMsg[MessageCount - 2] = 0xFF;
	BufMsg[MessageCount - 1] = 0xFF;
	uint16_t index = 0;
	*/
/* 	ret = bus_spi_init(0, 21000000, NULL);
	ret = bus_spi_open();
	uint8_t REBOOT[5];
	uint8_t CheckReady[5];
	uint8_t read_buff[Empty_Msg_BufferLength];
	memset(read_buff, 0, Empty_Msg_BufferLength);
	memset(REBOOT, 0, 5);
	memset(CheckReady, 0, 5);
	REBOOT[0] = 0xFE;
	CheckReady[0]=0xFF;
	//重启
	io_spi_write(REBOOT, 5);
	io_spi_read(read_buff ,Empty_Msg_BufferLength);
	sleep(1);
	int checktime = 3;
	while(checktime > 0)
	{
		io_spi_write(CheckReady, 5);
		io_spi_read(read_buff ,Empty_Msg_BufferLength);
		hex_dump(read_buff, Empty_Msg_BufferLength,32,"READ");
		if(read_buff[2] == RECV_ACK)
		{
			checktime--;
		}
		usleep(1000);
	}
	 */
/* 	uint8_t firstpack[Empty_Msg_BufferLength];
	uint8_t secondpack[Empty_Msg_BufferLength];
	uint8_t thirdpack[Empty_Msg_BufferLength + 1];
	memset(firstpack, 0, Empty_Msg_BufferLength);
	memset(secondpack, 0, Empty_Msg_BufferLength);
	memset(thirdpack, 0, Empty_Msg_BufferLength + 1);
	firstpack[0] = MSG_HEAD;
	firstpack[1] = GetMsgID();
	firstpack[2] = MSG_QUERYPACK;
	firstpack[Empty_Msg_BufferLength - 2] = MSG_END;
	firstpack[Empty_Msg_BufferLength - 1] =crc_high_first(firstpack, Empty_Msg_BufferLength - 1);

	secondpack[0] = MSG_HEAD;
	secondpack[1] = GetMsgID();
	secondpack[2] = 0xFF;
	secondpack[Empty_Msg_BufferLength - 2] = 0x08;
	secondpack[Empty_Msg_BufferLength - 1] = crc_high_first(secondpack, Empty_Msg_BufferLength - 1);

	for(int i = 0;i< Empty_Msg_BufferLength + 1;i++)
	{
		thirdpack[i] = i * 4 + 3;
	}

	uint8_t read_buff[Empty_Msg_BufferLength];
	memset(read_buff, 0, Empty_Msg_BufferLength);
	uint8_t secread_buff[Empty_Msg_BufferLength + 1];
	memset(secread_buff, 0, Empty_Msg_BufferLength + 1);
 */
	// uint8_t CheckReady[5];
	// memset(CheckReady, 0, 5);
	// CheckReady[0]=0xFF;
/* 	int checktime = 3;
	ret = bus_spi_init(0, 21000000, NULL);
	ret = bus_spi_open();
	while(checktime > 0)
	{
		transfer(spi_fd, firstpack, read_buff, Empty_Msg_BufferLength);
		hex_dump(firstpack, Empty_Msg_BufferLength, 32, "SEND");
		hex_dump(read_buff,Empty_Msg_BufferLength, 32, "SENDREC");
		transfer(spi_fd, secondpack, read_buff, Empty_Msg_BufferLength);
		hex_dump(secondpack, Empty_Msg_BufferLength, 32, "SEND");
		hex_dump(read_buff,Empty_Msg_BufferLength, 32, "SENDREC");
		transfer(spi_fd, thirdpack, secread_buff, Empty_Msg_BufferLength + 1);
		hex_dump(thirdpack, Empty_Msg_BufferLength + 1, 32, "SEND");
		hex_dump(secread_buff, Empty_Msg_BufferLength + 1, 32, "SENDREC");
		//io_spi_write(CheckReady, 5);
		//io_spi_read(read_buff ,Empty_Msg_BufferLength);
		//hex_dump(read_buff, Empty_Msg_BufferLength,32,"READ");
		sleep(1);
	} */
/* 	while(sendpackCount--)
	{
	 	ret = bus_spi_write(BufMsg, MessageCount);

		//  BufMsg[1] = index >> 8 & 0x7F;
		//  BufMsg[2] = index & 0xFF;
		//  index++;
		//  index = index & 0x7FFF;

		 if(ret < 0)
		 {
			 printf("Send failed.\n");
		 }
	} */
#endif
int main(void)
{
	ret = bus_spi_init(0, 21000000, NULL);
	ret = bus_spi_open();
	ret = INIT_Checking();

	uint32_t MessageCount = 1027;
	int sendpackCount = 50000;
	uint8_t BufMsg[MessageCount];
	memset(BufMsg, 0, MessageCount);
	BufMsg[0] = 250;
	BufMsg[MessageCount - 2] = 0xFF;
	BufMsg[MessageCount - 1] = 0xFF;
	for(int i = 0;i < (MessageCount - 3) / 4 - 1;i++)
	{
		BufMsg[i * 4 + 1] = ((i + 1) >> 8) & 0xFF;
		BufMsg[i * 4 + 2] = ((i + 1) & 0xFF);
		BufMsg[i * 4 + 3] = (((15 * i) % 360) >> 8) & 0xFF;
		BufMsg[i * 4 + 4] = ((15 * i)% 360) & 0xFF;
	}

	while(sendpackCount--){
		bus_spi_write(BufMsg, MessageCount);
	}
	// uint8_t CheckReady[5];
	// memset(CheckReady, 0, 5);
	// CheckReady[0]=0xFF;
/* 	int checktime = 3;
	ret = bus_spi_init(0, 21000000, NULL);
	ret = bus_spi_open();
	while(checktime > 0)
	{
		transfer(spi_fd, firstpack, read_buff, Empty_Msg_BufferLength);
		hex_dump(firstpack, Empty_Msg_BufferLength, 32, "SEND");
		hex_dump(read_buff,Empty_Msg_BufferLength, 32, "SENDREC");
		transfer(spi_fd, secondpack, read_buff, Empty_Msg_BufferLength);
		hex_dump(secondpack, Empty_Msg_BufferLength, 32, "SEND");
		hex_dump(read_buff,Empty_Msg_BufferLength, 32, "SENDREC");
		transfer(spi_fd, thirdpack, secread_buff, Empty_Msg_BufferLength + 1);
		hex_dump(thirdpack, Empty_Msg_BufferLength + 1, 32, "SEND");
		hex_dump(secread_buff, Empty_Msg_BufferLength + 1, 32, "SENDREC");
		//io_spi_write(CheckReady, 5);
		//io_spi_read(read_buff ,Empty_Msg_BufferLength);
		//hex_dump(read_buff, Empty_Msg_BufferLength,32,"READ");
		sleep(1);
	} */
	return 0;
}


