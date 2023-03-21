/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
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


#ifdef UART_TEST

#define MODBUS_RTU_MAX_ADU_LENGTH 4096


char datetime_buf[256];
char *datetime_str = &datetime_buf[0];

// Start on Friday 5th of June 2020 15:45:00
datetime_t datetimeNow = {
    .year = 2022,
    .month = 07,
    .day = 20,
    .dotw = 3, // 0 is Sunday, so 5 is Friday
    .hour = 16,
    .min = 20,
    .sec = 00
};

/// \tag::multicore_dispatch[]

static modbus_t *ctx;

static modbus_t *ctxServer;
uint8_t *queryServer;

deviceData_t *deviceData = NULL;

const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
PIO pio = pio0;
uint sm = 0;
gpsData_t *gpsData;
#define uart_rx_program_charLen 256
uint8_t uart_rx_program_char[uart_rx_program_charLen];
uint8_t uart_rx_program_char_copy[uart_rx_program_charLen];
uint16_t uart_rx_program_charIndex = 0;
uint16_t uart_rx_program_char_copyIndex = 0;
modbus_mapping_t *mb_mapping;

uint16_t saveTimes  = 0;
uint8_t *flashData;

int key1, key2, key3, key4;

void printhelp() {
    puts("\r\nCommands:");
    puts("e0, ...\t: set test value");
    puts("a\t: show all the index value");
    puts("t\t: test value");
    puts("k\t: keys value");
    puts("d?\t: set debug value");
    puts("f\t: set debug false");
    puts("c\t: change pool");
    
}

volatile uint8_t testVal  = 19;

// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    uint8_t poolNum;
    char c;
    size_t i;

    while (1)
    {
        c = getchar_unlocked();
        // printf("get commond:%c\r\n", c);
        switch (c) {
            case 'e':
                c = getchar_unlocked();
                testVal = c -'0';
                printf("set test value:%d\r\n", testVal);
                break;
            case 'a': {
                modbus_rtu_t *ctx_rtu = ctxServer->backend_data;
                printf("chars_rx_index:%d,chars_rx_server_index:%d,chars_rxed:%d\r\n",ctx_rtu->chars_rx_index,ctx_rtu->chars_rx_server_index,ctx_rtu->chars_rxed);
                break;
            }
            case 'v': {
                printf("testVal: %d\r\n", testVal);
                break;
            }
            case 'k':
                printf("key1:%d,key2:%d,key3:%d,key4:%d\r\n", key1,key2,key3,key4);
                break;
            case 'd':
                c = getchar_unlocked();
                modbus_set_debug(ctxServer,c);
                printf("ctxServer debug: %d\r\n", ctxServer->debug);
                break;
            case 'f':
                modbus_set_debug(ctxServer,false);
                printf("ctxServer debug: %d\r\n", ctxServer->debug);
                break;            
            case '\r':
            case '\n':
                break;
            case 'z':
                rtc_get_datetime(&datetimeNow);
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + YearMonthAddr] = (intToHex(datetimeNow.year - 2000) << 8) + intToHex(datetimeNow.month);
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + DateHourAddr] = (intToHex(datetimeNow.day) << 8) + intToHex(datetimeNow.hour);
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MinuteSecondAddr] = (intToHex(rand() % 15 + 10) << 8) + intToHex(rand() % 60);

                uint16_t codes2[] = {1009, 21011, 1010, 21003, 1018, 1014};
                float datas2[] = {2.2, 28.2, 301.8, 3101.1, 5101.1, 3101.9};

                for (i = 0; i < mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionNumsAddr]; i++)
                {
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionCodeAddr + PollutionDataLen * i] = codes2[i];
                    uint8_t *arrayVal;
                    arrayVal = (uint8_t *)malloc(4 * sizeof(uint8_t));
                    floatToByteArray(datas2[i], arrayVal);
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionDataAddr + PollutionDataLen * i] = arrayVal[2] + (arrayVal[3] << 8);
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionDataAddr + PollutionDataLen * i + 1] = arrayVal[0] + (arrayVal[1] << 8);
                    if (i > 2)
                    {
                        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionStateAddr + PollutionDataLen * i] = 0;
                    }
                }
                printf("poolNum:%d\r\n",poolNum);
                break;
            case 'c':

                // poolNum = mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + poolNumAddr];
                // poolNum++;
                // if(poolNum>mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + poolNumsAddr]){
                //     poolNum = 1;
                // }
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + poolNumAddr] = poolNum;
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 1] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 2] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 3] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 4] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 5] = 'A'+poolNum - 1 + ('C' << 8);
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 6] = 'D' + ('0' << 8);

                rtc_get_datetime(&datetimeNow);
                // datetimeNow.year
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + YearMonthAddr] = (intToHex(datetimeNow.year - 2000) << 8) + intToHex(datetimeNow.month);
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + DateHourAddr] = (intToHex(datetimeNow.day) << 8) + intToHex(datetimeNow.hour);
                mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MinuteSecondAddr] = (intToHex(rand() % 15 + 10) << 8) + intToHex(rand() % 60);

                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + YearMonthAddr] = 0x2110;
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + DateHourAddr]++;
                // mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MinuteSecondAddr] = 0x1121;

                uint16_t codes[] = {1009, 21011, 1010, 21003, 1018, 1014};
                float datas[] = {8.2, 78.2, 101.8, 2101.1, 4101.1, 5101.9};

                for (i = 0; i < mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionNumsAddr]; i++)
                {
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionCodeAddr + PollutionDataLen * i] = codes[i];
                    uint8_t *arrayVal;
                    arrayVal = (uint8_t *)malloc(4 * sizeof(uint8_t));
                    floatToByteArray(datas[i], arrayVal);
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionDataAddr + PollutionDataLen * i] = arrayVal[2] + (arrayVal[3] << 8);
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionDataAddr + PollutionDataLen * i + 1] = arrayVal[0] + (arrayVal[1] << 8);
                    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS+pollutionStateAddr + PollutionDataLen * i] = 1;
                }
                printf("poolNum:%d\r\n",poolNum);
                break;
            case 'h':
                printhelp();
                break;
            case 'i':
                mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS+6]+=0x100;
                printf("init\r\n");
                break;
            default:
                printf("\nUnrecognised command: %c\n", c);
                printhelp();
                break;
        }

    }
}

