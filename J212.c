#include "pico/types.h"
#include "header/J212.h"
#include "header/crc.h"
#include "header/device.h"
#include "header/common/handler.h"
#include "config.h"

#ifdef UART_KUNSHAN
    uint8_t uploadDevice_buf[2048] = "ST=32;CN=2011;PW=123456;MN=88888880000002001;CP=&&&&";
    uint8_t uploadDeviceBuffer[2048];
    uint16_t MN_index = 24;
#elif defined(UART_JIANGNING)
    uint8_t uploadDevice_buf[2048] = "ST=21;CN=2011;PW=123456;MN=88888880000002001;Flag=0;CP=&&&&";
    uint8_t uploadDeviceBuffer[2048];
    uint16_t MN_index = 24;
#else
    uint8_t uploadDevice_buf[2048] = "QN=20201212080808000;ST=32;CN=2011;PW=123456;MN=010000A8900016F000169DC0;Flag=5;CP=&&&&";
    uint8_t uploadDeviceBuffer[2048];
    uint16_t MN_index = 48;
#endif

uint8_t hourDataValue[32];
uint8_t maxDataValue[32];
uint8_t minDataValue[32];
uint8_t avgDataValue[32];

void assignPW(uint8_t *PW){
    uploadDevice_buf[38]=PW[0];
    uploadDevice_buf[39]=PW[1];
    uploadDevice_buf[40]=PW[2];
    uploadDevice_buf[41]=PW[3];
    uploadDevice_buf[42]=PW[4];
    uploadDevice_buf[43]=PW[5];
}
uint16_t assignMN(uint8_t *MN,uint16_t MN_len){
    size_t i;
    for (i = 0; i < MN_len; i++)
    {
        uploadDevice_buf[MN_index+i]=MN[i];
    }
    uint8_t next[] = ";Flag=5;CP=&&";
    for (i = 0; i < 13; i++)
    {
        uploadDevice_buf[MN_index+MN_len+i]=next[i];
    }
    return 61+MN_len;
}

uint16_t assignMNWithIndex(uint8_t *MN,uint16_t MN_len,uint16_t index){
    size_t i;
    uploadDevice_buf[index++] = 'M';
    uploadDevice_buf[index++] = 'N';
    uploadDevice_buf[index++] = '=';
    for (i = 0; i < MN_len; i++)
    {
        uploadDevice_buf[index+i]=MN[i];
    }
    #ifdef UART_KUNSHAN
        uint8_t next[] = ";CP=&&";
        uint16_t len = 6;
    #elif defined(UART_JIANGNING)
        uint8_t next[] = ";Flag=0;CP=&&";
        uint16_t len = 13;
    #else
        uint8_t next[] = ";Flag=5;CP=&&";
        uint16_t len = 13;
    #endif
    for (i = 0; i < len; i++)
    {
        uploadDevice_buf[index+MN_len+i]=next[i];
    }
    return index+len+MN_len;
}

uint16_t assignInit(uint8_t *QN, uint8_t *PW, uint8_t *ST, uint8_t *MN,uint16_t MN_len, uint16_t cn)
{
    #ifdef UART_SUZHOU
        return assignMNWithIndex(MN,MN_len,0);
    #endif
    #ifdef UART_KUNSHAN
        return assignMNWithIndex(MN,MN_len,MN_index);
    #endif
    #ifdef UART_JIANGNING
        assignST(ST);
        assignCN(cn);
        assignPW(PW);
        return assignMNWithIndex(MN,MN_len,MN_index);
    #endif
    assignQN(QN);
    assignPW(PW);
    assignST(ST);
    assignCN(cn);
    return assignMN(MN,MN_len);
}

void assignQN(uint8_t *QN){
    for (size_t i = 0; i < 17; i++)
    {
        uploadDevice_buf[i+3]=QN[i];
    }
}

void assignCN(uint16_t cn){
	char CP_len[5];
    lenStr(4, cn, CP_len);

    for (size_t i = 0; i < 4; i++)
    {
        #if defined(UART_KUNSHAN) || defined(UART_JIANGNING)
            uploadDevice_buf[9+i]=CP_len[i];
        #else
            uploadDevice_buf[30+i]=CP_len[i];
        #endif
    }
}

void assignST(uint8_t *ST){
    uploadDevice_buf[24] = ST[0];
    uploadDevice_buf[25] = ST[1];
}


