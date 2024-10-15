#ifndef DEVICE_H
#define DEVICE_H
#include <stdio.h>
#include <stdlib.h>
#include "hardware/uart.h"
#include "pico/util/datetime.h"
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
    YearMonthAddr = MNAddr+12,
    DateHourAddr,
    MinuteSecondAddr,
    pollutionCodeAddr,
    pollutionDataAddr,
    pollutionStateAddr=pollutionDataAddr+2,
} deviceDataAddr;//uint 16

typedef enum
{
    commonPollutionDataAddr,
    commonYearMonthAddr = 4,
    commonDateHourAddr,
    commonMinuteSecondAddr,
} commonDeviceDataAddr; // uint 16

#define PLC_DATE_REGISTERS_ADDRESS 0x19A

#if defined(UART_HUBEI) || defined(UART_TIBET) || defined(UART_SUQIAN_SENDING) || defined(UART_SUQIAN_RECEIVING) || defined(UART_SUZHOU)
#define UT_REGISTERS_ADDRESS 0x7D0
#elif defined(UART_TEST) 
    #define UT_REGISTERS_ADDRESS 0x7D0
#else
    #define UT_REGISTERS_ADDRESS 0x57
#endif

#define COMMON_DEVICE_REGISTERS_ADDRESS 0x0340
#define COMMON_DEVICE_nb_points 0x20

#define DEFAULT_nb_points 0x2

#ifdef UART_JIANGNING
    #define COMMON_DEVICE_MN "32018880000001"
    #define COMMON_DEVICE_CODE 11
    #define COMMON_MN_LEN     14
#elif defined(UART_SUZHOU) && !defined(usingMultiDevice)
    #define COMMON_DEVICE_MN "0000000D1000001000000014"
    #define COMMON_DEVICE_CODE 21003
    #define COMMON_MN_LEN     24
#else
    #define COMMON_DEVICE_MN "88888888888888"
    #define COMMON_DEVICE_CODE 11
    #define COMMON_MN_LEN     14
#endif

#ifdef UART_SUQIAN_RECEIVING
    #define setLedValueNums         4
    #define ledAddr              0x18
    #define LED_POOL_ADDRESS   0x7552
    #define LED_VALUE_ADDRESS  0x5080
#else
    #define setLedValueNums         6
    #define ledAddr                80
    #define LED_POOL_ADDRESS   0x10C0
    #define LED_VALUE_ADDRESS  0x5042
#endif

#define deviceAddr 1
#define uploadAddr 0x80

typedef enum {
    noResponse=0,
    newData,
    normal,
    MN_len_erro,
    poolNums_erro,
    date_erro,
} response_type_t;


typedef struct pollution {
    uint8_t *code;
    double data;
    uint16_t state;
    uint32_t time;
} pollution_t;

#define maxValueNums 100

typedef struct pollutions
{
    uint8_t *code;
    double data[maxValueNums];
    int dataIndex;
    uint16_t state;
    uint32_t time;
} pollutions_t;

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

typedef struct deviceDatas {
    /* Socket or file descriptor */
    uint16_t pollutionNums;
    uint8_t MN_len;
    uint8_t *MN;
    uint8_t *PW;
    uint8_t poolNum;
    pollutions_t *pollutions;
    uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} deviceDatas_t;


uint8_t *pollutionCode(uint16_t code);
uint8_t *pollutionName(uint16_t code);

response_type_t ask_all_devices(modbus_t *ctx, deviceData_t **deviceData);
response_type_t ask_device(modbus_t *ctx, deviceData_t **deviceData);
response_type_t ask_common_device(modbus_t *ctx, deviceData_t **deviceData);
response_type_t ask_probe(modbus_t *ctx, deviceData_t **deviceData, datetime_t *currentDate, uint8_t *code, uint8_t pollutionIndex, uint16_t device_addr, uint16_t valueAddr);
response_type_t ask_device_rtc(modbus_t *ctx, datetime_t *currentDate);

deviceData_t* new_deviceData(uint16_t poolNums,uint16_t pollutionNums,uint8_t MN_len);
void setPollutionNums(deviceData_t *deviceData,uint16_t pollutionNums);
uint8_t checkMN(uint8_t MN_len,uint16_t *current_MN,uint8_t *deviceData_MN);
uint8_t set_led_value(modbus_t *ctx, deviceData_t *deviceData);
uint8_t set_led_values(modbus_t *ctx, uint8_t pool,uint16_t valueNums,uint8_t *valueDatas);
uint8_t set_led_valueByAddr(modbus_t *ctx,uint16_t led_value_address,float *data,uint16_t dataLen);
void addNewDate(deviceData_t *deviceData, uint16_t *tab_rp_registers);
void addNewCommonDate(deviceData_t *deviceData, uint16_t *tab_rp_registers);
void addNewProbeDate(deviceData_t *deviceData, uint16_t *tab_rp_registers, datetime_t *currentDate, uint8_t *code, uint8_t pollutionIndex);

#endif