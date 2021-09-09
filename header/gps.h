#ifndef GPS_H
#define GPS_H
#include <stdio.h>
// $GNRMC,032352.00,A,3159.64143,N,11851.54803,E,0.025,,080921,,,D*67
// $GNRMC,033525.00,V,,,,,,,080921,,,N*63
// unsigned int crc16(uint8_t* puchMsg, uint16_t start,uint16_t len);
typedef struct gpsData {
    uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
    uint8_t state;
    float latitude;
    uint8_t latitudeFlag;
    float longitude;
    uint8_t longitudeFlag;
} gpsData_t;

typedef enum {
    GPGSV=0,
    GNGLL,
    GNRMC,
    GNVTG,
    GNGGA,
    GNGSA,
    GPRMC
} gps_response_type_t;

gps_response_type_t handleGPSString(uint8_t *gpsString,uint16_t len,gpsData_t* gpsData);


#define time_zone_shift 8

#endif