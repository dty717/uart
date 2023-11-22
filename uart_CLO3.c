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
#include "hardware/gpio.h"
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
#include "header/gps.h"
#include "header/common/handler.h"

#ifdef UART_CLO3

/// \tag::multicore_dispatch[]

static modbus_t *ctx;

static modbus_t *ctxServer;
uint8_t *queryServer;

deviceData_t *deviceData = NULL;

const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
PIO pio = pio0;
uint sm = 0;
gpsData_t *gps_data;
#define uart_rx_program_charLen 256
uint8_t uart_rx_program_char[uart_rx_program_charLen];
uint8_t uart_rx_program_char_copy[uart_rx_program_charLen];
uint16_t uart_rx_program_charIndex = 0;
uint16_t uart_rx_program_char_copyIndex = 0;
modbus_mapping_t *mb_mapping;

uint16_t saveTimes  = 0;
uint8_t *flashData;

int key1, key2, key3, key4;

// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    while (1)
    {
        char c = uart_rx_program_getc(pio, sm, 10000);
        // putchar(c);
        if(c!='\n'){
            if(uart_rx_program_charIndex>=uart_rx_program_charLen){
                uart_rx_program_charIndex = 0;
                uart_rx_program_char[uart_rx_program_charIndex+1] = '\0';
            }
            uart_rx_program_char[uart_rx_program_charIndex++] = c;
        }else{
            for (size_t i = 0; i < uart_rx_program_charIndex; i++)
            {
                uart_rx_program_char_copy[i] = uart_rx_program_char[i];
            }
            uart_rx_program_char_copy[uart_rx_program_charIndex] = '\0';
            uart_rx_program_char_copyIndex = uart_rx_program_charIndex;
            uart_rx_program_charIndex = 0;
            uart_rx_program_char[uart_rx_program_charIndex + 1] = '\0';
            gps_response_type_t gps_response_type= handleGPSString(uart_rx_program_char_copy, uart_rx_program_char_copyIndex, gps_data);
            if (gps_response_type == GNRMC||gps_response_type == GPRMC)
            {

                if (gps_data->state == 'A')
                {
                    // printf("gps info:%.4d-%.2d-%.2d %.2d:%.2d:%.2d %f%c,%f%c\r\n", gps_data->year, gps_data->month, gps_data->date, gps_data->hour, gps_data->minute, gps_data->second,
                    //        gps_data->latitude, gps_data->latitudeFlag, gps_data->longitude, gps_data->longitudeFlag);
                    // printf("%d\r\n",flashData[0]);
                    if(saveTimes==0){
                        flashData[3] = '4';
                        flashData[4] = '5';
                        flashData[checkAddr + gps_yearAddr] = gps_data->year-2000;
                        flashData[checkAddr + gps_monthAddr] = gps_data->month;
                        flashData[checkAddr + gps_dateAddr] = gps_data->date;
                        flashData[checkAddr + gps_hourAddr] = gps_data->hour;
                        flashData[checkAddr + gps_minuteAddr] = gps_data->minute;
                        flashData[checkAddr + gps_secondAddr] = gps_data->second;
                        flashData[checkAddr + gps_stateAddr] = gps_data->state;
                        AppendfloatToU8Array(gps_data->latitude, flashData, checkAddr + gps_latitudeAddr);
                        flashData[checkAddr + gps_latitudeFlagAddr] = gps_data->latitudeFlag;
                        AppendfloatToU8Array(gps_data->longitude, flashData, checkAddr + gps_longitudeAddr);
                        flashData[checkAddr + gps_longitudeFlagAddr] = gps_data->longitudeFlag;
                        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
                        flash_range_program(FLASH_TARGET_OFFSET, flashData, FLASH_PAGE_SIZE);
                        for (size_t i = 0; i < InputDetectAddr-1; i += 2)
                        {
                            mb_mapping->tab_registers[i / 2] = (flashData[i] << 8) + flashData[i + 1];
                        }
                    }else{
                        saveTimes++;
                        if(saveTimes==10){
                            saveTimes = 0;
                        }
                    }
                }
                else
                {
                    if(key4)
                        printf("gps state:%c\r\n", gps_data->state);
                }

            }
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

bool repeating_timer_callback(struct repeating_timer *t)
{
    if(key3)
        printf("gpio_get at %d\r\n", gpio_get(InputDetect_PIN));
    
    if (gpio_get(InputDetect_PIN)==1)
    {
        flashData[InputDetectAddr * 2] = 0;
        flashData[InputDetectAddr * 2 + 1] = 1;
        mb_mapping->tab_registers[InputDetectAddr] = 1;
    }
    else
    {
        flashData[InputDetectAddr * 2] = 0;
        flashData[InputDetectAddr * 2 + 1] = 0;
        mb_mapping->tab_registers[InputDetectAddr] = 0;
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
    gpio_event_handle(gpio, events);

    // printf("GPIO %d %s\n", gpio, event_str);
}

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    int res;
    int rc;
    int header_length;
    size_t i;

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

    uart_init(uart0, BAUD_RATE);
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
    gps_data = (gpsData_t *)malloc(sizeof(gpsData_t));
    
    queryServer = malloc(MODBUS_RTU_MAX_ADU_LENGTH);

    ctxServer = modbus_new_rtu("/dev/ttyUSB0", BAUD_RATE, 'N', 8, 1, uart0, UART0_EN_PIN,1);

    header_length = modbus_get_header_length(ctxServer);
    modbus_connect(ctxServer);
    modbus_set_slave(ctxServer, uploadAddr);
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
                                    0,0x100, 0,0);
    
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
    for (i = 0; i < FLASH_PAGE_SIZE; i+=2)
    {
        flashData[i] = flash_target_contents[i];
        flashData[i+1] = flash_target_contents[i+1];
        mb_mapping->tab_registers[i/2] = (flashData[i]<<8)+flashData[i+1];
    }

    if(flash_target_contents[0]=='1'&&flash_target_contents[1]=='2'&&flash_target_contents[2]=='3'){
        uint16_t *tab_rp_registers = NULL;
        tab_rp_registers = (uint16_t *)malloc(nb_points * sizeof(uint16_t));
        for (i = 0; i < nb_points; i++)
        {
            tab_rp_registers[i] = (flash_target_contents[i*2+configAddr]<<8)+flash_target_contents[i*2+configAddr+1];
        }
        uint16_t MN_len =  (flash_target_contents[2*MN_lenAddr+configAddr]<<8) + flash_target_contents[2*MN_lenAddr+configAddr+1];
        deviceData = new_deviceData(tab_rp_registers[poolNumsAddr], tab_rp_registers[pollutionNumsAddr], tab_rp_registers[MN_lenAddr]);
        deviceData->poolNum = tab_rp_registers[poolNumAddr];
        deviceData->PW = "123456";
        addNewDate(deviceData,tab_rp_registers);
    }
    if(flash_target_contents[3]=='4'&&flash_target_contents[4]=='5'){
        gps_data->year = flash_target_contents[checkAddr + gps_yearAddr] + 2000;
        gps_data->month = flash_target_contents[checkAddr + gps_monthAddr];
        gps_data->date = flash_target_contents[checkAddr + gps_dateAddr];
        gps_data->hour = flash_target_contents[checkAddr + gps_hourAddr];
        gps_data->minute = flash_target_contents[checkAddr + gps_minuteAddr];
        gps_data->second = flash_target_contents[checkAddr + gps_secondAddr];
        gps_data->latitude = bytesToFloat(flash_target_contents[checkAddr + gps_latitudeAddr + 3], flash_target_contents[checkAddr + gps_latitudeAddr + 2],
                                         flash_target_contents[checkAddr + gps_latitudeAddr + 1], flash_target_contents[checkAddr + gps_latitudeAddr]);
        gps_data->latitudeFlag = flash_target_contents[checkAddr + gps_latitudeFlagAddr];
        gps_data->longitude = bytesToFloat(flash_target_contents[checkAddr + gps_longitudeAddr + 3], flash_target_contents[checkAddr + gps_longitudeAddr + 2],
                                          flash_target_contents[checkAddr + gps_longitudeAddr + 1], flash_target_contents[checkAddr + gps_longitudeAddr]);
        gps_data->longitudeFlag = flash_target_contents[checkAddr + gps_longitudeFlagAddr];
    }

    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it
    multicore_launch_core1(core1_entry);

    while (true)
    {
        printf("uart_CLO3\r\n");

        // for (size_t j = 0; j < FLASH_PAGE_SIZE; j++)
        // {
        //     printf("[%.2X] ", flash_target_contents[j]);
        // }
        // printf("\r\n");
        // printf("%d ?== %d\r\n",&flashData,&flashData[0]);

        // if(1){
        //     sleep_ms(1000);
        //     gpio_put(LED_PIN, 0);
        //     printf("123\r\n");
        //     sleep_ms(1000);
        //     gpio_put(LED_PIN, 1);
        //     printf("123\r\n");
        //     sleep_ms(1000);
        //     gpio_put(LED_PIN, 0);
        //     printf("123\r\n");
        //     sleep_ms(1000);
        //     gpio_put(LED_PIN, 1);
        //     printf("123\r\n");
        // }
        do
        {
            // ctxServer_rtu->confirmation_to_ignore = 0;
            // modbus_flush(ctxServer);
            rc = modbus_receive(ctxServer, query);
            /* Filtered queries return 0 */
        } while (rc == 0 || rc == -1);
        // if (rc == -1 && errno != EMBBADCRC)
        // {
        //     /* Quit */
        //     break;
        // }
        // if(1){
        //     printf("slave: %d\r\n\r\n", ctxServer->slave);
        //     for (size_t j = 0; j < rc; j++)
        //     {
        //         printf("[%.2X] ", query[j]);
        //     }
        //     printf("\r\n");
        //     printf("**%.2X\r\n", MODBUS_GET_INT16_FROM_INT8(query, header_length + 1));

        //     continue;
        // }
        rc = modbus_reply(ctxServer, query, rc, mb_mapping);
        // if (rc == -1) {
        //     break;
        // }

        // if(deviceData!=NULL&&ctx->debug)
        //     printf("out %d %d %d\r\n", deviceData->poolNums, deviceData->pollutionNums, deviceData->MN_len);

        // adc_select_input(PH_ADC);
        // uint adc_PH = adc_read();
        // adc_select_input(TUR_ADC);
        // uint adc_TUR = adc_read();
        // data[0] = _4_20mvTofloat(adc_PH * conversion_factor / 160 * 1000, 0, 7);
        // data[1] = _4_20mvTofloat(adc_TUR * conversion_factor / 160 * 1000, 0, 100);

        // // printf("Raw value: 0x%03x, voltage: %f V\n", result, adc_PH * conversion_factor);
        // sleep_ms(1000);
        // val = !val;
        // gpio_put(LED_PIN, val);
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