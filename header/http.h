#ifndef __HTTP_H
#define __HTTP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "device.h"
uint8_t uploadJSON(deviceData_t *deviceData, datetime_t *currentDate, uart_inst_t *uart, uint8_t uart_en_pin);

#define ID                 no
#define siteID             siteNo
#define siteName           siteName
#define status             status
#define ph_unit            ph_unit
#define wt_unit            wt_unit
#define do_unit            do_unit
#define ec_unit            ec_unit
#define turbidity_unit     turbidity_unit
#define time               time

#ifdef __cplusplus
}
#endif

#endif 