#define endUpload() appendArray("&&",uploadDevice_buf,&index);\
	char CP_len[5];\
	lenStr(4, index, CP_len);\
    unsigned int crctemp = crc16_J212(uploadDevice_buf, 0, index);\
	char crcOut[5];\
	intToHexStr(crctemp, crcOut);\
    uint16_t sendBufferIndex=0;\
    appendArray("##",uploadDeviceBuffer,&sendBufferIndex);\
    appendArray(CP_len,uploadDeviceBuffer,&sendBufferIndex);\
    _append(uploadDevice_buf,index,uploadDeviceBuffer,&sendBufferIndex);\
    appendArray(crcOut,uploadDeviceBuffer,&sendBufferIndex);\
    appendArray("\r\n",uploadDeviceBuffer,&sendBufferIndex);


uint8_t uploadDevice(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin){

        // #if usingSIM
        //     if(uploadInitState!=1){
        //         resetWithBit(upload_device_info_priority);
        //         return uploadInitState;
        //     }
        // #endif

    // int needDebug = bits;
    // if(needDebug!=12){
    //     resetWithBit(upload_device_info_priority);
    //     return 1;
    // }
    int waitTimes=0;

    uint8_t *QN = assignTimeDecimalMicroSec(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second);
    
    uint16_t index = assignInit(QN,deviceData->PW,"32",deviceData->MN,deviceData->MN_len, getRealTimeDataCMD);

    appendArray("DataTime=",uploadDevice_buf,&index);
	assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
    // addDeviceData(U1);
    // addDeviceData(U2);
    // addDeviceData(U3);
    // addDeviceData(U4);
    // addDeviceData(U5);
    
    for (size_t i = 0; i < deviceData->pollutionNums+remainingPollutionNums; i++)
    {
        if(deviceData->pollutions[i].state){
            #ifdef UART_SUZHOU
                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
                appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
                continue;
            #endif
            #if defined(UART_KUNSHAN) || defined(UART_JIANGNING)
                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,3);
                appendArray(",", uploadDevice_buf, &index);

                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
                appendChar('N', uploadDevice_buf, &index);
                continue;
            #else
        
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(sample_time_str), uploadDevice_buf, &index);
            assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
            #ifdef UART_HUBEI
                if (deviceData->pollutions[i].code == "w01001")
                {
                    deviceData->pollutions[i].data = (int)(deviceData->pollutions[i].data + 0.5/1) + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 0);
                }
                else if (deviceData->pollutions[i].code == "w21003")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5/100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 2);
                }
                else if (deviceData->pollutions[i].code == "w21011")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5/1000) * 1000) / 1000.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 3);
                }
                else if (deviceData->pollutions[i].code == "w01003")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5/100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 2);
                }
                else
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5/10) * 10) / 10.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index, 1);
                }
            #else
                appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
            #endif
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
            appendChar('N', uploadDevice_buf, &index);
            #endif
        }
    }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    uploadDeviceBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, uploadDeviceBuffer);
    // uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    sleep_ms(60);
    #ifndef UART_JIANGNING
    gpio_put(uart_en_pin, 0);
    #endif
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);

    free(QN);
    QN = NULL;
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


uint8_t uploadDeviceMinutes(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin){

        // #if usingSIM
        //     if(uploadInitState!=1){
        //         resetWithBit(upload_device_info_priority);
        //         return uploadInitState;
        //     }
        // #endif

    // int needDebug = bits;
    // if(needDebug!=12){
    //     resetWithBit(upload_device_info_priority);
    //     return 1;
    // }
    int waitTimes=0;

    uint8_t *QN = assignTimeDecimalMicroSec(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second);
    
    uint16_t index = assignInit(QN,deviceData->PW,"32",deviceData->MN,deviceData->MN_len, getMinutesPollutionDataCMD);

    appendArray("DataTime=",uploadDevice_buf,&index);
	assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
    // addDeviceData(U1);
    // addDeviceData(U2);
    // addDeviceData(U3);
    // addDeviceData(U4);
    // addDeviceData(U5);
    
    for (size_t i = 0; i < deviceData->pollutionNums+remainingPollutionNums; i++)
    {
        if(deviceData->pollutions[i].state){
            #ifdef UART_SUZHOU
                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
                appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
                continue;
            #endif
            #if defined(UART_KUNSHAN) || defined(UART_JIANGNING)

                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_min_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_avg_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_max_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_cou_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
                appendChar('N', uploadDevice_buf, &index);
                continue;
            #else
        
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(sample_time_str), uploadDevice_buf, &index);
            assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
            appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
            appendChar('N', uploadDevice_buf, &index);
            #endif
        }
    }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    #ifndef UART_JIANGNING
    gpio_put(uart_en_pin, 0);
    #endif
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);

    free(QN);
    QN = NULL;
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

