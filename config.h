#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

#define TEST_NUM 10

#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART0_EN_PIN 2
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_EN_PIN 3
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5


#endif