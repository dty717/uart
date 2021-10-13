#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <string.h>
#include "modbus.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define _rtu_modbus_HEADER_LENGTH      1
#define _rtu_modbus_PRESET_REQ_LENGTH  6
#define _rtu_modbus_PRESET_RSP_LENGTH  2

#define _rtu_modbus_CHECKSUM_LENGTH    2

#define MODBUS_RTU_MAX_ADU_LENGTH  256
#define MODBUS_RTU_MAX_RESPONSE_TIME   20
#define MODBUS_RTU_MIN_RESPONSE_TIME   2

typedef struct _rtu_modbus {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*" on Mac OS X. */
    char *device;
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;

#if HAVE_DECL_TIOCSRS485
    int serial_mode;
#endif
#if HAVE_DECL_TIOCM_RTS
    int rts;
    int rts_delay;
    int onebyte_time;
    void (*set_rts) (modbus_t *ctx, int on);
#endif
    /* To handle many slaves on the same link */
    int confirmation_to_ignore;
    uint8_t isServer;
    uart_inst_t *uart;
    uint8_t uart_en_pin;
    uint8_t *ch;
    uint16_t chars_rxed;
    uint16_t chars_rx_index;
    uint16_t chars_rx_server_index;
} modbus_rtu_t;

MODBUS_API modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                                    int data_bit, int stop_bit,uart_inst_t *uart,uint8_t uart_en_pin,uint8_t isServer);

#endif