// $GNRMC,032352.00,A,3159.64143,N,11851.54803,E,0.025,,080921,,,D*67
#include "header/gps.h"
#include "header/common/handler.h"

gps_response_type_t handleGPSString(uint8_t *gpsString,uint16_t len,gpsData_t* gpsData){
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
                    gpsData->hour = strToNum(gpsString,startIndex,startIndex+2) + time_zone_shift;
                    gpsData->minute = strToNum(gpsString,startIndex+2,startIndex+4);
                    gpsData->second = strToNum(gpsString,startIndex+4,startIndex+6);
                    // printf("time:%d-%-d-%d\r\n",gpsData->hour,gpsData->minute,gpsData->second);
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
                    gpsData->state = gpsString[startIndex];
                    // printf("state:%c\r\n", gpsData->state);
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

                    gpsData->latitude = toFloat(gpsString,startIndex,pointIndex)/100;
                    // printf("Latitude:%f\r\n",gpsData->latitude);
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
                    gpsData->latitudeFlag = gpsString[startIndex];
                    // printf("LatitudeFlag:%c\r\n", gpsData->latitudeFlag);
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
                    gpsData->longitude = toFloat(gpsString, startIndex, pointIndex)/100;
                    // printf("Longitude:%f\r\n", gpsData->longitude);
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
                    gpsData->longitudeFlag = gpsString[startIndex];
                    // printf("LongitudeFlag:%c\r\n", gpsData->longitudeFlag);
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
                    gpsData->date = strToNum(gpsString, startIndex, startIndex + 2);
                    gpsData->month = strToNum(gpsString, startIndex + 2, startIndex + 4);
                    gpsData->year = strToNum(gpsString, startIndex + 4, startIndex + 6) + 2000;
                    // printf("date:%d-%d-%d\r\n", gpsData->year, gpsData->month, gpsData->date);
                }
                startIndex = pointIndex;
                break;
            }
        }
        return type;
}
