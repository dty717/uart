#include "pico/types.h"
#include "header/J212.h"
#include "header/http.h"
#include "header/crc.h"
#include "header/device.h"
#include "header/common/handler.h"
#include "config.h"

#define httpHeader "POST /test HTTP/1.1\r\n" \
"Content-Type: application/json\r\n" \
"User-Agent: delin/0.0.1\r\n" \
"Accept: */*\r\n" \
"Accept-Encoding: gzip, deflate, br\r\n" \
"Connection: close\r\n" \
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



uint8_t uploadJSON(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin)
{
    uint16_t index = 0;
    appendArray(_STRINGIFY(LeftBracket), http_buf, &index);

    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(ID), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("1234", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(siteID), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("7894", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(siteName), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("site", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(status), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("status", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    for (size_t i = 0; i < deviceData->pollutionNums + remainingPollutionNums; i++)
    {
        appendArray(CommaString, http_buf, &index);
        appendArray(_STRINGIFY(Quotation), http_buf, &index);
        appendArray(deviceData->pollutions[i].code, http_buf, &index);
        appendArray(_STRINGIFY(Quotation), http_buf, &index);
        appendArray(_STRINGIFY(Colon), http_buf, &index);
        appendFloatToStrWithLen(deviceData->pollutions[i].data, http_buf, &index, 3);
    }

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(ph_unit), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("mol/L", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    
    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(wt_unit), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("℃", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(do_unit), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("Mg/L", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(ec_unit), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("μs/cm", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(turbidity_unit), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("NTU", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);


    appendArray(CommaString, http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(time), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray(_STRINGIFY(Colon), http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);
    appendArray("2023-11-01 00:00:00", http_buf, &index);
    appendArray(_STRINGIFY(Quotation), http_buf, &index);

    appendArray(_STRINGIFY(RightBracket), http_buf, &index);

//     {
//         if (deviceData->pollutions[i].state)
//         {
            
//             // deviceData->pollutions[i].code
// #ifdef UART_SUZHOU
//             appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
//             appendArray(pollutionName(deviceData->pollutions[i].code), uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
//             appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
//             continue;
// #endif
// #if defined(UART_KUNSHAN) || defined(UART_JIANGNING)
//             appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
//             appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
//             appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 3);
//             appendArray(",", uploadDevice_buf, &index);

//             appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
//             appendChar('N', uploadDevice_buf, &index);
//             continue;
// #else

//             appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
//             appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(sample_time_str), uploadDevice_buf, &index);
//             assignTime(deviceData->year, deviceData->month, deviceData->date, deviceData->hour, deviceData->minute, deviceData->second, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

//             appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
// #ifdef UART_HUBEI
//             if (deviceData->pollutions[i].code == "w01001")
//             {
//                 deviceData->pollutions[i].data = (int)(deviceData->pollutions[i].data + 0.5 / 1) + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
//                 appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 0);
//             }
//             else if (deviceData->pollutions[i].code == "w21003")
//             {
//                 deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
//                 appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 2);
//             }
//             else if (deviceData->pollutions[i].code == "w21011")
//             {
//                 deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 1000) * 1000) / 1000.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
//                 appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 3);
//             }
//             else if (deviceData->pollutions[i].code == "w01003")
//             {
//                 deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
//                 appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 2);
//             }
//             else
//             {
//                 deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 10) * 10) / 10.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
//                 appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 1);
//             }
// #else
//             appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
// #endif
//             appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

//             appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
//             appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
//             appendChar('N', uploadDevice_buf, &index);
// #endif
//         }
//     }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    httpBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, httpBuffer);
    // uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    gpio_put(uart_en_pin, 0);
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    // Enable_Ux(U6,_send);
    // if(usingSIM){
    //     if(!SIM_send(sendBufferIndex)){
    //         goto errorSend;
    //     }
    // }else{
    //     HAL_UART_Transmit(&huart6, uploadDeviceBuffer,sendBufferIndex,2000);
    // }
    // osDelay(10);
    // Enable_Ux(U6,_rec);

    // resetWithBit(upload_device_info_priority);
    // return 1;
    // errorSend:
    //     #if usingSIM
    //         uploadInitState = 0;
    //     #endif
    //     resetWithBit(upload_device_info_priority);
    //     #if usingSIM
    //         return uploadInitState;
    //     #endif
    return 0;
}
