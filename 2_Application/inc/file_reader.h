/**
  ******************************************************************************
  * @file    file_reader.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _FILE_READER_H_
#define  _FILE_READER_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "rbuffer.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*File_reader)(struct rb *rb, void *dest_obj, uint32_t line);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
uint32_t csv_read_file(const char *filename, File_reader file_reader, void *dest_obj);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
