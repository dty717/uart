#include "pico/types.h"
#include "header/beidou.h"
#include "header/device.h"
#include "header/common/handler.h"
#include "config.h"

uint8_t beidou_buf[256] = "";
uint8_t beidouBuffer[256] = "";

uint8_t checkBuf(uint8_t* recBuf, uint16_t recBufLen){
    uint8_t checkVal = 0;
    size_t i = 0;
    for (; i < recBufLen; i++)
    {
        checkVal ^= recBuf[i];
    }
    return checkVal;
}

#define endUpload()                                              \
    uint16_t sendBufferIndex = 0;                                \
    appendArray(DollarString, beidouBuffer, &sendBufferIndex);   \
    _append(beidou_buf, index, beidouBuffer, &sendBufferIndex);  \
    appendArray(AsteriskString, beidouBuffer, &sendBufferIndex); \
    uint8_t checktemp = checkBuf(beidou_buf, index);             \
    char crcOut[3];                                              \
    byteToHexStr(checktemp, crcOut);                             \
    appendArray(crcOut, beidouBuffer, &sendBufferIndex);         \
    appendArray("\r\n", beidouBuffer, &sendBufferIndex);


uint8_t uploadBeidou(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin)
{
    uint16_t index = 0;
    size_t i = 0;
    char hexOut[3];
    // appendArray(DollarString, beidou_buf, &index);
    appendArray(Send_Info_CMD, beidou_buf, &index);
    appendArray(CommaString, beidou_buf, &index);
    appendArray(_STRINGIFY(beidouReceiverCardID), beidou_buf, &index);
    appendArray(CommaString, beidou_buf, &index);
    appendArray(_STRINGIFY(beidouChannel), beidou_buf, &index);
    appendArray(CommaString, beidou_buf, &index);
    appendArray(_STRINGIFY(beidouNeedComfirm), beidou_buf, &index);
    appendArray(CommaString, beidou_buf, &index);
    appendArray(_STRINGIFY(beidouCodeTpye), beidou_buf, &index);
    appendArray(CommaString, beidou_buf, &index);

    for (i = 0; i < deviceData->MN_len; i++)
    {
        byteToHexStr(deviceData->MN[i], hexOut);
        beidou_buf[index++]=hexOut[0];
        beidou_buf[index++]=hexOut[1];
    }
    appendArray("3A", beidou_buf, &index);
    for (i = 0; i < deviceData->pollutionNums + remainingPollutionNums; i++)
    {
        if (i != 0)
        {
            appendArray("2C", beidou_buf, &index);
        }
        appendFloatHexToStrWithLen(deviceData->pollutions[i].data, beidou_buf, &index, 3);
    }
    appendArray(CommaString, beidou_buf, &index);
    appendArray(_STRINGIFY(beidouSendFrequency), beidou_buf, &index);

    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    beidouBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, beidouBuffer);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    gpio_put(uart_en_pin, 0);
    return 0;
}
