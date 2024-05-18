/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "hardware/flash.h"
#include "hardware/rtc.h"

#include "uart_rx.pio.h"

#include "config.h"
#include "header/device.h"
#include "header/modbus.h"
#include "header/J212.h"
#include "header/modbusRTU.h"
#include "header/flash.h"
#include "header/gps.h"
#include "header/common/handler.h"

#ifdef UART_DRONE

/// \tag::multicore_dispatch[]

static modbus_t *ctx;
deviceData_t *deviceData = NULL;
deviceDatas_t *allDeviceDatas = NULL;

const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
PIO pio = pio0;
uint sm = 0;
gpsData_t *gps_data;
#define uart_rx_program_charLen 256
uint8_t uart_rx_program_char[uart_rx_program_charLen];
uint8_t uart_rx_program_char_copy[uart_rx_program_charLen];
uint16_t uart_rx_program_charIndex = 0;
uint16_t uart_rx_program_char_copyIndex = 0;

uint16_t saveTimes  = 0;
uint8_t *flashData;

float historyData[256];

uint16_t poolNums = 0;
uint16_t poolNum = 0;
uint16_t pollutionNums;

volatile uint8_t needUpdateServer = 0;
volatile uint8_t detect_3_3V;
volatile uint8_t lastDetect_3_3V;
datetime_t currentDate = {
    .year = 2024,
    .month = 3,
    .day = 6,
    .dotw = 4, // 0 is Sunday, so 3 is Wednesday
    .hour = 9,
    .min = 31,
    .sec = 00};

static void repeat_task_callback(void)
{
    rtc_get_datetime(&currentDate);

    if (currentDate.sec % 30 == 0)
    {
        if (deviceData != NULL)
        {
            needUpdateServer = 1;
        }
    }
}

// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    size_t i;
    size_t j;
    char c;
    uint32_t now;
    uint32_t during = 10 * 1000 * 1000;
    while (1)
    {
        // sleep_ms(500);
        now = time_us_32();
        while (0)
        {
            if (time_us_32() - now > during)
            {
                break;
            }
            continue;
            c = uart_rx_program_getc(pio, sm, 1 * 1000 * 1000);
            if (c != '\0')
            {
                if (c != '\n')
                {
                    if (uart_rx_program_charIndex >= uart_rx_program_charLen)
                    {
                        uart_rx_program_charIndex = 0;
                        uart_rx_program_char[uart_rx_program_charIndex + 1] = '\0';
                    }
                    uart_rx_program_char[uart_rx_program_charIndex++] = c;
                }
                else
                {
                    for (size_t i = 0; i < uart_rx_program_charIndex; i++)
                    {
                        uart_rx_program_char_copy[i] = uart_rx_program_char[i];
                    }
                    uart_rx_program_char_copy[uart_rx_program_charIndex] = '\0';
                    uart_rx_program_char_copyIndex = uart_rx_program_charIndex;
                    uart_rx_program_charIndex = 0;
                    uart_rx_program_char[uart_rx_program_charIndex + 1] = '\0';
                    gps_response_type_t gps_response_type = handleGPSString(uart_rx_program_char_copy, uart_rx_program_char_copyIndex, gps_data);
                    rtc_get_datetime(&currentDate);
                    if (gps_data->year > 0)
                    {
                        if (currentDate.year < gps_data->year || currentDate.month < gps_data->month || currentDate.day < gps_data->date ||
                            currentDate.hour * 60 + currentDate.min + 1 < gps_data->hour * 60 + gps_data->minute)
                        {
                            currentDate.year = gps_data->year;
                            currentDate.month = gps_data->month;
                            currentDate.day = gps_data->date;
                            currentDate.hour = gps_data->hour;
                            currentDate.min = gps_data->minute;
                            currentDate.sec = gps_data->second;
                            rtc_set_datetime(&currentDate);
                        }
                    }
                    // if (gps_response_type == GNRMC || gps_response_type == GPRMC)
                    // {
                    //     if (gps_data->year)
                    //     {
                    //         printf("gps DataTime:%d-%d-%d %d:%d:%d\r\n", gps_data->year, gps_data->month, gps_data->date, gps_data->hour, gps_data->minute, gps_data->second);
                    //     }
                    //     // sleep_ms(60 * 1000);
                    // }
                }
            }
        }
        if (needUpdateServer)
        {
            // for (i = 0; i < poolNums; i++)
            // {
            //     uploadDeviceWithData(&allDeviceDatas[i], &currentDate, getMinutesPollutionDataCMD, uart0, UART0_EN_PIN);
            //     for (j = 0; j < pollutionNums + remainingPollutionNums; j++)
            //     {
            //         allDeviceDatas[i].pollutions[j].dataIndex = 0;
            //     }
            //     sleep_ms(10000);
            // }
            rtc_get_datetime(&currentDate);
            deviceData->year = currentDate.year - 2000;
            deviceData->month = currentDate.month;
            deviceData->date = currentDate.day;
            deviceData->hour = currentDate.hour;
            deviceData->minute = currentDate.min;
            deviceData->second = currentDate.sec;
            deviceData->pollutions[0].data = 0.12;
            deviceData->pollutions[1].data = 10.12;
            deviceData->pollutions[2].data = 3.22;
            deviceData->pollutions[3].data = -1.1;
            deviceData->pollutions[4].data = -21.7;
            uploadDevice(deviceData, uart0, UART0_EN_PIN);
            needUpdateServer = 0;
        }
    }
}

