#ifndef CRC_H
#define CRC_H
#include <stdio.h>

unsigned int crc16_J212(uint8_t *puchMsg, uint16_t start, uint16_t len);
unsigned int crc16(uint8_t* puchMsg, uint16_t start,uint16_t len);

#endif