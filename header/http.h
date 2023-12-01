#ifndef __HTTP_H
#define __HTTP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "pico/util/datetime.h"
#include "device.h"

typedef enum
{
    _ = 0,
    _200,
    _400,
    _404
} http_response_type_t;

enum request_state
{
    STATE_METHOD,
    STATE_REPLY,
    STATE_PATH,
    STATE_VERSION,
    STATE_HEADER_KEY,
    STATE_HEADER_VALUE,
    STATE_BODY
};

typedef struct
{
    enum request_state state;
    char method[8];
    char reply[32];
    http_response_type_t response;
    char path[256];
    char destination[256];
    char contentType[32];
    char transferEncoding[32];
    datetime_t date;
    uint8_t chunked;
    char connection[32];
    char header_key[64];
    char header_value[256];
    char origin[64]; // We store the origin so we can reply back with it.
    char host[64];   // We store the host to check against origin.
    char body[512];
    size_t content_length;
    size_t offset;
    uint64_t timestamp_ms;
    bool redirect;
    bool done;
    bool in_progress;
    bool authenticated;
    bool expect;
    bool json;
    bool websocket;
    bool new_socket;
    uint32_t websocket_version;
    // RFC6455 for websockets says this header should be 24 base64 characters long.
    char websocket_key[24 + 1];
} _request;

uint8_t uploadJSON(deviceData_t *deviceData, datetime_t *currentDate, uart_inst_t *uart, uint8_t uart_en_pin);
void reset_request(_request *request);
http_response_type_t handleHttp(uint8_t *httpString, uint16_t *len, _request *request);
void convertChunked(uint8_t *body, uint16_t bodyLen);

#define ID                 no
#define siteID             siteNo
#define siteName           siteName
#define status             status
#define ph_unit            ph_unit
#define wt_unit            wt_unit
#define do_unit            do_unit
#define ec_unit            ec_unit
#define turbidity_unit     turbidity_unit
#define time               time

#ifdef __cplusplus
}
#endif

#endif 
