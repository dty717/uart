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

#include "uart_rx.pio.h"

#include "config.h"
#include "header/device.h"
#include "header/modbus.h"
#include "header/J212.h"
#include "header/modbusRTU.h"
#include "header/flash.h"
#include "header/common/handler.h"

#ifdef UART_JIANGNING



/// \tag::multicore_dispatch[]

static modbus_t *ctx;
deviceData_t *deviceData = NULL;

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

uint8_t needUpdateServer = 0;
// Ask core 1 to print a string, to make things easier on core 0
void core1_entry()
{
    size_t i;
    while (1)
    {
        // uploadDevice(deviceData);	
        // Function pointer is passed to us via the FIFO	
        // We have one incoming int32_t as a parameter, and will provide an	
        // int32_t return value by simply pushing it back on the FIFO	
        // which also indicates the result is ready.	
        
        // multicore_fifo_pop_blocking();	
        // sleep_ms(30000);	
        sleep_ms(1000);	
        if(deviceData!=NULL&&needUpdateServer){
            for (i = 0; i < 3; i++)
            {
                uploadDevice(deviceData, uart0, UART0_EN_PIN);
                sleep_ms(1000 * 5);
                uploadDeviceMinutes(deviceData, uart0, UART0_EN_PIN);
                printf("update server\r\n");
                sleep_ms(1000 * 60 * 5);
                // if (!needUpdateServer)
                // {
                //     break;
                // }
                // sleep_ms(1000 * 20);
                // if (!needUpdateServer)
                // {
                //     break;
                // }
            }
            needUpdateServer = 0;
        }
        // int32_t (*func)() = (int32_t(*)())multicore_fifo_pop_blocking();	
        // int32_t p = multicore_fifo_pop_blocking();	
        // int32_t result = (*func)(p);	
        // multicore_fifo_push_blocking(result);	
        // sleep_ms(1250);	
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
    gpio_put(uart0_EN, 1);
    gpio_put(uart1_EN, 1);

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
    size_t i;
    for (i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        flashData[i] = flash_target_contents[i];
    }
    uint16_t *tab_rp_registers = NULL;

    if(flash_target_contents[0]=='1'&&flash_target_contents[1]=='2'&&flash_target_contents[2]=='3'&&0){
        
        tab_rp_registers = (uint16_t *)malloc(nb_points * sizeof(uint16_t));
        for (i = 0; i < nb_points; i++)
        {
            tab_rp_registers[i] = (flash_target_contents[i*2+configAddr]<<8)+flash_target_contents[i*2+configAddr+1];
        }
        poolNums = tab_rp_registers[poolNumsAddr];
        pollutionNums = tab_rp_registers[pollutionNumsAddr];
        deviceData = new_deviceData(poolNums, tab_rp_registers[pollutionNumsAddr], tab_rp_registers[MN_lenAddr]);
        deviceData->poolNum = tab_rp_registers[poolNumAddr];
        deviceData->PW = "123456";
        addNewDate(deviceData,tab_rp_registers);
        
        // uint16_t shiftHistoryAddr = (4*(pollutionNums+remainingPollutionNums)+1)*(deviceData->poolNum-1);
        // historyData = (float *)malloc(nb_points * sizeof(float));

        for (i = 0; i < poolNums; i++)
        {
            // *((pollutionNums+remainingPollutionNums)*4+1)
            uint16_t shiftHistoryAddr = (4*(pollutionNums+remainingPollutionNums)+1)*i;
            if(flashData[HistroySaveAddr+shiftHistoryAddr]==i+1){
                for (size_t j = 0; j < pollutionNums; j++)
                {
                    historyData[i*(pollutionNums+remainingPollutionNums)+j] = flashData[i+j];
                }
            }
            // if(flashData[HistroySaveAddr + shiftHistoryAddr+1+4*i]){
            // }
        }
    }

    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it
    multicore_launch_core1(core1_entry);
    
    uint8_t times = 0;
    uint8_t currentPool = 0;
    // wait for dtu init
    sleep_ms(1000 * 60 * 1);

    while (true)
    {
        printf("uart_jiangning\r\n");
        if(deviceData!=NULL&&ctx->debug)
            printf("out %d %d %d\r\n", deviceData->poolNums, deviceData->pollutionNums, deviceData->MN_len);
        
        response_type_t needUpdate = ask_common_device(ctx,&deviceData);

        switch (needUpdate)
        {
        case noResponse:
            break;
        case normal:
            needUpdateServer = 1;
            break;
        case newData:
            // set_led_value(ctx, deviceData);
            poolNums = deviceData->poolNums;
            pollutionNums = deviceData->pollutionNums;
            needUpdateServer = 1;
            // deviceData->pollutions[deviceData->pollutionNums].code = "w01001";
            // deviceData->pollutions[deviceData->pollutionNums].data = data[0];
            // deviceData->pollutions[deviceData->pollutionNums].state = 1;

            // deviceData->pollutions[deviceData->pollutionNums + 1].code = "w01012";
            // deviceData->pollutions[deviceData->pollutionNums].data = data[1];
            // deviceData->pollutions[deviceData->pollutionNums].state = 1;
            
            // multicore_fifo_push_blocking(123);
            // uploadDevice(deviceData,uart0,uart0_EN);
            break;
        default:
            break;
        }
        // printf("Raw value: 0x%03x, voltage: %f V\n", result, adc_PH * conversion_factor);
        sleep_ms(1000);
        val = !val;
        gpio_put(LED_PIN, val);
    }
    return 0;
}
#endif