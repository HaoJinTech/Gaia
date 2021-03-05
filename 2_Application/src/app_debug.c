/**
  ******************************************************************************
  * @file    app_debug.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "app_debug.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void print_hex(const char* data, int data_len)
{
    int i, j;
    unsigned int offset = 0;
    int total_row = data_len / 16;
    int left_data_len = data_len % 16;

    /* print data_len / 16 */
    for (j = 0; j < total_row; j++)
    {
        printf("# %08X   ", offset);

        for (i = 0; i < 16; i++)
        {
            printf("%02X ", data[j*16+i]);
        }

        for (i = 0; i < 16; i++)
        {
            if ((data[j*16+i] < 0x20) || (data[j*16+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*16+i]);
            }
        }
        offset += 16;
        printf("\n");
    }

    /* print data_len % 16 */
    if (left_data_len > 0)
    {
        printf("# %08X   ", offset);
        for (i = 0; i < left_data_len; i++)
        {
            printf("%02X ", data[total_row*16+i]);
        }
        for (i = 0; i < 16 - left_data_len; i++)
        {
            printf("   ");
        }
        for (i = 0; i < left_data_len; i++)
        {
            if ((data[total_row*16+i] < 0x20) || (data[total_row*16+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*16+i]);
            }
        }
        printf("\n");
    }
    return;
} /* print_data */