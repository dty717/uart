#ifndef __BEIDOU_H
#define __BEIDOU_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "device.h"
#define Send_Info_CMD    "CCTCQ"

uint8_t checkBuf(uint8_t* recBuf, uint16_t recBufLen);
uint8_t uploadBeidou(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin);

#ifdef __cplusplus
}
#endif

#endif 

