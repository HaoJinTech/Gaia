/**
  ******************************************************************************
  * @file    user_config.h
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

#define SYS_CONFIG_PATH   "./gaia.conf"
#define CLI_NUM_MAX        64
#define SERVER_PORT_DEF    50000

#define TCP_RX_BUFFER_SIZE_DEF      4096
#define TCP_RX_BUFFER_SIZE_MAX_DEF  65536

#define LOCAL              static
#define RET_OK             1
#define RET_ERROR         -1

#endif
