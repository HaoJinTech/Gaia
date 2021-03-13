#ifndef  _CRC_H_
#define  _CRC_H_

#define _CRT_SECIRE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdint.h>
#include <stdio.h>

unsigned char crc_high_first(unsigned char *ptr, uint32_t len);

#endif