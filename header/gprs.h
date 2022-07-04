#ifndef __GPRS_H
#define __GPRS_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>  // malloc(), NULL, ...
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "../config.h"

#define AT 1
#define AT_check_if_open                            2
#define AT_close                                    3
#define AT_open                                     4
#define AT_check_for_signal                         5
#define AT_reset_IP                                 6
#define AT_check_if_connected                       7
#define AT_detach_from_GPRS                         8
#define AT_attach_to_GPRS                           9
#define AT_enable_single_connection                 10
#define AT_enable_multi_connection                  11
#define AT_ping                                     12
#define AT_bring_up_wireless_connection             13
#define AT_get_local_IP_address                     14
#define AT_start_connection                         15
#define AT_request_initiation_for_data_sending      16
#define AT_request_data_sending_error               17
#define AT_receive                                  18
#define AT_network_connnect                         19
#define AT_network_fail                             20
#define AT_error                                    21


#define AT_Commond                                       "AT"
#define AT_check_if_open_Commond                         "AT+CFUN?"
#define AT_close_Commond                                 "AT+CFUN=0"
#define AT_open_Commond                                  "AT+CFUN=1"
#define AT_check_for_signal_Commond                      "AT+CSQ"
#define AT_reset_IP_Commond                              "AT+CIPSHUT"
#define AT_check_if_connected_Commond                    "AT+CGATT?"
#define AT_detach_from_GPRS_Commond                      "AT+CGATT=0"
#define AT_attach_to_GPRS_Commond                        "AT+CGATT=1"
#define AT_enable_single_connection_Commond              "AT+CIPMUX=0"
#define AT_enable_multi_connection_Commond               "AT+CIPMUX=1"
#define AT_ping_Commond                                  "AT+CSTT="
#define AT_bring_up_wireless_connection_Commond          "AT+CIICR"
#define AT_get_local_IP_address_Commond                  "AT+CIFSR"
#define AT_start_connection_Commond                      "AT+CIPSTART="
#define AT_request_initiation_for_data_sending_Commond   "AT+CIPSEND"
#define AT_receive_Commond                               "+RECEIVE"

#define AT_send_end_Commond "\r\n"
#define AT_reply_end_Commond "\r\r\n"
#define TCP "TCP"
#define UDP "UDP"

#define pingConnect 1
#define connected   2
#define closed      3

#define sendIdle 0
#define sendAble 1
#define sendError 2

/*
 typedef struct struct_ConnectState
 {
    uint8_t channel;
    uint8_t state;  // unkonwn
    uint8_t networkType;
    uint8_t IP_address[4];//ipv4
    uint16_t port;
    uint8_t connectState;
    uint8_t sendState;
 } ConnectState;
*/
typedef struct AT_STATE_T
{
    uint8_t state;
    uint8_t function_state;
    uint8_t signal_state;
    uint8_t GPRS_state;
    uint8_t reset_state;
    uint8_t connection_mode;
    uint8_t ping_state;
    uint8_t bring_up_wireless_connection_state;
    uint8_t local_IP_address_$first_is_state$[5];//ipv4 first is state
} AT_STATE;

typedef struct struct_Client
{
    uint8_t channel;
    uint8_t state;
    uint8_t  *addr;
    uint8_t *networkType;
    uint16_t port;
    uint8_t beatData[20];
    uint8_t beatDataLen;
    uint8_t connectState;
    uint8_t sendState;
} Client;

// extern AT_STATE *AT_state;
// extern Client client[clientsNumbers];

typedef struct struct_GPRS
{
    AT_STATE *AT_state;
    Client client[clientsNumbers];
} GPRS_t;

void uploadInit(GPRS_t *gprs , uart_inst_t *uart);
void initNet(uart_inst_t *uart);
void sendATCommond(uint8_t AT_CPOMMOND,uart_inst_t *uart);
void sendATCommondMsg(uint8_t* AT_msg,uart_inst_t *uart);
int checkSingleATCommond(uint8_t ATx);
uint8_t* AT_send_message(uint8_t AT_CPOMMOND);
uint8_t* AT_reply_message(uint8_t AT_CPOMMOND);
uint8_t AT_Message_Handle(GPRS_t *gprs,uint8_t type,uint8_t *msg);
uint8_t* clearBuf(uint8_t *rec_buf,uint32_t *recLen,uint8_t *type);
void initATState(GPRS_t *gprs);
void resetATState(GPRS_t *gprs);
uint8_t* start_multi_channel_connection(uint8_t channel,uint8_t *netWorkType,uint8_t* url,uint16_t port);
uint8_t* AT_Ping(uint8_t* url);
uint8_t sendTCPHeader(uint8_t *header,uint16_t len,uint16_t *headerBufferIndex);
uint8_t handleReceive(uint8_t *msg);
uint32_t getKeyWordValue(uint8_t* key, uint8_t* msg);
uint8_t containKeyWords(uint8_t* key, uint8_t* msg);

#define NOState 0

#define OK 1
#define Error 2
#define Other 3

#define function_close 1
#define function_close_error 2
#define function_open 3
#define function_open_error 4
#define function_ready 5

#define hasReset 1
#define hasReset_error 2
#define resetComfirm 3
#define resetComfirm_error 4

#define detach_from_GPRS 1
#define detach_from_GPRS_error 2
#define attach_to_GPRS 3
#define attach_to_GPRS_error 4


#define enable_single_connection 1
#define enable_single_connection_error 2
#define enable_multi_connection 3
#define enable_multi_connection_error 4

#define bring_up_wireless_connection_state_success 1
#define bring_up_wireless_connection_state_error 2
 
#define ping_state_success 1
#define ping_state_error 2

#define Handle_idle 0
#define Handle_busy 1
#define Handle_before_finish 2

#define tryTimesConst 3

#ifdef __cplusplus
}
#endif

#endif