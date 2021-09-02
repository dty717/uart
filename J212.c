#include "header/J212.h"
#include "header/crc.h"
#include "header/device.h"
#include "header/common/handler.h"

uint8_t  uploadDevice_buf[2048]="QN=20201212080808000;ST=21;CN=2011;PW=123456;MN=010000A8900016F000169DC0;Flag=5;CP=&&&&";
uint8_t uploadDeviceBuffer[2048];

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
        uploadDevice_buf[48+i]=MN[i];
    }
    uint8_t next[] = ";Flag=5;CP=&&";
    for (i = 0; i < 13; i++)
    {
        uploadDevice_buf[48+MN_len+i]=next[i];
    }
    return 61+MN_len;
}

uint16_t assignInit(uint8_t *QN, uint8_t *PW, uint8_t *ST, uint8_t *MN,uint16_t MN_len, uint16_t cn)
{
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
        uploadDevice_buf[30+i]=CP_len[i];
    }
}

void assignST(uint8_t *ST){
    uploadDevice_buf[24] = ST[0];
    uploadDevice_buf[25] = ST[1];
}


#define endUpload() appendArray("&&",uploadDevice_buf,&index);\
	char CP_len[5];\
	lenStr(4, index, CP_len);\
    unsigned int crctemp = crc16(uploadDevice_buf, 0, index);\
	char crcOut[5];\
	intToHexStr(crctemp, crcOut);\
    uint16_t sendBufferIndex=0;\
    appendArray("##",uploadDeviceBuffer,&sendBufferIndex);\
    appendArray(CP_len,uploadDeviceBuffer,&sendBufferIndex);\
    _append(uploadDevice_buf,index,uploadDeviceBuffer,&sendBufferIndex);\
    appendArray(crcOut,uploadDeviceBuffer,&sendBufferIndex);\
    appendArray("\r\n",uploadDeviceBuffer,&sendBufferIndex);



uint8_t * assignTimeDecimal(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec) {
    uint8_t *buf;
    uint16_t a=0;
    buf = malloc(14*sizeof(uint8_t));
    buf[a++] ='2';
    buf[a++] ='0';
    buf[a++] = (year / 10) + '0';
	buf[a++] = (year % 10) + '0';
	buf[a++] = (month / 10) + '0';
	buf[a++] = (month % 10) + '0';
 	buf[a++] = (date / 10) + '0';
 	buf[a++] = (date % 10) + '0';
 	buf[a++] = (hour / 10) + '0';
 	buf[a++] = (hour % 10) + '0';
 	buf[a++] = (min / 10) + '0';
 	buf[a++] = (min % 10) + '0';
 	buf[a++] = (sec / 10) + '0';
 	buf[a++] = (sec % 10) + '0';
    return buf;
}

uint8_t * assignTimeDecimalMicroSec(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec) {
    uint8_t *buf;
    uint16_t a=0;
    buf = malloc(14*sizeof(uint8_t));
    buf[a++] ='2';
    buf[a++] ='0';
    buf[a++] = (year / 10) + '0';
	buf[a++] = (year % 10) + '0';
	buf[a++] = (month / 10) + '0';
	buf[a++] = (month % 10) + '0';
 	buf[a++] = (date / 10) + '0';
 	buf[a++] = (date % 10) + '0';
 	buf[a++] = (hour / 10) + '0';
 	buf[a++] = (hour % 10) + '0';
 	buf[a++] = (min / 10) + '0';
 	buf[a++] = (min % 10) + '0';
 	buf[a++] = (sec / 10) + '0';
 	buf[a++] = (sec % 10) + '0';
 	buf[a++] = '0';
 	buf[a++] = '0';
 	buf[a++] = '0';
    return buf;
}

void assignTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec, uint8_t* buf,uint16_t *index) {
    uint16_t a= *index;
    buf[a++] ='2';
    buf[a++] ='0';
    buf[a++] = (year / 10) + '0';
	buf[a++] = (year % 10) + '0';
	buf[a++] = (month / 10) + '0';
	buf[a++] = (month % 10) + '0';
 	buf[a++] = (date / 10) + '0';
 	buf[a++] = (date % 10) + '0';
 	buf[a++] = (hour / 10) + '0';
 	buf[a++] = (hour % 10) + '0';
 	buf[a++] = (min / 10) + '0';
 	buf[a++] = (min % 10) + '0';
 	buf[a++] = (sec / 10) + '0';
 	buf[a++] = (sec % 10) + '0';
    *index= a;
}

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
    uint16_t index = assignInit(QN,deviceData->PW,"21",deviceData->MN,deviceData->MN_len, getRealTimeDataCMD);

    appendArray("DataTime=",uploadDevice_buf,&index);
	assignTime(deviceData->year,deviceData->month,deviceData->date,deviceData->hour,deviceData->minute,deviceData->second, uploadDevice_buf,&index);
    // addDeviceData(U1);
    // addDeviceData(U2);
    // addDeviceData(U3);
    // addDeviceData(U4);
    // addDeviceData(U5);
    for (size_t i = 0; i < deviceData->pollutionNums; i++)
    {
        if(deviceData->pollutions[i].state){
            appendArray(_STRINGIFY(Semicolon), uploadDevice_buf, &index);
            appendArray(deviceData->pollutions[i].code, uploadDevice_buf, &index);
            // printf("code:%s\r\n",uploadDevice_buf);
            // return 0;
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
        }
    }
    endUpload();
    // printf("uploadDeviceBuffer:%s\r\n",uploadDeviceBuffer);
    gpio_put(uart_en_pin, 1);
    sleep_ms(20);
    uart_write_blocking(uart,uploadDeviceBuffer,sendBufferIndex);
    sleep_ms(5);
    gpio_put(uart_en_pin, 0);


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