/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "hardware/flash.h"

#include "uart_rx.pio.h"

#include "config.h"
#include "header/device.h"
#include "header/modbus.h"
#include "header/J212.h"
#include "header/modbusRTU.h"
#include "header/flash.h"
#include "header/common/handler.h"

#ifdef UART_SUQIAN_RECEIVING

/// \tag::multicore_dispatch[]

static modbus_t *ctx;
// deviceData_t *deviceData = NULL;
deviceDatas_t *allDeviceDatas = NULL;

const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
uint sm = 0;
#define uart_rx_program_charLen 256
uint8_t uart_rx_program_char[uart_rx_program_charLen];
uint8_t uart_rx_program_char_copy[uart_rx_program_charLen];
uint16_t uart_rx_program_charIndex = 0;
uint16_t uart_rx_program_char_copyIndex = 0;

uint16_t saveTimes  = 0;
uint8_t *flashData;

float historyData[256];

uint16_t poolNums = 0;
uint16_t pollutionNums;

volatile uint16_t volatile_uart0_recv_len;

uint8_t uart0_recv[1024];
uint16_t uart0_recv_len;


// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    int MN_index, time_index, pollution_index, poolNum_index, pollutionNums_index;
    uint8_t MN[25];
    uint8_t timeString[21];
    uint8_t poolNumString[3];
    uint8_t pollutionNumsString[3];
    uint8_t MN_len;
    uint8_t recvChar;
    uint16_t pollutionCodes[12] = {
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    double pollutionValues[12] = {
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    uint8_t keyStart = 0;
    uint8_t keyEnd = 0;
    uint8_t valueStart = 0;
    uint8_t valueEnd = 0;
    uint8_t startIndex = 0;
    int i, j, k;
    uint8_t pollutionIndex = 0;
    while (1)
    {
        sleep_ms(1000);
        if(volatile_uart0_recv_len){
            if (uart0_recv_len != volatile_uart0_recv_len)
            {
                uart0_recv_len = volatile_uart0_recv_len;
                continue;
            }
            else
            {
                uart0_recv[uart0_recv_len] = '\0';
                if (indexOf("\"deviceID\":\"\"", uart0_recv) != -1)
                {
                    gpio_put(UART0_EN_PIN, 1);
                    sleep_us(50);
                    uart_puts(uart0, "{\"deviceID\":\"" _STRINGIFY(deviceID) "\"}");
                    sleep_us(10152 * 1000 / BAUD_RATE2 * (35 + 1));
                    gpio_put(UART0_EN_PIN, 0);
                }
                else if (indexOf("\"toID\":\"" _STRINGIFY(deviceID) "\"", uart0_recv) != -1)
                {
                    MN_index = indexOf("\"MN\":\"", uart0_recv);
                    if (MN_index != -1)
                    {
                        for (i = 0; i < 25; i++)
                        {
                            recvChar = uart0_recv[i + MN_index + sizeof("\"MN\":\"") - 1];
                            if (recvChar == '"')
                            {
                                MN[i] = '\0';
                                break;
                            }
                            MN[i] = recvChar;
                        }
                        MN_len = strlen(MN);
                    }
                    time_index = indexOf("\"time\":\"", uart0_recv);
                    if (time_index != -1)
                    {
                        for (i = 0; i < 21; i++)
                        {
                            recvChar = uart0_recv[i + time_index + sizeof("\"time\":\"") - 1];
                            if (recvChar == '"')
                            {
                                timeString[i] = '\0';
                                break;
                            }
                            timeString[i] = recvChar;
                        }
                    }
                    poolNum_index = indexOf("\"poolNum\":", uart0_recv);
                    if (poolNum_index != -1)
                    {
                        for (i = 0; i < 3; i++)
                        {
                            recvChar = uart0_recv[i + poolNum_index + sizeof("\"poolNum\":") - 1];
                            if (recvChar < '0' || recvChar > '9')
                            {
                                poolNumString[i] = '\0';
                                break;
                            }
                            poolNumString[i] = recvChar;
                        }
                    }
                    pollutionNums_index = indexOf("\"pollutionNums\":", uart0_recv);
                    if (pollutionNums_index != -1)
                    {
                        for (i = 0; i < 3; i++)
                        {
                            recvChar = uart0_recv[i + pollutionNums_index + sizeof("\"pollutionNums\":") - 1];
                            if (recvChar < '0' || recvChar > '9')
                            {
                                pollutionNumsString[i] = '\0';
                                break;
                            }
                            pollutionNumsString[i] = recvChar;
                        }
                    }

                    pollution_index = indexOf("\"w", uart0_recv);
                    if (pollution_index != -1)
                    {
                        if (toNumber(pollutionNumsString, 0, strlen(pollutionNumsString)) < 12)
                        {
                            pollutionIndex = 0;
                            keyStart = 1;
                            keyEnd = 0;
                            valueStart = 0;
                            valueEnd = 0;
                            startIndex = pollution_index + sizeof("\"w") - 2;
                            for (i = pollution_index + sizeof("\"w") - 2; i < pollution_index + sizeof("\"w") - 2 + 300; i++)
                            {
                                recvChar = uart0_recv[i];
                                if (recvChar == '}')
                                {
                                    if(valueStart){
                                        pollutionValues[pollutionIndex] = toFloat(uart0_recv, startIndex, i);
                                    }
                                    break;
                                }
                                if (keyStart)
                                {
                                    if (recvChar == '"')
                                    {
                                        keyStart = 0;
                                        keyEnd = 1;
                                        pollutionCodes[pollutionIndex] = toNumber(uart0_recv, startIndex + 1, i);
                                    }
                                }
                                else if (keyEnd)
                                {
                                    if (recvChar == ':'){
                                        keyEnd = 0;
                                        valueStart = 1;
                                        startIndex = i + 1;
                                    }
                                }
                                else if (valueStart)
                                {
                                    if (recvChar == ',')
                                    {
                                        valueStart = 0;
                                        valueEnd = 1;
                                        pollutionValues[pollutionIndex] = toFloat(uart0_recv, startIndex, i);
                                    }
                                }
                                else if (valueEnd)
                                {
                                    if (recvChar == '"')
                                    {
                                        pollutionIndex++;
                                        valueEnd = 0;
                                        keyStart = 1;
                                        startIndex = i + 1;
                                    }
                                }
                            }
                        }
                    }
                    
                    for (i = 0; i < allPoolNums; i++)
                    {
                        if (!allDeviceDatas[i].MN_len)
                        {
                            if (MN_index != -1)
                            {
                                MN_len = strlen(MN);
                                allDeviceDatas[i].MN_len = MN_len;
                                allDeviceDatas[i].MN = malloc(MN_len + 1);
                                memset(allDeviceDatas[i].MN, 0, (MN_len + 1) * sizeof(uint8_t));
                                memcpy(allDeviceDatas[i].MN, MN, MN_len + 1);
                                
                                allDeviceDatas[i].PW = malloc(PW_LENGTH);
	                            memset(allDeviceDatas[i].PW, 0, PW_LENGTH * sizeof(uint8_t));
                                allDeviceDatas[i].PW= "123456";

                                if (pollutionNums_index != -1)
                                {
                                    allDeviceDatas[i].pollutionNums = toNumber(pollutionNumsString, 0, strlen(pollutionNumsString));
                                    if (allDeviceDatas[i].pollutionNums)
                                    {
                                        allDeviceDatas[i].pollutions = (pollutions_t *)malloc((allDeviceDatas[i].pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                                        memset(allDeviceDatas[i].pollutions, 0, (allDeviceDatas[i].pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                                    }
                                }
                                if (poolNum_index != -1)
                                {
                                    allDeviceDatas[i].poolNum = toNumber(poolNumString, 0, strlen(poolNumString));
                                }
                                if (time_index != -1)
                                {
                                    allDeviceDatas[i].year = toNumber(timeString, 0, 4) - 2000;
                                    allDeviceDatas[i].month = toNumber(timeString, 5, 7);
                                    allDeviceDatas[i].date = toNumber(timeString, 8, 10);
                                    allDeviceDatas[i].hour = toNumber(timeString, 11, 13);
                                    allDeviceDatas[i].minute = toNumber(timeString, 14, 16);
                                    allDeviceDatas[i].second = toNumber(timeString, 17, 19);
                                }
                                goto saveData;
                            }
                        }
                        else
                        {
                            if (MN_index != -1)
                            {
                                if (strcmp(allDeviceDatas[i].MN, MN) == 0)
                                {
                                    if (time_index != -1)
                                    {
                                        if (allDeviceDatas[i].year != toNumber(timeString, 0, 4) - 2000 ||
                                            allDeviceDatas[i].month != toNumber(timeString, 5, 7) ||
                                            allDeviceDatas[i].date != toNumber(timeString, 8, 10) ||
                                            allDeviceDatas[i].hour != toNumber(timeString, 11, 13) ||
                                            allDeviceDatas[i].minute != toNumber(timeString, 14, 16) ||
                                            allDeviceDatas[i].second != toNumber(timeString, 17, 19))
                                        {
                                            allDeviceDatas[i].year = toNumber(timeString, 0, 4) - 2000;
                                            allDeviceDatas[i].month = toNumber(timeString, 5, 7);
                                            allDeviceDatas[i].date = toNumber(timeString, 8, 10);
                                            allDeviceDatas[i].hour = toNumber(timeString, 11, 13);
                                            allDeviceDatas[i].minute = toNumber(timeString, 14, 16);
                                            allDeviceDatas[i].second = toNumber(timeString, 17, 19);
                                            if (pollutionNums_index != -1)
                                            {
                                                if (allDeviceDatas[i].pollutionNums != toNumber(pollutionNumsString, 0, strlen(pollutionNumsString)))
                                                {
                                                    allDeviceDatas[i].pollutionNums = toNumber(pollutionNumsString, 0, strlen(pollutionNumsString));
                                                    free(allDeviceDatas[i].pollutions);
                                                    allDeviceDatas[i].pollutions = NULL;
                                                    allDeviceDatas[i].pollutions->dataIndex = 0;
                                                    allDeviceDatas[i].pollutions = (pollutions_t *)malloc((allDeviceDatas[i].pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                                                    memset(allDeviceDatas[i].pollutions, 0, (allDeviceDatas[i].pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                                                }
                                                saveData:
                                                if (pollution_index != -1)
                                                {
                                                    for (j = 0; j < allDeviceDatas[i].pollutionNums; j++)
                                                    {
                                                        allDeviceDatas[i].pollutions[j].code = pollutionCode(pollutionCodes[j]);
                                                        allDeviceDatas[i].pollutions[j].data[0] = pollutionValues[j];
                                                        allDeviceDatas[i].pollutions[j].dataIndex = 1;
                                                        allDeviceDatas[i].pollutions[j].state = 'N';
                                                    }
                                                }
                                                flashData[0] = flashKeyFirst;
                                                flashData[1] = flashKeySecond;
                                                flashData[2] = flashKeyThird;
                                                
                                                flashData[2 * poolNumsAddr + configAddr] = 0;
                                                flashData[2 * poolNumsAddr + configAddr + 1] = allPoolNums;
                                                uint16_t shiftIndex = 0;
                                                for (j = 0; j < allPoolNums; j++)
                                                {
                                                    if (allDeviceDatas[j].MN_len)
                                                    {
                                                        flashData[2 * poolNumAddr + configAddr + shiftIndex] = 0;
                                                        flashData[2 * poolNumAddr + configAddr + shiftIndex + 1] = allDeviceDatas[j].poolNum & 0xff;
                                                        flashData[2 * pollutionNumsAddr + configAddr + shiftIndex] = allDeviceDatas[j].pollutionNums >> 8;
                                                        flashData[2 * pollutionNumsAddr + configAddr + shiftIndex + 1] = allDeviceDatas[j].pollutionNums & 0xff;
                                                        flashData[2 * MN_lenAddr + configAddr + shiftIndex] = 0;
                                                        flashData[2 * MN_lenAddr + configAddr + shiftIndex + 1] = allDeviceDatas[j].MN_len & 0xff;
                                                        strcpy(flashData + 2 * MNAddr + configAddr + shiftIndex, allDeviceDatas[j].MN);

                                                        flashData[2 * YearMonthAddr + configAddr + shiftIndex] = intToHex(allDeviceDatas[j].year);
                                                        flashData[2 * YearMonthAddr + configAddr + shiftIndex + 1] = intToHex(allDeviceDatas[j].month);
                                                        flashData[2 * DateHourAddr + configAddr + shiftIndex] = intToHex(allDeviceDatas[j].date);
                                                        flashData[2 * DateHourAddr + configAddr + shiftIndex + 1] = intToHex(allDeviceDatas[j].hour);
                                                        flashData[2 * MinuteSecondAddr + configAddr + shiftIndex] = intToHex(allDeviceDatas[j].minute);
                                                        flashData[2 * MinuteSecondAddr + configAddr + shiftIndex + 1] = intToHex(allDeviceDatas[j].second);

                                                        uint16_t _pollutionCode;
                                                        for (k = 0; k < allDeviceDatas[j].pollutionNums; k++)
                                                        {
                                                            _pollutionCode = toNumber(allDeviceDatas[j].pollutions[k].code, 1, strlen(allDeviceDatas[j].pollutions[0].code));
                                                            flashData[2 * pollutionCodeAddr + k * PollutionDataLen * 2 + configAddr + shiftIndex] = _pollutionCode >> 8;
                                                            flashData[2 * pollutionCodeAddr + k * PollutionDataLen * 2 + configAddr + shiftIndex + 1] = _pollutionCode & 0xff;
                                                            AppendfloatToU8Array(allDeviceDatas[j].pollutions[k].data[0], flashData, 2 * pollutionDataAddr + k * PollutionDataLen * 2 + configAddr + shiftIndex);
                                                            flashData[2 * pollutionStateAddr + k * PollutionDataLen * 2 + configAddr + shiftIndex] = 0;
                                                            flashData[2 * pollutionStateAddr + k * PollutionDataLen * 2 + configAddr + shiftIndex + 1] = 'N' & 0xff;
                                                        }
                                                        shiftIndex += (allDeviceDatas[j].pollutionNums * PollutionDataLen + (pollutionCodeAddr - poolNumAddr)) * 2;
                                                    }
                                                    else
                                                    {
                                                        flashData[2 * MN_lenAddr + configAddr + shiftIndex] = 0;
                                                        flashData[2 * MN_lenAddr + configAddr + shiftIndex + 1] = 0;
                                                        shiftIndex += (pollutionCodeAddr - poolNumAddr) * 2;
                                                    }
                                                }
                                                flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
                                                flash_range_program(FLASH_TARGET_OFFSET, flashData, FLASH_PAGE_SIZE);
                                                // print_buf(flashData, 255);
                                                // printf("\r\n");
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }

                    printf("time:%s\r\n", timeString);
                    printf("pollutionNums:%s\r\n", pollutionNumsString);
                    printf("poolNum:%s\r\n", poolNumString);
                    
                    // allDeviceDatas[poolNum - 1].pollutions = (pollutions_t *)malloc((deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                    for (i = 0; i < allPoolNums; i++){
                        if (allDeviceDatas[i].MN_len)
                        {
                            printf("allDeviceDatas[%d] poolNum:%d pollutionNums:%d MN_len:%d MN:%s PW:%s DataTime:%d-%d-%d %d:%d:%d\r\n",
                                   i, allDeviceDatas[i].poolNum, allDeviceDatas[i].pollutionNums, allDeviceDatas[i].MN_len, allDeviceDatas[i].MN, allDeviceDatas[i].PW,
                                   allDeviceDatas[i].year + 2000, allDeviceDatas[i].month, allDeviceDatas[i].date, allDeviceDatas[i].hour, allDeviceDatas[i].minute, allDeviceDatas[i].second);

                            for (j = 0; j < allDeviceDatas[i].pollutionNums + remainingPollutionNums; j++)
                            {
                                printf("pollution %d: code=%s,data=%f,state=%c\r\n", j + 1, allDeviceDatas[i].pollutions[j].code, allDeviceDatas[i].pollutions[j].data[0], allDeviceDatas[i].pollutions[j].state);
                            }
                        }
                    }
                }
                else
                {
                    printf("\"toID\":\"" _STRINGIFY(deviceID) "\"");
                }
                printf("uart0_recv:%s\r\n", uart0_recv);
                volatile_uart0_recv_len = 0;
                uart0_recv_len = 0;
            }
        }
    }
}

// uart0 RX interrupt handler
void on_uart0_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t ch = uart_getc(uart0);
        if (!ch)
        {
            continue;
        }
        uart0_recv[volatile_uart0_recv_len++] = ch;
        if (volatile_uart0_recv_len == 1024)
        {
            volatile_uart0_recv_len = 0;
        }
    }
}
// uart1 RX interrupt handler
void on_uart1_rx() {
    while (uart_is_readable(uart1)) {
        uint8_t ch = uart_getc(uart1);
        // Can we send it back?
        // printf("(%.2X)", ch);
        modbus_add_RXData(ctx,ch);
        if (uart_is_writable(uart1)) {
            // Change it slightly first!
            // ch++;
            // uart_putc(uart0, ch);
        }
        // chars_rxed++;
    }
}

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    int res;

    stdio_init_all();    

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    const uint uart0_EN = UART0_EN_PIN;
    const uint uart1_EN = UART1_EN_PIN;
    gpio_init(uart0_EN);
    gpio_init(uart1_EN);
    gpio_set_dir(uart0_EN, GPIO_OUT);
    gpio_set_dir(uart1_EN, GPIO_OUT);
    gpio_put(uart0_EN, 0);
    gpio_put(uart1_EN, 0);

    uart_init(uart0, BAUD_RATE2);
    uart_init(uart1, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart0, false, false);
    uart_set_hw_flow(uart1, false, false);

    // Set our data format
    uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);
    uart_set_format(uart1, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(uart0, false);
    uart_set_fifo_enabled(uart1, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART0_IRQ, on_uart0_rx);
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART0_IRQ, true);
    irq_set_enabled(UART1_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart0, true, false);
    uart_set_irq_enables(uart1, true, false);

    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(PH_ADC_PIN);

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(TUR_ADC_PIN);


    // Set up the state machine we're going to use to receive them.

    flashData = (uint8_t *)malloc(FLASH_PAGE_SIZE * sizeof(uint8_t));
    
    /*

    multicore_fifo_push_blocking((uintptr_t)&factorial);
    multicore_fifo_push_blocking(TEST_NUM);

    // // We could now do a load of stuff on core 0 and get our result later

    res = multicore_fifo_pop_blocking();

    printf("Factorial %d is %d\n", TEST_NUM, res);

    // Now try a different function
    multicore_fifo_push_blocking((uintptr_t)&fibonacci);
    multicore_fifo_push_blocking(TEST_NUM);

    res = multicore_fifo_pop_blocking();

    printf("Fibonacci %d is %d\n", TEST_NUM, res);
*/

    ctx = modbus_new_rtu("/dev/ttyUSB0", BAUD_RATE, 'N', 8, 1,uart1,UART1_EN_PIN,0);
    // modbus_rtu_t *ctx_rtu = (modbus_rtu_t *)ctx->backend_data;
    modbus_mapping_t *mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
    modbus_rtu_t  *ctx_rtu = (modbus_rtu_t *)ctx->backend_data;
    uint8_t *query;
    query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
    if (mb_mapping == NULL) {
        printf("err:Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    mb_mapping->tab_bits[0]=2;
    // modbus_set_debug(ctx, TRUE);
    modbus_connect(ctx);
    modbus_set_slave(ctx, 1);

    int val = 0;
    const float conversion_factor = 3.3f / (1 << 12);
    // deviceData = new_deviceData(1, 2, 3);
    // uint8_t changeFlag = false;
    uint16_t dataLen =2 ;
    float data[dataLen];
    size_t i,j;
    for (i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        flashData[i] = flash_target_contents[i];
    }
    uint16_t *tab_rp_registers = NULL;

    if (flash_target_contents[0] == flashKeyFirst && flash_target_contents[1] == flashKeySecond && flash_target_contents[2] == flashKeyThird)
    {
        sleep_ms(100);
        sleep_ms(4000);
        printf("init \r\n");
        poolNums = (flash_target_contents[2 * poolNumsAddr + configAddr] << 8) + flash_target_contents[2 * poolNumsAddr + configAddr + 1];
        if (!poolNums)
        {
            poolNums = allPoolNums;
        }

        if (allDeviceDatas == NULL)
        {
            allDeviceDatas = (deviceDatas_t *)malloc(poolNums * sizeof(deviceDatas_t));
            for (i = 0; i < poolNums; i++)
            {
                allDeviceDatas[i].MN_len = 0;
            }
        }

        uint16_t shiftIndex = 0;
        for (i = 0; i < poolNums; i++)
        {
            allDeviceDatas[i].MN_len = (flash_target_contents[2 * MN_lenAddr + configAddr + shiftIndex] << 8) + flash_target_contents[2 * MN_lenAddr + configAddr + shiftIndex + 1];
            if (allDeviceDatas[i].MN_len)
            {
                allDeviceDatas[i].poolNum = (flash_target_contents[2 * poolNumAddr + configAddr + shiftIndex] << 8) +
                                            flash_target_contents[2 * poolNumAddr + configAddr + shiftIndex + 1];
                allDeviceDatas[i].pollutionNums = (flash_target_contents[2 * pollutionNumsAddr + configAddr + shiftIndex] << 8) +
                                                  flash_target_contents[2 * pollutionNumsAddr + configAddr + shiftIndex + 1];
                allDeviceDatas[i].MN = malloc(allDeviceDatas[i].MN_len + 1);
                memset(allDeviceDatas[i].MN, 0, (allDeviceDatas[i].MN_len + 1) * sizeof(uint8_t));
                memcpy(allDeviceDatas[i].MN, flash_target_contents + 2 * MNAddr + configAddr + shiftIndex, allDeviceDatas[i].MN_len + 1);
                allDeviceDatas[i].MN[allDeviceDatas[i].MN_len] = '\0';
                allDeviceDatas[i].PW = malloc(PW_LENGTH);
                memset(allDeviceDatas[i].PW, 0, PW_LENGTH * sizeof(uint8_t));
                allDeviceDatas[i].PW = "123456";
                allDeviceDatas[i].year = hexToInt(flash_target_contents[2 * YearMonthAddr + configAddr + shiftIndex]);
                allDeviceDatas[i].month = hexToInt(flash_target_contents[2 * YearMonthAddr + configAddr + shiftIndex + 1]);
                allDeviceDatas[i].date = hexToInt(flash_target_contents[2 * DateHourAddr + configAddr + shiftIndex]);
                allDeviceDatas[i].hour = hexToInt(flash_target_contents[2 * DateHourAddr + configAddr + shiftIndex + 1]);
                allDeviceDatas[i].minute = hexToInt(flash_target_contents[2 * MinuteSecondAddr + configAddr + shiftIndex]);
                allDeviceDatas[i].second = hexToInt(flash_target_contents[2 * MinuteSecondAddr + configAddr + shiftIndex + 1]);
                printf("allDeviceDatas[%d] poolNum:%d pollutionNums:%d MN_len:%d MN:%s PW:%s DataTime:%d-%d-%d %d:%d:%d\r\n",
                       i, allDeviceDatas[i].poolNum, allDeviceDatas[i].pollutionNums, allDeviceDatas[i].MN_len, allDeviceDatas[i].MN, allDeviceDatas[i].PW,
                       allDeviceDatas[i].year + 2000, allDeviceDatas[i].month, allDeviceDatas[i].date, allDeviceDatas[i].hour, allDeviceDatas[i].minute, allDeviceDatas[i].second);

                allDeviceDatas[i].pollutions = (pollutions_t *)malloc((allDeviceDatas[i].pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));

                uint16_t _pollutionCode;
                for (j = 0; j < allDeviceDatas[i].pollutionNums; j++)
                {
                    allDeviceDatas[i].pollutions[j].code = pollutionCode((flash_target_contents[2 * pollutionCodeAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex] << 8) +
                                                                         flash_target_contents[2 * pollutionCodeAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 1]);
                    allDeviceDatas[i].pollutions[j].data[0] = bytesToFloat(
                        flash_target_contents[flash_target_contents, 2 * pollutionDataAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 3],
                        flash_target_contents[flash_target_contents, 2 * pollutionDataAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 2],
                        flash_target_contents[flash_target_contents, 2 * pollutionDataAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 1],
                        flash_target_contents[flash_target_contents, 2 * pollutionDataAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 0]);

                    allDeviceDatas[i].pollutions[j].state = (flash_target_contents[2 * pollutionStateAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex] << 8) +
                                                            flash_target_contents[2 * pollutionStateAddr + j * PollutionDataLen * 2 + configAddr + shiftIndex + 1];
                    printf("pollution %d: code=%s,data=%f,state=%c\r\n", j + 1, allDeviceDatas[i].pollutions[j].code, allDeviceDatas[i].pollutions[j].data[0], allDeviceDatas[i].pollutions[j].state);
                }
                shiftIndex += (allDeviceDatas[i].pollutionNums * PollutionDataLen + (pollutionCodeAddr - poolNumAddr)) * 2;
            }
        }
    }

    if (allDeviceDatas == NULL)
    {
        allDeviceDatas = (deviceDatas_t *)malloc(allPoolNums * sizeof(deviceDatas_t));
        for (i = 0; i < allPoolNums; i++)
        {
            allDeviceDatas[i].MN_len = 0;
        }
    }
    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it
    multicore_launch_core1(core1_entry);
    
    uint8_t times = 0;
    uint8_t currentPool = 0;

    while (true)
    {
        // printf("uart_suqian_receiving\r\n");
        // print_buf(flashData,40);
        // printf("\r\n");
        // print_buf(flashData+16,40);
        
        // printf("\r\n");
        #ifdef usingLEDScreen
            times++;
            if(times>4){
                times = 0;
                if(allPoolNums){
                    if (allDeviceDatas[currentPool].MN_len)
                    {
                        modbus_set_slave(ctx, ledAddr);

                        int rc = 0;
                        rc = modbus_write_register(ctx, LED_POOL_ADDRESS, allDeviceDatas[currentPool].poolNum - 1);
                        uint16_t *values;
                        values = (uint16_t *)malloc(2 * allDeviceDatas[currentPool].pollutionNums * sizeof(uint16_t));
                        for (i = 0; i < allDeviceDatas[currentPool].pollutionNums; i++)
                        {
                            AppendfloatToU16Array(allDeviceDatas[currentPool].pollutions[i].data[0], values, i * 2);
                        }
                        rc = modbus_write_registers(ctx, LED_VALUE_ADDRESS, setLedValueNums * 2, values);
                        free(values);
                        values = NULL;
                    }
                    currentPool++;
                    if(currentPool>=allPoolNums){
                        currentPool = 0;
                    }
                }
            }
        #endif

        // printf("Raw value: 0x%03x, voltage: %f V\n", result, adc_PH * conversion_factor);
        sleep_ms(1000);
        val = !val;
        gpio_put(LED_PIN, val);
    }
    return 0;
}
#endif