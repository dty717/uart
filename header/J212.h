#ifndef __J212_H
#define __J212_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "device.h"

enum errorFlag
{
    header_ErrorFlag = -1,
    requestRefused_ErrorFlag = -2,
    PW_ErrorFlag = -3,
    MN_ErrorFlag = -4,
    ST_ErrorFlag = -5,
    Flag_ErrorFlag = -6,
    QN_ErrorFlag = -7,
    CN_ErrorFlag = -8,
    CRC_ErrorFlag = -9,
    end_ErrorFlag = -10,
    Unknown_ErrorFlag = -100
};

enum systemCode {
  //上位向现场  
    //初始化命令
    setTimeOutAndErrorCountCMD = 1000, //设置超时时间及重发次数
    
    //参数命令
    getDeviceSystemTimeCMD = 1011,//提取现场机时间
    setDeviceSystemTimeCMD = 1012,//设置现场机时间
    askDeviceSystemTimeCMD = 1013,//问询系统时间
    getRealTimeDataPeriodCMD = 1061,//提取实时数据间隔
    setRealTimeDataPeriodCMD = 1062,//提取实时数据间隔
    getMinutesTimePeriodCMD = 1063,//提取分钟数据间隔
    setMinutesTimePeriodCMD = 1064,//提取分钟数据间隔
    setPasswordCMD = 1072,//设置现场机密码

    //数据命令
    getRealTimeDataCMD= 2011,//取污染物实时数据
    stopGetRealTimeDataCMD = 2012,//停止察看污染物实时数据
    getDeviceStateCMD = 2021,//取设备运行状态数据
    stopGetDeviceStateCMD =2022,//停止察看设备运行状态
    
    getDailyPollutionHistoryCMD = 2031,//取污染物日历史数据
    getDeviceRunningDailyPollutionHistoryCMD = 2041,//取设备运行时间日历史数据

    getMinutesPollutionDataCMD = 2051,//取污染物分钟历史数据
    getHoursPollutionDataCMD = 2061,//取污染物小时历史数据

    deviceOpenTimeCMD = 2081,//上传数采仪开机时间

    //控制命令
    zeroCheckCMD = 3011,//零点校准量程校准
    instantSampleCMD = 3012,//即时采样
    cleanCMD = 3013,//启动清洗
    compareSampleCMD = 3014,//比对采样
    keepPollutedSampleCMD = 3015,//超标留样
    setSamplePeriodCMD = 3016,//设置采样时间周期
    getSamplePeriodCMD = 3017,//提取采样时间周期
    
    getFinishedSampleTimeCMD = 3018,// 提取出样时间
    getDeviceMNCMD = 3019,//提取设备唯一标识
    getDeviceInfoCMD = 3020,//提取现场机信息

    setDeviceParamCMD = 3021,//设置现场机参数

    reqReplyCMD = 9011,//通知应答
    finishReplyCMD = 9012,//数据应答

    noticeReplyCMD = 9013,//通知应答
    dataReplyCMD = 9014//数据应答
};
#define uint8_t_flash_size 1
#define uint16_t_flash_size 2
#define uint32_t_flash_size 4
#define float_flash_size 4

uint8_t uploadDevice(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin);
uint8_t uploadDeviceMinutes(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin);
uint8_t uploadDeviceHours(deviceData_t *deviceData,uart_inst_t *uart,uint8_t uart_en_pin);
uint8_t uploadDeviceWithData(deviceDatas_t *deviceDatas, datetime_t *currentDate, uint16_t cmd, uart_inst_t *uart, uint8_t uart_en_pin);
uint8_t askDeviceSystemTime(deviceData_t *deviceData, uart_inst_t *uart, uint8_t uart_en_pin);

void assignCN(uint16_t cn);
void assignQN(uint8_t *QN);
void assignPW(uint8_t *PW);
void assignST(uint8_t *ST);
uint16_t assignMN(uint8_t *MN,uint16_t MN_len);

uint16_t assignInit(uint8_t *QN, uint8_t *PW, uint8_t *ST, uint8_t *MN,uint16_t MN_len, uint16_t cn);

#define request_reply_priority         8
#define finish_reply_priority          7
#define upload_device_info_priority    5
#define ask_for_system_time_priority   4
#define upload_device_id_priority      3
#define init_device_priority           2

#define sample_time_str -SampleTime=
#define real_time_data_str -Rtd=
#define minute_data_min_str -Min=
#define minute_data_max_str -Max=
#define minute_data_avg_str -Avg=
#define minute_data_cou_str -Cou=
#define device_running_start_str -Flag=
#define device_ExternRunning_start_str -EFlag=
#define FLT_DECIMAL_DIG 10

#ifdef __cplusplus
}
#endif

#endif 

