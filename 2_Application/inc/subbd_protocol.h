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

/* Exported types ------------------------------------------------------------*/
typedef struct data_type_SCSV
{
    char data_type; // DATA_TYPE_SCSV

    int32_t channel;
    int32_t value;
}SCSV;

typedef struct data_type_MCMV
{
    char data_type; // DATA_TYPE_SCSV

    int32_t *channel;
    int32_t *value;
    uint32_t lenth;
}MCMV;

typedef struct data_type_CCMV
{
    char data_type; // DATA_TYPE_CCSV

    int32_t offset;
    int32_t lenth;
    uint32_t *value;
}CCMV;

typedef struct data_type_CCSV
{
    char data_type; // DATA_TYPE_CCSV

    int32_t offset;
    int32_t lenth;
    uint32_t value;
}CCSV;

typedef struct ctrl_msg
{
    uint32_t  type;
    void      *val;
}CTRL_MSG;

typedef int32_t (*subbd_protocol_init)(void *param);
typedef int32_t (*subbd_protocol_open)(void *param);
typedef int32_t (*subbd_protocol_write)(void *data);
typedef void *(*subbd_protocol_read)(int len);
typedef int32_t (*subbd_protocol_ioctrl)(CTRL_MSG *ctrl);
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
#define PROTOCOL_ID_RR485       0
#define PROTOCOL_ID_HSSPI       1

/* Exported functions --------------------------------------------------------*/ 

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/