uint8_t uploadDeviceHours(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin)
{
    int waitTimes = 0;
    uint8_t *QN = assignTimeDecimalMicroSec(deviceData->year, deviceData->month, deviceData->date, deviceData->hour, deviceData->minute, deviceData->second);
    uint16_t index = assignInit(QN, deviceData->PW, "21", deviceData->MN, deviceData->MN_len, getHoursPollutionDataCMD);

    appendArray("DataTime=", uploadDevice_buf, &index);
    assignTime(deviceData->year, deviceData->month, deviceData->date, deviceData->hour, deviceData->minute, deviceData->second, uploadDevice_buf, &index);

    uint16_t dataIndex = 0;

    for (size_t i = 0; i < deviceData->pollutionNums + remainingPollutionNums; i++)
    {
        if (deviceData->pollutions[i].state)
        {
            dataIndex = 0;
            #ifdef UART_HUBEI
                if (deviceData->pollutions[i].code == "w01001")
                {
                    deviceData->pollutions[i].data = (int)(deviceData->pollutions[i].data + 0.5 / 1) + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 0);
                }
                else if (deviceData->pollutions[i].code == "w21003")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 2);
                }
                else if (deviceData->pollutions[i].code == "w21011")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 1000) * 1000) / 1000.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 3);
                }
                else if (deviceData->pollutions[i].code == "w01003")
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 100) * 100) / 100.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 2);
                }
                else
                {
                    deviceData->pollutions[i].data = (int)((deviceData->pollutions[i].data + 0.5 / 10) * 10) / 10.0 + (deviceData->pollutions[i].data > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 1);
                }
            #else
                appendFloatToStrWithLen(deviceData->pollutions[i].data, hourDataValue, &dataIndex, 2);
            #endif
            hourDataValue[dataIndex] = '\0';
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_min_str), uploadDevice_buf, &index);
            appendArray(hourDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_avg_str), uploadDevice_buf, &index);
            appendArray(hourDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_max_str), uploadDevice_buf, &index);
            appendArray(hourDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_cou_str), uploadDevice_buf, &index);
            appendArray(hourDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
            appendChar('N', uploadDevice_buf, &index);
        }
    }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    uploadDeviceBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, uploadDeviceBuffer);
    // uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    #ifndef UART_JIANGNING
        gpio_put(uart_en_pin, 0);
    #endif
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    free(QN);
    QN = NULL;
    return 0;
}

// double pollutionsValues[12][240];
// int pollutionsValuesIndex[12];

double getMaxValue(double *pollutionsValues, int pollutionsValuesIndex)
{
    double maxPollutionsValue = pollutionsValues[0];
    for (size_t i = 1; i < pollutionsValuesIndex; i++)
    {
        if (maxPollutionsValue < pollutionsValues[i])
        {
            maxPollutionsValue = pollutionsValues[i];
        }
    }
    return maxPollutionsValue;
}

double getMinValue(double *pollutionsValues, int pollutionsValuesIndex)
{
    double minPollutionsValue = pollutionsValues[0];
    for (size_t i = 1; i < pollutionsValuesIndex; i++)
    {
        if (minPollutionsValue > pollutionsValues[i])
        {
            minPollutionsValue = pollutionsValues[i];
        }
    }
    return minPollutionsValue;
}

double getAvgValue(double *pollutionsValues, int pollutionsValuesIndex)
{
    double avgPollutionsValue = 0;
    for (size_t i = 0; i < pollutionsValuesIndex; i++)
    {
        avgPollutionsValue += pollutionsValues[i];
    }
    return avgPollutionsValue / pollutionsValuesIndex;
}

