#include "pico/types.h"
#include "header/J212.h"
#include "header/http.h"
#include "header/crc.h"
#include "header/device.h"
#include "header/common/handler.h"
#include "config.h"

#define httpHeader "POST /waterQuality/sync HTTP/1.1\r\n"   \
                   "Content-Type: application/json\r\n"     \
                   "User-Agent: delin/0.0.1\r\n"            \
                   "Host: 47.103.30.148:30001\r\n"          \
                   "Accept: */*\r\n"                        \
                   "Accept-Encoding: gzip, deflate, br\r\n" \
                   "Connection: close\r\n"                  \
                   "Content-Length: "

uint8_t http_buf[400];
uint8_t httpBuffer[564] = httpHeader;

#define endUpload()                                          \
    uint16_t sendBufferIndex = _len_(httpHeader);            \
    appendNumberToStr(index, httpBuffer, &sendBufferIndex);  \
    appendArray(ReturnString, httpBuffer, &sendBufferIndex); \
    appendArray(ReturnString, httpBuffer, &sendBufferIndex); \
    _append(http_buf, index, httpBuffer, &sendBufferIndex);  \
    appendArray(ReturnString, httpBuffer, &sendBufferIndex);

uint8_t uploadJSON(deviceData_t *deviceData, datetime_t *currentDate, uart_inst_t *uart, uint8_t uart_en_pin)
{
    uint16_t index = 0;
    size_t i;
    appendArray(_STRINGIFY(LeftBracket), http_buf, &index);

    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(ID), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("1234", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(siteID), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    for (i = 0; i < deviceData->MN_len; i++)
    {
        http_buf[index++] = deviceData->MN[i];
    }
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(siteName), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("siteName", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(status), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("status", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    for (size_t i = 0; i < deviceData->pollutionNums + remainingPollutionNums; i++)
    {
        appendArray(CommaString, http_buf, &index);
        appendArray(QuotationString, http_buf, &index);
        appendArray(deviceData->pollutions[i].code, http_buf, &index);
        appendArray(QuotationString, http_buf, &index);
        appendArray(_STRINGIFY(Colon), http_buf, &index);
        appendFloatToStrWithLen(deviceData->pollutions[i].data, http_buf, &index, 3);
    }

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(ph_unit), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("mol/L", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(wt_unit), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("℃", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(do_unit), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("Mg/L", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(ec_unit), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("μs/cm", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(turbidity_unit), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray("NTU", http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(time), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(QuotationString, http_buf, &index);
    assignISOTime(currentDate->year - 2000, currentDate->month, currentDate->day, currentDate->hour, currentDate->min, currentDate->sec, http_buf, &index);
    appendArray(QuotationString, http_buf, &index);

    appendArray(_STRINGIFY(RightBracket), http_buf, &index);

    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    httpBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, httpBuffer);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    gpio_put(uart_en_pin, 0);
    return 0;
}

void reset_request(_request *request)
{
    request->state = STATE_VERSION;
    request->origin[0] = '\0';
    request->host[0] = '\0';
    request->content_length = 0;
    request->chunked = 0;
    request->offset = 0;
    request->timestamp_ms = 0;
    request->redirect = false;
    request->done = false;
    request->in_progress = false;
    request->new_socket = false;
    request->authenticated = false;
    request->expect = false;
    request->json = false;
    request->websocket = false;
}

http_response_type_t handleHttp(uint8_t *httpString, uint16_t *len, _request *request)
{
    size_t i;
    uint8_t _char;
    bool error = false;

    reset_request(request);

    for (i = 0; i < *len; i++)
    {
        _char = httpString[i];
        switch (request->state)
        {
            case STATE_REPLY:
            {
                if (_char == '\r')
                {
                    request->reply[request->offset] = '\0';
                    if (startWith(" 200", request->reply))
                    {
                        request->response = _200;
                    }
                    else if (startWith(" 400", request->reply))
                    {
                        request->response = _400;
                    }
                    else if (startWith(" 404", request->reply))
                    {
                        request->response = _404;
                    }
                    request->offset = 0;
                    request->state = STATE_HEADER_KEY;
                }
                else if (request->offset > sizeof(request->reply) - 1)
                {
                    // Skip replys that are too long.
                }
                else
                {
                    request->reply[request->offset] = _char;
                    request->offset++;
                }
                break;
            }
            case STATE_METHOD:
            {
                if (_char == ' ')
                {
                    request->method[request->offset] = '\0';
                    request->offset = 0;
                    request->state = STATE_PATH;
                }
                else if (request->offset > sizeof(request->method) - 1)
                {
                    // Skip methods that are too long.
                }
                else
                {
                    request->method[request->offset] = _char;
                    request->offset++;
                }
                break;
            }
            case STATE_PATH:
            {
                if (_char == ' ')
                {
                    request->path[request->offset] = '\0';
                    request->offset = 0;
                    request->state = STATE_VERSION;
                }
                else if (request->offset > sizeof(request->path) - 1)
                {
                    // Skip methods that are too long.
                }
                else
                {
                    request->path[request->offset] = _char;
                    request->offset++;
                }
                break;
            }
            case STATE_VERSION:
            {
                const char *supported_version = "HTTP/1.1";
                error = supported_version[request->offset] != _char;
                request->offset++;
                if (request->offset == strlen(supported_version))
                {
                    request->state = STATE_REPLY;
                    request->offset = 0;
                }
                break;
            }
            case STATE_HEADER_KEY:
            {
                if (_char == '\r')
                {
                    request->state = STATE_BODY;
                    request->offset = 0;
                }
                else if (_char == '\n')
                {
                    // Consume the \n
                }
                else if (_char == ':')
                {
                    request->header_key[request->offset] = '\0';
                    request->offset = 0;
                    request->state = STATE_HEADER_VALUE;
                }
                else if (request->offset > sizeof(request->header_key) - 1)
                {
                    // Skip methods that are too long.
                }
                else
                {
                    request->header_key[request->offset] = _char;
                    request->offset++;
                }
                break;
            }
            case STATE_HEADER_VALUE:
            {
                if (request->offset == 0)
                {
                    error = _char != ' ';
                    request->offset++;
                }
                else if (_char == '\r')
                {
                    request->header_value[request->offset - 1] = '\0';
                    request->offset = 0;
                    request->state = STATE_HEADER_KEY;
                    if (strcasecmp(request->header_key, "Authorization") == 0)
                    {
                        const char *prefix = "Basic ";
                        request->authenticated = strncmp(request->header_value, prefix, strlen(prefix)) == 0 &&
                                                strcmp("_api_password", request->header_value + strlen(prefix)) == 0;
                    }
                    else if (strcasecmp(request->header_key, "Host") == 0)
                    {
                        // Do a prefix check so that port is ignored. Length must be the same or the
                        // header ends in :.
                        const char *cp_local = "circuitpython.local";
                        request->redirect = strncmp(request->header_value, cp_local, strlen(cp_local)) == 0 &&
                                            (strlen(request->header_value) == strlen(cp_local) ||
                                            request->header_value[strlen(cp_local)] == ':');
                        strncpy(request->host, request->header_value, sizeof(request->host) - 1);
                        request->host[sizeof(request->host) - 1] = '\0';
                    }
                    else if (strcasecmp(request->header_key, "Content-Length") == 0)
                    {
                        request->content_length = strtoul(request->header_value, NULL, 10);
                    }
                    else if (strcasecmp(request->header_key, "Expect") == 0)
                    {
                        request->expect = strcmp(request->header_value, "100-continue") == 0;
                    }
                    else if (strcasecmp(request->header_key, "Accept") == 0)
                    {
                        request->json = strcasecmp(request->header_value, "application/json") == 0;
                    }
                    else if (strcasecmp(request->header_key, "Origin") == 0)
                    {
                        strncpy(request->origin, request->header_value, sizeof(request->origin) - 1);
                        request->origin[sizeof(request->origin) - 1] = '\0';
                    }
                    else if (strcasecmp(request->header_key, "X-Timestamp") == 0)
                    {
                        request->timestamp_ms = strtoull(request->header_value, NULL, 10);
                    }
                    else if (strcasecmp(request->header_key, "Upgrade") == 0)
                    {
                        request->websocket = strcmp(request->header_value, "websocket") == 0;
                    }
                    else if (strcasecmp(request->header_key, "Sec-WebSocket-Version") == 0)
                    {
                        request->websocket_version = strtoul(request->header_value, NULL, 10);
                    }
                    else if (strcasecmp(request->header_key, "Sec-WebSocket-Key") == 0 &&
                            strlen(request->header_value) == 24)
                    {
                        strcpy(request->websocket_key, request->header_value);
                    }
                    else if (strcasecmp(request->header_key, "X-Destination") == 0)
                    {
                        strcpy(request->destination, request->header_value);
                    }
                    else if (strcasecmp(request->header_key, "Content-Type") == 0)
                    {
                        strcpy(request->contentType, request->header_value);
                    }
                    else if (strcasecmp(request->header_key, "Transfer-Encoding") == 0)
                    {
                        strcpy(request->transferEncoding, request->header_value);
                        if (containKeyWords("chunked", request->header_value))
                        {
                            request->chunked = 1;
                        }
                    }
                    else if (strcasecmp(request->header_key, "Date") == 0)
                    {
                        convertUTCString(request->header_value, &request->date);
                    }
                    else if (strcasecmp(request->header_key, "Connection") == 0)
                    {
                        strcpy(request->connection, request->header_value);
                    }
                }
                else if (request->offset > sizeof(request->header_value) - 1)
                {
                    // Skip methods that are too long.
                }
                else
                {
                    request->header_value[request->offset - 1] = _char;
                    request->offset++;
                }
                break;
            }
            case STATE_BODY:
                request->done = true;
                request->body[request->offset++] = _char;
                break;
        }
    }
    if (request->body)
    {
        request->body[request->offset] = '\0';
        if(request->chunked){
            convertChunked(request->body,request->offset);
        }
        request->offset = 0;
    }
    return request->response;
}

void convertChunked(uint8_t *body, uint16_t bodyLen)
{
    size_t i;
    uint16_t bodyNewLen = 0;
    uint16_t chunkedLen = 0;
    uint8_t chunked = 0;
    uint8_t _char;
    for (i = 0; i < bodyLen; i++)
    {
        _char = body[i];
        if (chunked)
        {
            if (chunkedLen-- > 0)
            {
                body[bodyNewLen++] = _char;
            }
            else
            {
                i += 1;
                chunked = 0;
                chunkedLen = 0;
            }
        }
        else
        {
            if (_char >= '0' && _char <= '9')
            {
                chunkedLen = chunkedLen * 0x10 + _char - '0';
            }
            else if (_char >= 'A' && _char <= 'F')
            {
                chunkedLen = chunkedLen * 0x10 + _char - 'A' + 10;
            }
            else
            {
                if(chunkedLen){
                    chunked = 1;
                    i++;
                }
            }
        }
    }
    body[bodyNewLen] = '\0';
}