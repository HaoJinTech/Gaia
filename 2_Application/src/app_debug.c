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
#define LINE_LEN    32

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void print_hex(const char* data, int data_len)
{
    int i, j;
    unsigned int offset = 0;
    int total_row = data_len / LINE_LEN;
    int left_data_len = data_len % LINE_LEN;

    /* print data_len / 16 */
    for (j = 0; j < total_row; j++)
    {
        printf("# %08X   ", offset);

        for (i = 0; i < LINE_LEN; i++)
        {
            if(i%8 == 0) printf(" ");
            printf("%02X ", data[j*LINE_LEN+i]);
        }

        for (i = 0; i < LINE_LEN; i++)
        {
            if(i%8 == 0) printf(" ");
            if ((data[j*LINE_LEN+i] < 0x20) || (data[j*LINE_LEN+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*LINE_LEN+i]);
            }
        }
        offset += LINE_LEN;
        printf("\n");
    }

    /* print data_len % 16 */
    if (left_data_len > 0)
    {
        printf("# %08X   ", offset);
        for (i = 0; i < left_data_len; i++)
        {
            printf("%02X ", data[total_row*LINE_LEN+i]);
        }
        for (i = 0; i < LINE_LEN - left_data_len; i++)
        {
            printf("   ");
        }
        for (i = 0; i < left_data_len; i++)
        {
            if ((data[total_row*LINE_LEN+i] < 0x20) || (data[total_row*LINE_LEN+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*LINE_LEN+i]);
            }
        }
        printf("\n");
    }
    return;
} /* print_data */