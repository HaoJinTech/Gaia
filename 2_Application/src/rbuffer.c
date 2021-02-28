/**
  ******************************************************************************
  * @file    rbuffer.c
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   Round buffer stuffs, used for telnet.
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "app_debug.h"
#include "rbuffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef RB_ADDITION_SIZE
#	define RB_ADDITION_SIZE 128
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static LIST_HEAD(rb_item_root);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
struct rb * rb_malloc(char *name, uint16_t size, uint32_t buffer_max_size)
{
    uint8_t *pbuf = NULL;
    struct rb *prb = NULL;
    
    pbuf = (uint8_t*)malloc(size);
    prb = (struct rb*)malloc(sizeof(struct rb));
    if(pbuf == NULL || prb == NULL){
        return NULL;
    }
    prb->buffer_ptr = pbuf;
    prb->buffer_size = size;

    prb->read_index = prb->write_index = 0;
    prb->buffer_max_size = buffer_max_size;
    if(name){
        sprintf(prb->name,"%s",name);
    }else{
        sprintf(prb->name,"NULL");
    }
    list_add(&(prb->list), &rb_item_root);
    return prb;
}

/** --------------------------------------------------------------------------
  * @brief  Initialize a round buffer.
  * @note   None.
  * @param  rb     - Return the new round buffer.
	*         pool   - Allocated buffer pointer.
	*         size   - Size of the buffer.
  * @retval None.
  */
void rb_init(struct rb* rb, uint8_t *pool, uint16_t size)
{
	APP_ASSERT(rb);

	rb->read_index = rb->write_index = 0;

	rb->buffer_ptr = pool;
	rb->buffer_size = size;

    list_add(&(rb->list), &rb_item_root);
}

uint32_t rb_remalloc(struct rb* prb)
{
	uint8_t *pdata = NULL;
	uint32_t length = 0;
	if(prb->buffer_size >= RB_MAX_SIZE || prb->buffer_size>= prb->buffer_max_size)
		return 0;
	length = prb->buffer_size + RB_ADDITION_SIZE>RB_MAX_SIZE ? RB_MAX_SIZE:prb->buffer_size + RB_ADDITION_SIZE;
	pdata = (uint8_t*)malloc(length);
    APP_ASSERT(pdata);

	length = rb_lenth(prb);
	rb_get(prb, pdata, length);
	free(prb->buffer_ptr);
	prb->buffer_ptr = pdata;
	
	prb->buffer_size += RB_ADDITION_SIZE;
	prb->read_index = 0;
	prb->write_index = length;
	
	APP_DEBUGF(RB_DEBUG | APP_DBG_TRACE , 
	    ("rb_remalloc:(%s)remalloc round buffer.\r\n",prb->name));
	
	return 1;
}

/** --------------------------------------------------------------------------
  * @brief  Write data into the roundbuffer.
  * @note   None.
  * @param  rb     - The dest round buffer.
	*         pool   - The source buffer.
	*         size   - Size of the source buffer.
  * @retval None.
  */
uint32_t rb_put(struct rb* rb, const uint8_t *ptr, uint16_t length)
{
    uint32_t size;
retry:
    if (rb->read_index > rb->write_index)
        size = rb->read_index - rb->write_index;
    else
        size = rb->buffer_size - rb->write_index + rb->read_index;

    if (size < length){// length = size;
			if(rb_remalloc(rb))
				goto retry;
			else
				return 0;
		}
		
    if (rb->read_index > rb->write_index) {
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        rb->write_index += length;
    } else {
        if (rb->buffer_size - rb->write_index > length) {
            memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
            rb->write_index += length;
        } else {
            memcpy(&rb->buffer_ptr[rb->write_index], ptr,
                   rb->buffer_size - rb->write_index);
            memcpy(&rb->buffer_ptr[0], &ptr[rb->buffer_size - rb->write_index],
                   length - (rb->buffer_size - rb->write_index));
            rb->write_index = length - (rb->buffer_size - rb->write_index);
        }
    }

    return length;
}

/** --------------------------------------------------------------------------
  * @brief  Write a char into the roundbuffer.
  * @note   None.
  * @param  rb     - The dest round buffer.
	*         ch     - The source char.
  * @retval None.
  */
uint32_t rb_putchar(struct rb* rb, const uint8_t ch)
{
    uint16_t next;
retry:
    next = rb->write_index + 1;
    if (next >= rb->buffer_size) next = 0;

    if (next == rb->read_index){
			if(rb_remalloc(rb)){
				goto retry;
			}else{
				rb->read_index ++;
				if(rb->read_index >= rb->buffer_size){
					rb->read_index = 0;
				}
			}
		}

    rb->buffer_ptr[rb->write_index] = ch;
    rb->write_index = next;

    return 1;
}

