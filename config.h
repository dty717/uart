#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

#define TEST_NUM 10

#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define BAUD_RATE2 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART0_EN_PIN 2
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_EN_PIN 3
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

#define KEY1_PIN 6
#define KEY2_PIN 7
#define KEY3_PIN 8
#define KEY4_PIN 9

#define PIO_BAUD_RATE    9600
#define PIO_RX_PIN    20

#define PH_ADC_PIN    26
#define PH_ADC         0
#define TUR_ADC_PIN   28
#define TUR_ADC        2

#define InputDetect_PIN 21

// #define UART
                            #ifndef UART
#define UART_SUZHOU
                            #ifndef UART_SUZHOU
#define UART_CLO3
                            #endif
                            #endif
                            #endif