// uart0 RX interrupt handler
void on_uart0_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t ch = uart_getc(uart0);
        // Can we send it back?
        // printf("(%.2X)", ch);
        modbus_add_RXData(ctxServer,ch);
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
        printf("%c", ch);
        modbus_add_RXData(ctx,ch);
        if (uart_is_writable(uart1)) {
            // Change it slightly first!
            // ch++;
            // uart_putc(uart0, ch);
        }
        // chars_rxed++;
    }
}

bool repeating_timer_callback(struct repeating_timer *t)
{

    key1 = gpio_get(KEY1_PIN);
    key2 = gpio_get(KEY2_PIN);
    key3 = gpio_get(KEY3_PIN);
    key4 = gpio_get(KEY4_PIN);
    rtc_get_datetime(&datetimeNow);
    if (datetimeNow.hour % 4 != 0)
    {
        return true;
    }

    // datetimeNow.year
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + YearMonthAddr] = (intToHex(datetimeNow.year - 2000) << 8) + intToHex(datetimeNow.month);
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + DateHourAddr] = (intToHex(datetimeNow.day) << 8) + intToHex(datetimeNow.hour);
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MinuteSecondAddr] = (intToHex(rand() % 15 + 10) << 8) + intToHex(rand() % 60);

    uint16_t codes[] = {1009, 21011, 1010, 21003, 1018, 1014};
    float datas[] = {rand()%10000/1000.0, rand()%10000/1000.0,rand()%10000/1000.0,rand()%10000/1000.0, rand()%10000/1000.0, rand()%10000/1000.0};
    for (int i = 0; i < mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionNumsAddr]; i++)
    {
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionCodeAddr + PollutionDataLen * i] = codes[i];
        uint8_t *arrayVal;
        arrayVal = (uint8_t *)malloc(4 * sizeof(uint8_t));
        floatToByteArray(datas[i], arrayVal);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionDataAddr + PollutionDataLen * i] = arrayVal[2] + (arrayVal[3] << 8);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionDataAddr + PollutionDataLen * i + 1] = arrayVal[0] + (arrayVal[1] << 8);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionStateAddr + PollutionDataLen * i] = 1;
    }

    return true;
}

static char event_str[128];

