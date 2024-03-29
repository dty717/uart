// $GNRMC,032352.00,A,3159.64143,N,11851.54803,E,0.025,,080921,,,D*67
#include "header/gps.h"
#include "header/common/handler.h"

gps_response_type_t handleGPSString(uint8_t *gpsString,uint16_t len,gpsData_t* gps_data){
    uint16_t pointIndex = 0;
    uint16_t startIndex = 0;
    gps_response_type_t type;
    if (startWith("$GNRMC", gpsString))
    {
        // printf("get:%s", gpsString);
        type = GNRMC;
        goto analyseGPS;
    }
    else if (startWith("$GPRMC", gpsString))
    {
        // printf("get:%s", gpsString);
        type = GPRMC;
        goto analyseGPS;
    }
    return GNGGA;

    analyseGPS:
        pointIndex = 7;
        startIndex = 7;
        while (pointIndex<len)
        {
            if(gpsString[pointIndex++]==','){
                if(pointIndex>startIndex+5){
                    gps_data->hour = strToNum(gpsString,startIndex,startIndex+2) + time_zone_shift;
                    gps_data->minute = strToNum(gpsString,startIndex+2,startIndex+4);
                    gps_data->second = strToNum(gpsString,startIndex+4,startIndex+6);
                    // printf("time:%d-%-d-%d\r\n",gps_data->hour,gps_data->minute,gps_data->second);
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 1)
                {
                    gps_data->state = gpsString[startIndex];
                    // printf("state:%c\r\n", gps_data->state);
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 1)
                {

                    gps_data->latitude = toFloat(gpsString,startIndex,pointIndex)/100;
                    // printf("Latitude:%f\r\n",gps_data->latitude);
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 1)
                {
                    gps_data->latitudeFlag = gpsString[startIndex];
                    // printf("LatitudeFlag:%c\r\n", gps_data->latitudeFlag);
                }
                startIndex = pointIndex;
                break;
            }
        }

        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 1)
                {
                    gps_data->longitude = toFloat(gpsString, startIndex, pointIndex)/100;
                    // printf("Longitude:%f\r\n", gps_data->longitude);
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 1)
                {
                    gps_data->longitudeFlag = gpsString[startIndex];
                    // printf("LongitudeFlag:%c\r\n", gps_data->longitudeFlag);
                }
                startIndex = pointIndex;
                break;
            }
        }

        while (pointIndex<len)
        {
            if(gpsString[pointIndex++]==','){
                if(pointIndex==startIndex+1){
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex == startIndex + 1)
                {
                }
                startIndex = pointIndex;
                break;
            }
        }
        while (pointIndex < len)
        {
            if (gpsString[pointIndex++] == ',')
            {
                if (pointIndex > startIndex + 5)
                {
                    gps_data->date = strToNum(gpsString, startIndex, startIndex + 2);
                    gps_data->month = strToNum(gpsString, startIndex + 2, startIndex + 4);
                    gps_data->year = strToNum(gpsString, startIndex + 4, startIndex + 6) + 2000;
                    // printf("date:%d-%d-%d\r\n", gps_data->year, gps_data->month, gps_data->date);
                }
                startIndex = pointIndex;
                break;
            }
        }
        return type;
}