uint8_t uploadDeviceWithData(deviceDatas_t *deviceDatas, datetime_t *currentDate, uint16_t cmd, uart_inst_t *uart, uint8_t uart_en_pin)
{
    int waitTimes = 0;

    uint8_t *QN = assignTimeDecimalMicroSec(currentDate->year - 2000, currentDate->month, currentDate->day, currentDate->hour, currentDate->min, currentDate->sec);
    uint16_t index = assignInit(QN, deviceDatas->PW, "21", deviceDatas->MN, deviceDatas->MN_len, cmd);
    appendArray("DataTime=", uploadDevice_buf, &index);
    assignTime(currentDate->year - 2000, currentDate->month, currentDate->day, currentDate->hour, currentDate->min, currentDate->sec, uploadDevice_buf, &index);

    uint16_t maxDataIndex = 0;
    uint16_t minDataIndex = 0;
    uint16_t avgDataIndex = 0;
    double maxValue = 0;
    double minValue = 0;
    double avgValue = 0;
    for (size_t i = 0; i < deviceDatas->pollutionNums + remainingPollutionNums; i++)
    {
        if (deviceDatas->pollutions[i].dataIndex > 0||(deviceDatas->pollutions[i].state != 1 && deviceDatas->pollutions[i].state != 'N'))
        {
            maxDataIndex = 0;
            minDataIndex = 0;
            avgDataIndex = 0;
            if (deviceDatas->pollutions[i].state != 1 && deviceDatas->pollutions[i].state != 'N')
            {
                maxValue = 0;
                minValue = 0;
                avgValue = 0;
            }
            else
            {
                maxValue = getMaxValue(deviceDatas->pollutions[i].data, deviceDatas->pollutions[i].dataIndex);
                minValue = getMinValue(deviceDatas->pollutions[i].data, deviceDatas->pollutions[i].dataIndex);
                avgValue = getAvgValue(deviceDatas->pollutions[i].data, deviceDatas->pollutions[i].dataIndex);
            }
            #ifdef UART_HUBEI
                if (deviceDatas->pollutions[i].code == "w01001")
                {
                    maxValue = (int)(maxValue + 0.5 / 1) + (maxValue > 0 ? 1 : -1) / 100000000.0;
                    minValue = (int)(minValue + 0.5 / 1) + (minValue > 0 ? 1 : -1) / 100000000.0;
                    avgValue = (int)(avgValue + 0.5 / 1) + (avgValue > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 0);
                    appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 0);
                    appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 0);
                }
                else if (deviceDatas->pollutions[i].code == "w21003")
                {
                    maxValue = (int)((maxValue + 0.5 / 100) * 100) / 100.0 + (maxValue > 0 ? 1 : -1) / 100000000.0;
                    minValue = (int)((minValue + 0.5 / 100) * 100) / 100.0 + (minValue > 0 ? 1 : -1) / 100000000.0;
                    avgValue = (int)((avgValue + 0.5 / 100) * 100) / 100.0 + (avgValue > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 2);
                    appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 2);
                    appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 2);
                }
                else if (deviceDatas->pollutions[i].code == "w21011")
                {
                    maxValue = (int)((maxValue + 0.5 / 1000) * 1000) / 1000.0 + (maxValue > 0 ? 1 : -1) / 100000000.0;
                    minValue = (int)((minValue + 0.5 / 1000) * 1000) / 1000.0 + (minValue > 0 ? 1 : -1) / 100000000.0;
                    avgValue = (int)((avgValue + 0.5 / 1000) * 1000) / 1000.0 + (avgValue > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 3);
                    appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 3);
                    appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 3);
                }
                else if (deviceDatas->pollutions[i].code == "w01003")
                {
                    maxValue = (int)((maxValue + 0.5 / 100) * 100) / 100.0 + (maxValue > 0 ? 1 : -1) / 100000000.0;
                    minValue = (int)((minValue + 0.5 / 100) * 100) / 100.0 + (minValue > 0 ? 1 : -1) / 100000000.0;
                    avgValue = (int)((avgValue + 0.5 / 100) * 100) / 100.0 + (avgValue > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 2);
                    appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 2);
                    appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 2);
                }
                else
                {
                    maxValue = (int)((maxValue + 0.5 / 10) * 10) / 10.0 + (maxValue > 0 ? 1 : -1) / 100000000.0;
                    minValue = (int)((minValue + 0.5 / 10) * 10) / 10.0 + (minValue > 0 ? 1 : -1) / 100000000.0;
                    avgValue = (int)((avgValue + 0.5 / 10) * 10) / 10.0 + (avgValue > 0 ? 1 : -1) / 100000000.0;
                    appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 1);
                    appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 1);
                    appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 1);
                }
            #else
                appendFloatToStrWithLen(maxValue, maxDataValue, &maxDataIndex, 2);
                appendFloatToStrWithLen(minValue, minDataValue, &minDataIndex, 2);
                appendFloatToStrWithLen(avgValue, avgDataValue, &avgDataIndex, 2);
            #endif
            maxDataValue[maxDataIndex] = '\0';
            minDataValue[minDataIndex] = '\0';
            avgDataValue[avgDataIndex] = '\0';
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceDatas->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_min_str), uploadDevice_buf, &index);
            appendArray(minDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceDatas->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_avg_str), uploadDevice_buf, &index);
            appendArray(avgDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceDatas->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(minute_data_max_str), uploadDevice_buf, &index);
            appendArray(maxDataValue, uploadDevice_buf, &index);
            appendArray(",", uploadDevice_buf, &index);
            // appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            // appendArray(_STRINGIFY(minute_data_cou_str), uploadDevice_buf, &index);
            // appendArray(hourDataValue, uploadDevice_buf, &index);
            // appendArray(",", uploadDevice_buf, &index);
            appendArray(deviceDatas->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
            #ifdef UART_HUBEI
                if (deviceDatas->pollutions[i].state == 1 || deviceDatas->pollutions[i].state == 'N')
                {
                    appendChar('N', uploadDevice_buf, &index);
                }
                else
                {
                    appendChar((uint8_t)deviceDatas->pollutions[i].state, uploadDevice_buf, &index);
                }
            #else
                appendChar('N', uploadDevice_buf, &index);
            #endif
        }
    }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    // printf("index:%d\r\n",index);
    uploadDeviceBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, uploadDeviceBuffer);
    // uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
