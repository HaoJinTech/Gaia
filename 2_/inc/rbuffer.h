/**
  ******************************************************************************
  * @file    rbuffer.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _RBUFFER_H_
#define  _RBUFFER_H_
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "list.h"
/* Exported types ------------------------------------------------------------*/
typedef struct rb
{
    char name[32];
    uint32_t buffer_max_size;

    uint32_t read_index, write_index;
    char *buffer_ptr;
    uint32_t buffer_size;

    struct list_head list;
}RBUF;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define RB_MAX_SIZE		2048

//typedef struct rb RBUFFER;
/* Exported functions --------------------------------------------------------*/ 
struct rb * rb_malloc(char *name, uint16_t size, uint32_t buffer_max_size);
void rb_init(struct rb* rb, char *pool, uint16_t size);
void rb_free(struct rb* rb);
void rb_cleanup(struct rb* rb);
void rb_empty(struct rb *rb);

uint32_t rb_put(struct rb* rb, const uint8_t *ptr, uint16_t length);
uint32_t rb_putchar(struct rb* rb, const uint8_t ch);
uint32_t rb_get(struct rb* rb, char *ptr, uint16_t length);
char *rb_getline(struct rb* rb, char *tailed, uint16_t tcount, uint32_t *rx_size);
uint32_t rb_line_available(struct rb* rb, const uint8_t *tailed, uint16_t tcount);
uint32_t rb_available(struct rb* rb);
uint32_t rb_lenth(struct rb* rb);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
