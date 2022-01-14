#ifndef DEVICE_H
#define DEVICE_H
#include <stdio.h>
#include <stdlib.h>
#include "hardware/uart.h"
#include "modbus.h"
#include "modbusRTU.h"
#include "../config.h"

#define MN_MAX_LENGTH     30
#define PW_LENGTH     6
#define PollutionDataLen  4
#define checkAddr  5  //1 2 3 4 5
#define configAddr 30

typedef enum {
    gps_yearAddr,
    gps_monthAddr,
    gps_dateAddr,
    gps_hourAddr,
    gps_minuteAddr,
    gps_secondAddr,
    gps_stateAddr,
    gps_latitudeAddr,
    gps_latitudeFlagAddr = gps_latitudeAddr+4,
    gps_longitudeAddr,
    gps_longitudeFlagAddr=gps_longitudeAddr+4,
} gpsAddr;//uint 8

#define InputDetectAddr (checkAddr+gps_longitudeFlagAddr+1)/2+1

typedef enum {
    poolNumsAddr,
    poolNumAddr,
    pollutionNumsAddr,
    MN_lenAddr,
    MNAddr,
#ifdef UART_JIANGNING
    YearMonthAddr = 4,
#else
    YearMonthAddr = MNAddr+12,
#endif
    DateHourAddr,
    MinuteSecondAddr,
    pollutionCodeAddr,
#ifdef UART_JIANGNING
    pollutionDataAddr = 0,
#else
    pollutionDataAddr,
#endif
    pollutionStateAddr=pollutionDataAddr+2,
} deviceDataAddr;//uint 16




#define UT_REGISTERS_ADDRESS 0x57
#define LED_POOL_ADDRESS 0x10C0
#define LED_VALUE_ADDRESS 0x5042


#define COMMON_DEVICE_REGISTERS_ADDRESS 0x0340
#define COMMON_DEVICE_nb_points 0x20

#ifdef UART_JIANGNING
    #define COMMON_DEVICE_MN "32018880000001"
#else
    #define COMMON_DEVICE_MN "88888880000001"
#endif
#define COMMON_DEVICE_CODE "011"

#define nb_points 0x2B
#define setLedValueNums 6
#define ledAddr 80
#define deviceAddr 1
#define uploadAddr 0x80

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
response_type_t ask_common_device(modbus_t *ctx, deviceData_t **deviceData);

deviceData_t* new_deviceData(uint16_t poolNums,uint16_t pollutionNums,uint8_t MN_len);
void setPollutionNums(deviceData_t *deviceData,uint16_t pollutionNums);
uint8_t checkMN(uint8_t MN_len,uint16_t *current_MN,uint8_t *deviceData_MN);
uint8_t set_led_value(modbus_t *ctx, deviceData_t *deviceData);
uint8_t set_led_values(modbus_t *ctx, uint8_t pool,uint16_t valueNums,uint8_t *valueDatas);
uint8_t set_led_valueByAddr(modbus_t *ctx,uint16_t led_value_address,float *data,uint16_t dataLen);
void addNewDate(deviceData_t *deviceData, uint16_t *tab_rp_registers);

#endif