#if !(defined(UART_JIANGNING) || defined(UART_HUBEI))
    gpio_put(uart_en_pin, 0);
#endif
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    free(QN);
    QN = NULL;
    return 0;
}

uint8_t askDeviceSystemTime(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin)
{
    int waitTimes = 0;
    uint8_t *QN = assignTimeDecimalMicroSec(deviceData->year - 2000, deviceData->month, deviceData->date, deviceData->hour, deviceData->minute, deviceData->second);
    uint16_t index = assignInit(QN, deviceData->PW, "32", deviceData->MN, deviceData->MN_len, askDeviceSystemTimeCMD);
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    uploadDeviceBuffer[sendBufferIndex] = '\0';
    uart_puts(uart, uploadDeviceBuffer);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    sleep_ms(60);
    #ifndef UART_JIANGNING
    gpio_put(uart_en_pin, 0);
    #endif
    free(QN);
    QN = NULL;
    return 0;
}

uint8_t uploadHeartBeat(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin){

        // #if usingSIM
        //     if(uploadInitState!=1){
        //         resetWithBit(upload_device_info_priority);
        //         return uploadInitState;
        //     }
        // #endif

    // int needDebug = bits;
    // if(needDebug!=12){
    //     resetWithBit(upload_device_info_priority);
    //     return 1;
    // }
    int waitTimes=0;

    uint8_t *QN = assignTimeDecimalMicroSec(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second);
    
    uint16_t index = assignInit(QN,deviceData->PW,"21",deviceData->MN,deviceData->MN_len, getMinutesPollutionDataCMD);

    appendArray("DataTime=",uploadDevice_buf,&index);
	assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
    // addDeviceData(U1);
    // addDeviceData(U2);
    // addDeviceData(U3);
    // addDeviceData(U4);
    // addDeviceData(U5);
    
    for (size_t i = 0; i < deviceData->pollutionNums+remainingPollutionNums; i++)
    {
        if(deviceData->pollutions[i].state){
            #ifdef UART_SUZHOU
                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
                appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
                continue;
            #endif
            #if defined(UART_KUNSHAN) || defined(UART_JIANGNING)

                appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_min_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_avg_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_max_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(minute_data_cou_str), uploadDevice_buf, &index);
                appendFloatToStrWithLen(deviceData->pollutions[i].data, uploadDevice_buf, &index,2);
                appendArray(",", uploadDevice_buf, &index);
                appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
                appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
                appendChar('N', uploadDevice_buf, &index);
                continue;
            #else
        
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(sample_time_str), uploadDevice_buf, &index);
            assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(real_time_data_str), uploadDevice_buf, &index);
            appendFloatToStr(deviceData->pollutions[i].data, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);

            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            appendArray(_STRINGIFY(device_running_start_str), uploadDevice_buf, &index);
            appendChar('N', uploadDevice_buf, &index);
            #endif
        }
    }
    endUpload();
    gpio_put(uart_en_pin, 1);
    sleep_us(50);
    uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    sleep_us(10152 * 1000 / BAUD_RATE2 * (sendBufferIndex + 1));
    #ifndef UART_JIANGNING
    gpio_put(uart_en_pin, 0);
    #endif
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);

    free(QN);
    QN = NULL;
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



