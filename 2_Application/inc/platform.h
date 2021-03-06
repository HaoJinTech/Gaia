/**
  ******************************************************************************
  * @file    platform.h
  * @author  YORK
  * @version V0.1.0
  * @date    02-28-2021
  * @brief   
  *
	********** Copyright (C), 2014-2022,HJ technologies **************************
	*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  USER_CONFIG_H
#define  USER_CONFIG_H

#include "stdint.h"

#define PRJ_FILE_PATH  "./prj_files/" 
#define CLI_NUM_MAX         64
#define SERVER_PORT_DEF     50000

/* PROTOCLOL ID */
#define PROTOCOL_ID_RR485       0
#define PROTOCOL_ID_HSSPI       1

/* BUS ID */
#define BUS_ID_SPI      0
#define BUS_ID_UART     1

/* DEST TYPE */
#define DEST_ATT        0x01
#define DEST_PHA        0x02
#define DEST_ATT_PHA    0x03
#define DEST_SWITCH     0x10

#define LOCAL              static
#define RET_OK             1
#define RET_ERROR         -1

#endif