// uart0 RX interrupt handler
void on_uart0_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t ch = uart_getc(uart0);
        // Can we send it back?
        // printf("(%.2X)", ch);
        // modbus_add_RXData(ctx,ch);
        if(ch=='1'){
            needUpdateServer = 0;
            printf("hasUpdateServer\r\n");
        }
        if (uart_is_writable(uart0)) {
            // Change it slightly first!
            // ch++;
            // uart_putc(uart0, ch);
        }
        // chars_rxed++;
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

#define askProbe(probe)                                                                                                   \
    needUpdate = ask_probe(ctx, &deviceData, &currentDate, probe##_Code, probe##_Index, probe##_Addr, probe##_ValueAddr); \
    if (needUpdate != newData && deviceData)                                                                              \
    {                                                                                                                     \
        deviceData->pollutions[probe##_Index].state = 0;                                                                  \
    }


int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    const uint power = POWER_PIN;
    int res;

    stdio_init_all();    

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(power);
    gpio_set_dir(power, GPIO_OUT);
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
    adc_gpio_init(DETECT_3_3V_PIN);

    // Set up the state machine we're going to use to receive them.
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_program);
    uart_rx_program_init(pio, sm, offset, PIO_RX_PIN, PIO_BAUD_RATE);

    flashData = (uint8_t *)malloc(FLASH_PAGE_SIZE * sizeof(uint8_t));
    gps_data = (gpsData_t *)malloc(sizeof(gpsData_t));
    gps_data->year = 0;

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

    size_t i;
    for (i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        flashData[i] = flash_target_contents[i];
    }
    uint16_t *tab_rp_registers = NULL;

    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it
    multicore_launch_core1(core1_entry);

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&currentDate);

    // clk_sys is >2000x faster than clk_rtc, so datetime is not updated immediately when rtc_get_datetime() is called.
    // tbe delay is up to 3 RTC clock cycles (which is 64us with the default clock settings)
    sleep_us(64);

    // Alarm once a minute
    datetime_t repeatTask = {
        .year  = -1,
        .month = -1,
        .day   = -1,
        .dotw  = -1,
        .hour  = -1,
        .min   = -1,
        .sec   = 00
    };

    rtc_set_alarm(&repeatTask, &repeat_task_callback);
    sleep_ms(10000);

    while (true)
    {
        printf("uart_drone\r\n");
    
        adc_select_input(DETECT_3_3V);
        lastDetect_3_3V = detect_3_3V;
        detect_3_3V = adc_read() * 3.3f / (1 << 12) * (2 + 15) / 2 > 2.2;
        detect_3_3V = 1;
        if (!detect_3_3V)
        {
            gpio_put(power, 0);
            printf("not detect working signal.\r\n");
            goto led_toggle;
        }
        else
        {
            if (!lastDetect_3_3V)
            {
                sleep_ms(5000);
            }
            gpio_put(power, 1);
        }

        if(deviceData!=NULL&&ctx->debug)
            printf("out %d %d %d\r\n", deviceData->poolNums, deviceData->pollutionNums, deviceData->MN_len);
        response_type_t needUpdate;
        askProbe(PH);
        askProbe(Temp);
        askProbe(O2);
        askProbe(Tur);
        askProbe(Ele);

        if (deviceData)
        {
            printf("deviceData poolNum:%d poolNums:%d pollutionNums:%d MN_len:%d MN:%s PW:%s DataTime:%d-%d-%d %d:%d:%d\r\n", deviceData->poolNum, deviceData->poolNums, deviceData->pollutionNums, deviceData->MN_len, deviceData->MN, deviceData->PW,
                   deviceData->year + 2000, deviceData->month, deviceData->date, deviceData->hour, deviceData->minute, deviceData->second);
            poolNums = deviceData->poolNums;
            poolNum = deviceData->poolNum;
            pollutionNums = deviceData->pollutionNums;
            if (allDeviceDatas == NULL)
            {
                allDeviceDatas = (deviceDatas_t *)malloc(poolNums * sizeof(deviceDatas_t));
                for (i = 0; i < poolNums; i++)
                {
                    allDeviceDatas[i].MN_len = 0;
                }
            }
            if (poolNum > 0)
            {
                if (allDeviceDatas[poolNum - 1].MN_len == 0)
                {
                    allDeviceDatas[poolNum - 1].MN_len = deviceData->MN_len;
                    allDeviceDatas[poolNum - 1].pollutionNums = deviceData->pollutionNums;
                    allDeviceDatas[poolNum - 1].MN = deviceData->MN;
                    allDeviceDatas[poolNum - 1].PW = deviceData->PW;
                    allDeviceDatas[poolNum - 1].poolNum = deviceData->poolNum;
                    allDeviceDatas[poolNum - 1].pollutions = (pollutions_t *)malloc((deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                    memset(allDeviceDatas[poolNum - 1].pollutions, 0, (deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollutions_t));
                    // for (i = 0; i < pollutionNums + remainingPollutionNums; i++)
                    // {
                        // allDeviceDatas[poolNum - 1].pollutions[i].code = deviceData->pollutions[i].code;
                    // }
                    // rtc_get_datetime(&currentDate);
                    // if (currentDate.year > deviceData->year + 2000 || currentDate.month > deviceData->month || currentDate.day > deviceData->date)
                    // {
                    //     continue;
                    // }
                    // else
                    // {
                    //     if (deviceData->hour < currentDate.hour / 4 * 4 || deviceData->hour >= currentDate.hour / 4 * 4 + 4)
                    //     {
                    //         continue;
                    //     }
                    // }
                }
                allDeviceDatas[poolNum - 1].year = deviceData->year;
                allDeviceDatas[poolNum - 1].month = deviceData->month;
                allDeviceDatas[poolNum - 1].date = deviceData->date;
                allDeviceDatas[poolNum - 1].hour = deviceData->hour;
                allDeviceDatas[poolNum - 1].minute = deviceData->minute;
                allDeviceDatas[poolNum - 1].second = deviceData->second;
                for (i = 0; i < pollutionNums + remainingPollutionNums; i++)
                {
                    allDeviceDatas[poolNum - 1].pollutions[i].code = deviceData->pollutions[i].code;
                    allDeviceDatas[poolNum - 1].pollutions[i].state = deviceData->pollutions[i].state;
                    if (deviceData->pollutions[i].state)
                    {
                        allDeviceDatas[poolNum - 1].pollutions[i].data[allDeviceDatas[poolNum - 1].pollutions[i].dataIndex] = deviceData->pollutions[i].data;
                        if (allDeviceDatas[poolNum - 1].pollutions[i].dataIndex == maxValueNums - 1)
                        {
                        }
                        else
                        {
                            allDeviceDatas[poolNum - 1].pollutions[i].dataIndex++;
                        }
                    }
                }
            }
        }
    led_toggle:
        sleep_ms(3000);
        val = !val;
        gpio_put(LED_PIN, val);
    }
    return 0;
}
#endif