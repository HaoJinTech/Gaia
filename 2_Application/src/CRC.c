#include "CRC.h"

unsigned char Divider = 0x07;

unsigned char crc_high_first(unsigned char *ptr, uint32_t len)
{
    unsigned char i;
    char crc = 0x00; /* 计算的初始crc值 */ 
 
    while(len--)
    {
        crc ^= *ptr++;  /* 每次先与需要计算的数据异或,计算完指向下一数据 */  
        for (i = 8; i > 0; --i)   /* 下面这段计算过程与计算一个字节crc一样 */  
        { 
            if (crc & 0x80)
                crc = (crc << 1) ^ Divider;
            else
                crc = (crc << 1);
        }
    }
 
    return (crc);
}

