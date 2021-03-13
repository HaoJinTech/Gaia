/**
  ******************************************************************************
  * @file    subbd_protocol.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _SUBBD_PROTOCOL_H_
#define  _SUBBD_PROTOCOL_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus_prototype.h"

#include <semaphore.h>
/* Exported types ------------------------------------------------------------*/
typedef struct data_type_FLEX
{
    long data_type; // DATA_TYPE_FLEX
    long dest_type;

    void *value;
}FLEX;

typedef struct data_type_SCSV
{
    long data_type; // DATA_TYPE_SCSV
    long dest_type;

    int32_t channel;
    int32_t value;
}SCSV;

typedef struct data_type_MCMV
{
    long data_type; // DATA_TYPE_MCMV
    long dest_type;

    int32_t *channel;
    int32_t *value;
    uint32_t ch_lenth;
}MCMV;

typedef struct data_type_CCMV
{
    long data_type; // data_type_CCMV
    long dest_type;

    int32_t offset;
    int32_t ch_lenth;
    int32_t *value;
}CCMV;

typedef struct data_type_CCSV
{
    long data_type; // DATA_TYPE_CCSV
    long dest_type;

    int32_t offset;
    int32_t ch_lenth;
    int32_t value;
}CCSV;

typedef struct data_type_MCMMV
{
    long data_type; // DATA_TYPE_MCMMV
    long dest_type;

    int32_t *channel;
    int32_t ch_lenth;
    int32_t *value;
    int32_t val_count;  // the count of values that will send at same time, (if ATT & PHA enable, val_count = 2)
}MCMMV;

typedef struct data_type_CCMMV
{
    long data_type; // DATA_TYPE_CCMMV
    long dest_type;

    int32_t offset;
    int32_t ch_lenth;
    int32_t *value;
    int32_t val_count;  // the count of values that will send at same time, (if ATT & PHA enable, val_count = 2)
}CCMMV;
typedef struct subbd_protocol SUBBD_PROTOCOL;
typedef int32_t (*subbd_protocol_init)(SUBBD_PROTOCOL *devs, void *param);
typedef int32_t (*subbd_protocol_open)(SUBBD_PROTOCOL *devs, void *param);
typedef int32_t (*subbd_protocol_write)(SUBBD_PROTOCOL *devs, BUS_DRIVER *bus, void *data);
typedef void *(*subbd_protocol_read)(SUBBD_PROTOCOL *devs, BUS_DRIVER *bus, int len);
typedef int32_t (*subbd_protocol_ioctrl)(SUBBD_PROTOCOL *devs, int request, ...);
typedef int32_t (*subbd_protocol_close)(SUBBD_PROTOCOL *devs, void *param);

struct subbd_protocol
{
    int     protocol_id;

    sem_t       *sem_rx_ready;
    void        *data;     // private data

    subbd_protocol_init  init;
    subbd_protocol_open  open;
    subbd_protocol_write write;
    subbd_protocol_read  read;
    subbd_protocol_ioctrl ioctrl;
    subbd_protocol_close close;

};

/* Exported constants --------------------------------------------------------*/
extern SUBBD_PROTOCOL protocols[];
extern uint32_t SUBBD_PROTOCOL_SIZE;

/* Exported macro ------------------------------------------------------------*/
#define IO_CTRL_MSG_START_CASE_UPLOAD        0x0019
#define IO_CTRL_MSG_UPDATE_CASE_LINE         0x0020

/* DEST TYPE */
#define DEST_ATT             0x01
#define DEST_PHA             0x02
#define DEST_ATT_PHA         0x03
#define DEST_SWITCH          0x10
#define DEST_SWITCH_NPNT     0x11
#define DEST_UPLD_ATT        0x21
#define DEST_UPLD_PHA        0x22
#define DEST_UPLD_ATT_PHA    0x23
#define DEST_UPLD_ATT_PHA_EX 0x24

/* Exported functions --------------------------------------------------------*/ 

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/

