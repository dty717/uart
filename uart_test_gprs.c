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
#include "header/gprs.h"
#include "header/common/handler.h"

#ifdef UART_TEST_GPRS

char datetime_buf[256];
char *datetime_str = &datetime_buf[0];

// Start on Friday 5th of June 2020 15:45:00
datetime_t datetimeNow = {
    .year = 2022,
    .month = 06,
    .day = 9,
    .dotw = 4, // 0 is Sunday, so 5 is Friday
    .hour = 16,
    .min = 05,
    .sec = 00};

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

int handleRec_test0(uint8_t *msg)
{
    switch (msg[0])
    {
    case 'a':
        uart_puts(uart1, "get A\r\n");
        break;
    case 'b':
        uart_puts(uart1, "get B\r\n");
        break;
    case 'c':
        uart_puts(uart1, "get C\r\n");
        break;
    default:
        uart_puts(uart1, "get other\r\n");
        break;
    }
    // sleep_ms(100);
    // uart_puts(uart1, "clientChannel-0:");
    // uart_puts(uart1, msg);
    // uart_puts(uart1, "\r\n");
}

static GPRS_t *gprs = &(GPRS_t){
    "", // rec[1024]
    0,  // recLen
    0,  // recIndex
    {
        1,       // state
        2,       // function_state
        3,       // signal_state
        NOState, // GPRS_state
        NOState, // reset_state
        NOState, // connection_mode
        NOState, // ping_state
        NOState, // bring_up_wireless_connection_state
        {0},     // local_IP_address_$first_is_state$[5]//ipv4 first is state
    },
    .client[0] =
        {
            0,                // channel
            0,                // state
            "155.138.195.23", // *addr
            "TCP",            //*networkType
            1211,
            "",             // beatData[20]
            0,              // beatDataLen
            NOState,        // connectState
            NOState,        // sendState
            handleRec_test0 // handleRec
        }};

uint16_t saveTimes = 0;
uint8_t *flashData;

int key1, key2, key3, key4;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint LED1 = LED1_PIN;
const uint LED2 = LED2_PIN;
const uint LED3 = LED3_PIN;
const uint LED4 = LED4_PIN;

void printhelp()
{
    puts("\r\nCommands:");
    puts("e0, ...\t: set test value");
    puts("a\t: show all the index value");
    puts("t\t: test value");
    puts("k\t: keys value");
    puts("d?\t: set debug value");
    puts("f\t: set debug false");
    puts("c\t: change pool");
}

// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    while (1)
    {
        if (gprs->AT_state.local_IP_address_$first_is_state$[0] != NOState)
        {
            for (size_t i = 0; i < clientsNumbers; i++)
            {
                if (gprs->client[i].connectState == connected)
                {
                    gprs->state = Handle_busy;
                    SIM_send(&gprs->client[i], gprs, "Hello World!\r\n", uart0);
                    gprs->state = Handle_idle;
                }
            }
        }
        sleep_ms(1000 * 60);
    }
}

// uart0 RX interrupt handler
void on_uart0_rx()
{
    while (uart_is_readable(uart0))
    {
        uint8_t ch = uart_getc(uart0);
        gprs_add_RXData(gprs, ch);
        if (uart_is_writable(uart1))
        {
            uart_putc(uart1, ch);
        }
        // chars_rxed++;
    }
}

// uart1 RX interrupt handler
void on_uart1_rx()
{
    while (uart_is_readable(uart1))
    {
        uint8_t ch = uart_getc(uart1);
        if (uart_is_writable(uart0))
        {
            uart_putc(uart0, ch);
        }
        // chars_rxed++;
    }
}

bool repeating_timer_callback(struct repeating_timer *t)
{   
    if(gprs->state == Handle_idle){
        gpio_put(LED_PIN, 1);
    }else{
        gpio_put(LED_PIN, 0);
    }
    return true;
}

int main()
{
    int res;
    int rc;
    int header_length;
    size_t i;

    stdio_init_all();

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
    uint8_t getTimes = 0;
    resetATState(gprs);
    struct repeating_timer timer;
    add_repeating_timer_ms(20, repeating_timer_callback, NULL, &timer);

    multicore_launch_core1(core1_entry);

    while (true)
    {
        printf("\r\nuart_test_gprs\r\n");
        printf("%d,%d,%d\r\n", sizeof(GPRS_t), sizeof(AT_STATE), sizeof(Client));
        // printf("%ld,%ld,%ld\r\n",gprs,gprs->AT_state,gprs->client[0]);
        if (gprs->AT_state.local_IP_address_$first_is_state$[0] == NOState)
        {
            gprs->state = NOState;
            uploadInit(gprs, uart0);
            gprs->state = Handle_idle;
            if(gprs->AT_state.local_IP_address_$first_is_state$[0] != NOState){
                sleep_ms(200);
                for (size_t i = 0; i < clientsNumbers; i++)
                {
                    if (gprs->client[i].connectState == connected)
                    {
                    }
                    else
                    {
                        gprs->state = Handle_busy;
                        setUpConnection(gprs->client[i], gprs, uart0);
                        gprs->state = Handle_idle;
                    }
                }
            }
        }
        else
        {
            if(gprs->state = Handle_idle){
                if(AT_Message_HandleRecive(gprs)){
                    if(gprs->state = Handle_idle){
                        gprs_flush(gprs);
                    }
                }
            }
            // if(strcmp("",msg)){
            //     uart_puts(uart1, "free\r\n");
            //     free(msg);
            // }
            for (size_t i = 0; i < clientsNumbers; i++)
            {
                if (gprs->client[i].connectState == connected)
                {
                }
                else
                {
                    gprs->state = Handle_busy;
                    setUpConnection(gprs->client[i], gprs, uart0);
                    gprs->state = Handle_idle;
                }
            }
        }
        sleep_ms(200);
        // printf("%d,%d,%s\r\n",gprs->AT_state->connection_mode,gprs->client[0].state,gprs->client[0].beatData);
        printf("%d %d %d\r\n", gprs->AT_state.state, gprs->AT_state.function_state, gprs->AT_state.signal_state);
        // // printf("%d,%d,%s\r\n",gprs->AT_state->connection_mode,gprs->client[0].state,gprs->client[0].beatData);
        printf("%d %d %d\r\n", gprs->AT_state.state, gprs->AT_state.function_state, gprs->AT_state.signal_state);
        gpio_put(LED1, getTimes & 1);
        gpio_put(LED2, (getTimes >> 1) & 1);
        gpio_put(LED3, (getTimes >> 2) & 1);
        gpio_put(LED4, (getTimes >> 3) & 1);
        getTimes++;
    }
    return 0;
}

#endif