/** --------------------------------------------------------------------------
  * @brief  read data from the roundbuffer.
  * @note   None.
  * @param  rb     - The source round buffer.
	*         ptr    - The dest buffer pointer.
	*         length - Desired length of round buffer.
  * @retval None.
  */
uint32_t rb_get(struct rb* rb, uint8_t *ptr, uint16_t length)
{
    uint32_t size;

    if (rb->read_index > rb->write_index)
        size = rb->buffer_size - rb->read_index + rb->write_index;
    else
        size = rb->write_index - rb->read_index;

    if (size == 0) return 0;

    if (size < length) length = size;

    if (rb->read_index > rb->write_index) {
        if (rb->buffer_size - rb->read_index > length) {
            memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
            rb->read_index += length;
        } else {
            memcpy(ptr, &rb->buffer_ptr[rb->read_index],
                   rb->buffer_size - rb->read_index);
            memcpy(&ptr[rb->buffer_size - rb->read_index], &rb->buffer_ptr[0],
                   length - rb->buffer_size + rb->read_index);
            rb->read_index = length - rb->buffer_size + rb->read_index;
        }
    } else {
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        rb->read_index += length;
    }

    return length;
}

uint8_t *rb_getline(struct rb* rb, uint8_t *tailed, uint16_t tcount, uint32_t *rx_size)
{
	uint16_t temp_index = rb->read_index;
	uint16_t temp_tcount = 0;
	uint8_t *line = NULL;
	uint16_t size = 0;

	if(rx_size == NULL)
		return NULL;
	*rx_size = 0;
	while(temp_index != rb->write_index){
		if(tailed[temp_tcount] == rb->buffer_ptr[temp_index]){
			temp_tcount++;
			if(temp_tcount >= tcount){
				size++;
				break;
			}
		}else if(temp_tcount != 0){ 
			temp_tcount = 0;
		}
		temp_index = temp_index >= rb->buffer_size -1? 0: temp_index+1;
		size ++;
	}
	
	if(temp_tcount == tcount){
		line = (uint8_t*)malloc(size+1);
		if(line == NULL)
			return NULL;
		
		rb_get(rb, line, size);
		line[size] = 0;
		*rx_size = size;
		return line;
	}
	return NULL;
}

/** --------------------------------------------------------------------------
  * @brief  Determine the round buffer is available.
  * @note   Intercept from the round buffer by the terminator.
  * @param  rb     - The round buffer.
	*         tailed - Terminator.
	*         tcount - Terminator size.
  * @retval None.
  */
uint32_t rb_line_available(struct rb* rb, const uint8_t *tailed, uint16_t tcount)
{
	uint16_t temp_index = rb->read_index;
	uint16_t temp_tcount = 0;
	uint16_t size = 0;

	while(temp_index != rb->write_index){
		if(tailed[temp_tcount] == rb->buffer_ptr[temp_index]){
			temp_tcount++;
			if(temp_tcount >= tcount){
				size++;
				break;
			}
		}else if(temp_tcount != 0){
			temp_tcount = 0;
		}
		temp_index = temp_index >= rb->buffer_size -1? 0: temp_index+1;
		size ++;
	}
	
	if(temp_tcount == tcount){
		return size;
	}
	return 0;
}

uint32_t rb_lenth(struct rb* rb)
{
	uint32_t size=0;

	if (rb->read_index > rb->write_index)
			size = rb->buffer_size - rb->read_index + rb->write_index;
	else
			size = rb->write_index - rb->read_index;

	return size;
}

void rb_cleanup(struct rb *rb)
{
	rb->read_index = 0;
	rb->write_index = 0;
}

void rb_free(struct rb *rb)
{
	free(rb->buffer_ptr);
    list_del(&(rb->list));
	free(rb);
}

void rb_empty(struct rb *rb)
{
	free(rb->buffer_ptr);
	list_del(&(rb->list));

}
/*
void list_rb(void)
{
	struct rt_list_node *node;
	struct rb *rb_item;

	rt_kprintf("round buffer list:\r\n");
	rt_kprintf("name\tsize\tused\r\n");
	for (node = rb_item_root.next; node != &(rb_item_root); node = node->next){
		rb_item = (struct rb *)rt_list_entry(node, struct rb, list);
		rt_kprintf("%s\t%d\r\n",rb_item->name, rb_item->buffer_size);
	}
}
*/