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

typedef struct data_type_CCMMV
{
    long data_type; // DATA_TYPE_CCMMV
    long dest_type;

    int32_t offset;
    int32_t ch_lenth;
    int32_t val_count;  // the count of values that will send at same time, (if ATT & PHA enable, val_count = 2)
    int32_t *value;
}CCMMV;

typedef struct protocl_ctrl_msg
{
    uint32_t  type;
    void      *val;
}PROTOCL_CTRL_MSG;

typedef int32_t (*subbd_protocol_init)(void *param);
typedef int32_t (*subbd_protocol_open)(void *param);
typedef int32_t (*subbd_protocol_write)(BUS_DRIVER *bus, void *data);
typedef void *(*subbd_protocol_read)(BUS_DRIVER *bus, int len);
typedef int32_t (*subbd_protocol_ioctrl)(PROTOCL_CTRL_MSG *ctrl);
typedef int32_t (*subbd_protocol_close)(void *param);

typedef struct subbd_protocol
{
    int     protocol_id;

    subbd_protocol_init  init;
    subbd_protocol_open  open;
    subbd_protocol_write write;
    subbd_protocol_read  read;
    subbd_protocol_ioctrl ioctrl;
    subbd_protocol_close close;

}SUBBD_PROTOCOL;

/* Exported constants --------------------------------------------------------*/
extern SUBBD_PROTOCOL protocols[];
extern uint32_t SUBBD_PROTOCOL_SIZE;

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/