void gpio_event_string(char *buf, uint32_t events);
void gpio_event_handle(uint gpio, uint32_t events);

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    // gpio_event_handle(gpio, events);

    // printf("GPIO %d %s\n", gpio, event_str);
}

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    const uint LED1 = LED1_PIN;
    const uint LED2 = LED2_PIN;
    const uint LED3 = LED3_PIN;
    const uint LED4 = LED4_PIN;
    int res;
    int rc;
    int header_length;
    size_t i;

    stdio_init_all();    

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&datetimeNow);


    gpio_init(LED_PIN);
    gpio_init(LED1);
    gpio_init(LED2);
    gpio_init(LED3);
    gpio_init(LED4);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_set_dir(LED2, GPIO_OUT);
    gpio_set_dir(LED3, GPIO_OUT);
    gpio_set_dir(LED4, GPIO_OUT);

    const uint uart0_EN = UART0_EN_PIN;
    const uint uart1_EN = UART1_EN_PIN;

    gpio_init(uart0_EN);
    gpio_init(uart1_EN);
    gpio_set_dir(uart0_EN, GPIO_OUT);
    gpio_set_dir(uart1_EN, GPIO_OUT);
    gpio_put(uart0_EN, 0);
    gpio_put(uart1_EN, 0);

    uart_init(uart0, BAUD_RATE);
    uart_init(uart1, BAUD_RATE2);

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

    gpio_set_irq_enabled_with_callback(InputDetect_PIN, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    const uint key1Pin = KEY1_PIN;
    const uint key2Pin = KEY2_PIN;
    const uint key3Pin = KEY3_PIN;
    const uint key4Pin = KEY4_PIN;
    
    gpio_init(key1Pin);
    gpio_init(key2Pin);
    gpio_init(key3Pin);
    gpio_init(key4Pin);

    gpio_set_dir(key1Pin, GPIO_IN);
    gpio_set_dir(key2Pin, GPIO_IN);
    gpio_set_dir(key3Pin, GPIO_IN);
    gpio_set_dir(key4Pin, GPIO_IN);
    key1 = gpio_get(KEY1_PIN);
    key2 = gpio_get(KEY2_PIN);
    key3 = gpio_get(KEY3_PIN);
    key4 = gpio_get(KEY4_PIN);


    // Set up the state machine we're going to use to receive them.
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_program);
    uart_rx_program_init(pio, sm, offset, PIO_RX_PIN, PIO_BAUD_RATE);

    flashData = (uint8_t *)malloc(FLASH_PAGE_SIZE * sizeof(uint8_t));
    gpsData = (gpsData_t *)malloc(sizeof(gpsData_t));
    
    queryServer = malloc(MODBUS_RTU_MAX_ADU_LENGTH);

    ctxServer = modbus_new_rtu("/dev/ttyUSB0", BAUD_RATE, 'N', 8, 1, uart0, UART0_EN_PIN,1);

    header_length = modbus_get_header_length(ctxServer);
    modbus_connect(ctxServer);
    modbus_set_slave(ctxServer, 1);
    // if(key1){
    //     modbus_set_debug(ctxServer, TRUE);
    // }else{
    //     // modbus_set_debug(ctxServer, TRUE);
    // }

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

    ctx = modbus_new_rtu("/dev/ttyUSB1", BAUD_RATE, 'N', 8, 1,uart1,UART1_EN_PIN,0);
    // modbus_rtu_t *ctx_rtu = (modbus_rtu_t *)ctx->backend_data;
    mb_mapping = modbus_mapping_new_start_address(0,MODBUS_MAX_READ_BITS/10, 0,0,
                                    0,0xffff, 0,0);
    
    modbus_rtu_t  *ctxServer_rtu = (modbus_rtu_t *)ctxServer->backend_data;
    uint8_t *query;
    query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
    if (mb_mapping == NULL) {
        printf("err:Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    // modbus_set_debug(ctx, TRUE);
    modbus_connect(ctx);
    modbus_set_slave(ctx, 1);

    int val = 0;
    const float conversion_factor = 3.3f / (1 << 12);

    // deviceData = new_deviceData(1, 2, 3);
    // uint8_t changeFlag = false;
    uint16_t dataLen =2 ;
    float data[dataLen];
    // for (i = 0; i < FLASH_PAGE_SIZE; i+=2)
    // {
    //     flashData[i] = flash_target_contents[i];
    //     flashData[i+1] = flash_target_contents[i+1];
    //     // mb_mapping->tab_registers[i/2] = (flashData[i]<<8)+flashData[i+1];
    //     mb_mapping->tab_registers[i/2] = i;
    //     // (flashData[i]<<8)+flashData[i+1];
    // }
    
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS] = datetimeNow.year;
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS + 1] = intToHex(datetimeNow.month);
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS + 2] = intToHex(datetimeNow.day);
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS + 3] = intToHex(datetimeNow.hour);
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS + 4] = intToHex(datetimeNow.min);
    mb_mapping->tab_registers[PLC_DATE_REGISTERS_ADDRESS + 5] = intToHex(datetimeNow.sec);

    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + poolNumsAddr] = 1;
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + poolNumAddr] = 1;
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionNumsAddr] = 6;
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MN_lenAddr] = 24;

    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr] = ('4' << 8) + '2';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 1] = ('0' << 8) + '2';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 2] = ('2' << 8) + '2';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 3] = ('0' << 8) + '2';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 4] = ('D' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 5] = ('2' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 6] = ('0' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 7] = ('0' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 8] = ('0' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 9] = ('0' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 10] = ('0' << 8) + '0';
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MNAddr + 11] = ('2' << 8) + '0';

    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + YearMonthAddr] = (intToHex(datetimeNow.year - 2000) << 8) + intToHex(datetimeNow.month);
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + DateHourAddr] = (intToHex(datetimeNow.day) << 8) + intToHex((datetimeNow.hour / 4) * 4);
    mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + MinuteSecondAddr] = (intToHex(datetimeNow.min) << 8) + intToHex(datetimeNow.sec);

    uint16_t codes[] = {1009, 21011, 1010, 21003, 1018, 1014};
    float datas[] = {2.3, 22.1, 301.8, 1131, 3401.9, 6201.9};
    for (i = 0; i < mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionNumsAddr]; i++)
    {
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionCodeAddr + PollutionDataLen * i] = codes[i];
        uint8_t *arrayVal;
        arrayVal = (uint8_t *)malloc(4 * sizeof(uint8_t));
        floatToByteArray(datas[i], arrayVal);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionDataAddr + PollutionDataLen * i] = arrayVal[2] + (arrayVal[3] << 8);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionDataAddr + PollutionDataLen * i + 1] = arrayVal[0] + (arrayVal[1] << 8);
        mb_mapping->tab_registers[UT_REGISTERS_ADDRESS + pollutionStateAddr + PollutionDataLen * i] = 1;
    }

    mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS] = 0x3F80;
    mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS + 1] = 0xECCD;
    mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS + 4] = 0x2112;
    mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS + 5] = 0x0813;
    mb_mapping->tab_registers[COMMON_DEVICE_REGISTERS_ADDRESS + 6] = 0x0030;

    struct repeating_timer timer;
    // add_repeating_timer_ms(10 * 10 * 1000, repeating_timer_callback, NULL, &timer);

    datetime_t alarm = {
        .year  = -1,
        .month = -1,
        .day   = -1,
        .dotw  = -1,
        .hour  = -1,
        .min   = 20,
        .sec   = 00
    };
    // rtc_set_alarm(&alarm, &repeating_timer_callback);

    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it
    multicore_launch_core1(core1_entry);
    
    uint8_t getTimes = 0;
    while (true)
    {
        // printf("\r\nuart_test\r\n");
        
        gpio_put(LED1,getTimes&1);
        gpio_put(LED2,(getTimes>>1)&1);
        gpio_put(LED3,(getTimes>>2)&1);
        gpio_put(LED4,(getTimes>>3)&1);
        getTimes++;
        do
        {
            rc = modbus_receive(ctxServer, query);
            if(testVal==5){
                printf("modbus_receive\r\n");
            }
            /* Filtered queries return 0 */
        } while (rc == 0 || rc == -1);

        rc = modbus_reply(ctxServer, query, rc, mb_mapping);

    }
    return 0;
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_handle(uint gpio, uint32_t events) {
    if(gpio==InputDetect_PIN){
        for (uint i = 0; i < 4; i++) {
            uint mask = (1 << i);
            if (events & mask) {
                // Copy this event string into the user string
                if(i==0||i==2){
                    mb_mapping->tab_registers[InputDetectAddr] = 0;
                    flashData[InputDetectAddr*2] = 0;
                    flashData[InputDetectAddr*2+1] = 0;
                }else if(i==1||i==3){
                    mb_mapping->tab_registers[InputDetectAddr] = 1;
                    flashData[InputDetectAddr * 2] = 0;
                    flashData[InputDetectAddr*2+1] = 1;
                }                
                events &= ~mask;
            }
        }
    }
}

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}
#endif