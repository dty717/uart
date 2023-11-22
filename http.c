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
