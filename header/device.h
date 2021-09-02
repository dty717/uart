#ifndef DEVICE_H
#define DEVICE_H
#include <stdio.h>
#include <stdlib.h>
#include "hardware/uart.h"
#include "modbus.h"
#include "modbusRTU.h"

#define MN_MAX_LENGTH     30
#define PW_LENGTH     6
#define PollutionDataLen  4
typedef enum {
    poolNumAddr,
    poolNumsAddr,
    pollutionNumsAddr,
    MN_lenAddr,
    MNAddr,
    YearMonthAddr = MNAddr+12,
    DateHourAddr,
    MinuteSecondAddr,
    pollutionCodeAddr,
    pollutionDataAddr,
    pollutionStateAddr=pollutionDataAddr+2,
} deviceDataAddr;


#define UT_REGISTERS_ADDRESS 0x57
#define LED_POOL_ADDRESS 0x10C0
#define LED_VALUE_ADDRESS 0x5042

#define nb_points 0x2B
#define setLedValueNums 6
#define ledAddr 80
#define deviceAddr 1

#define remainingPollutionNums 2
typedef enum {
    noResponse=0,
    newData,
    normal,
    MN_len_erro,
    poolNums_erro,
} response_type_t;


typedef struct pollution {
    uint8_t *code;
    double data;
    uint16_t state;
    uint32_t time;
} pollution_t;

typedef struct deviceData {
    /* Slave address */
    uint16_t poolNums;
    /* Socket or file descriptor */
    uint16_t pollutionNums;
    uint8_t MN_len;
    uint8_t *MN;
    uint8_t *PW;
    uint8_t poolNum;
    pollution_t *pollutions;
    uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} deviceData_t;
uint8_t *pollutionCode(uint16_t code);

response_type_t ask_all_devices(modbus_t *ctx, deviceData_t **deviceData);
response_type_t ask_device(modbus_t *ctx, deviceData_t **deviceData);

deviceData_t* new_deviceData(uint16_t poolNums,uint16_t pollutionNums,uint8_t MN_len);
void setPollutionNums(deviceData_t *deviceData,uint16_t pollutionNums);
uint8_t checkMN(uint8_t MN_len,uint16_t *current_MN,uint8_t *deviceData_MN);
uint8_t set_led_value(modbus_t *ctx, deviceData_t *deviceData);
uint8_t set_led_valueByAddr(modbus_t *ctx,uint16_t led_value_address,float *data,uint16_t dataLen);
void addNewDate(deviceData_t *deviceData, uint16_t *tab_rp_registers);

#endif