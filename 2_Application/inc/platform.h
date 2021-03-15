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

#include <stdint.h>
#include <stddef.h>

// featrue enable
#define SINGLE_PACKAGE    1

//#define PRJ_FILE_PATH  "/home/pi/git/Gaia/2_Application/prj_files" 
//#define PRJ_FILE_PATH "./prj_files" 
#define PRJ_FILE_PATH  "/usr/Gaia/prj_files"
#define CLI_NUM_MAX         64
#define SERVER_PORT_DEF     50000
#define FILE_PATH_LEN       128

/* PROTOCLOL ID */
#define PROTOCOL_ID_RR485       0
#define PROTOCOL_ID_HSSPI       1

/* BUS ID */
#define BUS_ID_DUMP     0
#define BUS_ID_SPI      1
#define BUS_ID_UART     2

#define LOCAL              static
#define RET_OK             1
#define RET_ERROR         -1

#endif
