#include "../header/common/handler.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "pico/util/datetime.h"
#include "config.h"

uint8_t __twoString[2];
uint8_t *twoString(int val)
{
    if (val < 10)
    {
        __twoString[0] = val + '0';
        __twoString[1] = '\0';
    }
    else
    {
        __twoString[0] = val / 10 + '0';
        __twoString[1] = val % 10 + '0';
    }
    return __twoString;
}
uint8_t startWith(uint8_t *target, uint8_t *source)
{
    uint32_t len = _len_(target);
    for (size_t i = 0; i < len; i++)
    {
        if (source[i] != target[i])
        {
            return 0;
        }
    }
    return 1;
}

uint8_t startWithIndex(uint8_t *target, uint8_t *source, uint16_t start, uint16_t end)
{
    uint32_t len = _len_(target);
    if (len != end - start)
    {
        return 0;
    }
    for (size_t i = start; i < end; i++)
    {
        if (source[i] != target[i - start])
        {
            return 0;
        }
    }
    return 1;
}

uint8_t endWith(uint8_t *target, uint8_t *source)
{
    uint32_t lenSource = _len_(source);
    return endWithWithLen(target, source, lenSource);
}

uint8_t endWithWithLen(uint8_t *target, uint8_t *source, uint32_t lenSource)
{
    uint32_t len = _len_(target);
    if (lenSource < len)
    {
        return 0;
    }
    for (size_t i = 0; i < len; i++)
    {
        if (source[lenSource - len + i] != target[i])
        {
            return 0;
        }
    }
    return 1;
}



float bytesToFloat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    float output;

    *((uint8_t *)(&output) + 3) = b0;
    *((uint8_t *)(&output) + 2) = b1;
    *((uint8_t *)(&output) + 1) = b2;
    *((uint8_t *)(&output) + 0) = b3;

    return output;
}
void floatToByteArray(float f, uint8_t *arrayVal)
{
    unsigned int asInt = *((int *)&f);
    int i;
    for (i = 0; i < 4; i++)
    {
        arrayVal[i] = (asInt >> 8 * i) & 0xFF;
    }
}
void AppendfloatToU8Array(float f, uint8_t *arrayVal, uint16_t index)
{
    unsigned int asInt = *((int *)&f);
    int i;
    for (i = 0; i < 4; i++)
    {
        arrayVal[index + i] = (asInt >> 8 * i) & 0xFF;
    }
}
float _4_20mvTofloat(float I,float Imin,float Imax){
  return (I-4) *(Imax-Imin)/16+Imin;
}
void *AppendfloatToU16Array(float f, uint16_t *target, uint16_t index)
{
    unsigned int asInt = *((int *)&f);
    int i;
    for (i = 0; i < 2; i++)
    {
        target[index + i] = (asInt >> 16 * i) & 0xFFFF;
    }
}

int hexToInt(int hex)
{
    return hex / 16 * 10 + hex % 16;
}
int intToHex(int hex)
{
    return hex / 10 * 16 + hex % 10;
}
void _append(uint8_t *target, uint16_t targetSize, uint8_t *origin, uint16_t *index)
{
    //size_t count = sizeof(target);
    int a = *index;
    targetSize = (targetSize > 0 && target[targetSize - 1] == '\0') ? targetSize - 1 : targetSize;
    for (size_t i = 0; i < targetSize; i++)
    {
        origin[a++] = target[i];
    }
    *index = a;
}

void appendChar(uint8_t target, uint8_t *origin, uint16_t *index)
{
    int a = *index;
    origin[a++] = target;
    *index = a;
}

uint32_t strToNum(uint8_t *target, uint16_t start, uint16_t end)
{
    uint32_t num = 0;
    for (size_t i = start; i < end; i++)
    {
        num *= 10;
        num += target[i] - 0x30;
    }
    return num;
}

void appendNumberToStr(int32_t num, uint8_t *target, uint16_t *index)
{
    if (num < 0)
    {
        target[*index++] = '-';
        num = -num;
    }
    uint8_t size = log10(num);
    uint16_t a = *index;
    // int16_t numValue = (int16_t)num;

    for (int i = size; i >= 0; i--)
    {
        uint8_t tem = num % 10;
        target[a + i] = tem + '0';
        num = (num - tem) / 10;
    }
    a += size + 1;
    *index = a;
}

void appendFloatToStrWithLen(float num, uint8_t *target, uint16_t *index, uint16_t len)
{
    int i ;
    uint16_t a = *index;
    if (num < 0)
    {
        target[a++] = '-';
        num = -num;
    }
    uint8_t size = log10(num);
    int16_t numValue = (int16_t)num;

    for (i= size; i >= 0; i--)
    {
        uint8_t tem = numValue % 10;
        target[a + i] = tem + '0';
        numValue = (numValue - tem) / 10;
    }
    a += size + 1;
    if (len > 0)
    {
        target[a++] = '.';
        uint16_t power = 10;
        for (i = 0; i < len; i++)
        {
            target[a++] = ((uint32_t)(num * power)) % 10 + '0';
            power *= 10;
        }
    }

    *index = a;
}

void appendFloatHexToStrWithLen(float num, uint8_t *target, uint16_t *index, uint16_t len)
{
    int i;
    uint16_t a = *index;
    if (num < 0)
    {
        target[a++] = '2';
        target[a++] = 'D';
        num = -num;
    }
    uint8_t size = log10(num);
    int16_t numValue = (int16_t)num;

    for (i = size; i >= 0; i--)
    {
        uint8_t tem = numValue % 10;
        target[a + 2 * i] = '3';
        target[a + 2 * i + 1] = tem + '0';
        numValue = (numValue - tem) / 10;
    }
    a += 2 *( size + 1);
    if (len > 0)
    {
        target[a++] = '2';
        target[a++] = 'E';
        uint16_t power = 10;
        for (i = 0; i < len; i++)
        {
            target[a++] = '3';
            target[a++] = ((uint32_t)(num * power)) % 10 + '0';
            power *= 10;
        }
    }
    *index = a;
}

void appendFloatToStr(float num, uint8_t *target, uint16_t *index)
{
    uint16_t a = *index;
    if (num < 0)
    {
        target[a++] = '-';
        num = -num;
    }
    uint8_t size = log10(num);
    int16_t numValue = (int16_t)num;

    for (int i = size; i >= 0; i--)
    {
        uint8_t tem = numValue % 10;
        target[a + i] = tem + '0';
        numValue = (numValue - tem) / 10;
    }
    a += size + 1;
    if (num != numValue)
    {
#define checkZero                 \
    target[a++] = numValue + '0'; \
    if (numValue)                 \
    {                             \
        lastNotZero = a;          \
    }
        uint16_t lastNotZero = a;
        numValue = (int16_t)num;
        num -= numValue;
        target[a++] = '.';
        numValue = (int)(num * 10) % 10;
        checkZero;
        numValue = (int)(num * 100) % 10;
        checkZero;
        numValue = (int)(num * 1000) % 10;
        checkZero;
        numValue = (int)(num * 10000) % 10;
        checkZero;
        numValue = (int)(num * 100000) % 10;
        checkZero;
#undef checkZero

        *index = lastNotZero;
    }
    else
    {
        *index = a;
    }
}

void lenStr(int len, int data, uint8_t *str)
{
    int index = 0;
    while (data > 0)
    {
        str[index++] = data % 10 + 0x30;
        data -= data % 10;
        data /= 10;
    }
    if (index < len)
    {
        len = len - index;
        for (int i = 0; i < len; i++)
        {
            str[index++] = 0x30;
        }
    }
    for (int i = 0; i < index / 2; i++)
    {
        uint8_t tem = str[i];
        str[i] = str[index - i - 1];
        str[index - i - 1] = tem;
    }
    str[index] = '\0';
}

uint32_t _len_(uint8_t *buf)
{
    uint32_t len = 0;
    while (buf[len])
    {
        len++;
        if (len > 4294967290)
        {
            break;
        }
    }
    return len;
}

void byteToHexStr(uint8_t crctemp, uint8_t *out)
{
    int tem = crctemp / 16;
    if (tem > 9)
    {
        out[0] = tem - 10 + 'A';
    }
    else
    {
        out[0] = tem + '0';
    }
    tem = crctemp % 16;
    if (tem > 9)
    {
        out[1] = tem - 10 + 'A';
    }
    else
    {
        out[1] = tem + '0';
    }
    out[2] = '\0';
}

void intToHexStr(unsigned int crctemp, uint8_t *out)
{

    int tem = (uint8_t)(crctemp >> 8) / 16;
    if (tem > 9)
    {
        out[0] = tem - 10 + 'A';
    }
    else
    {
        out[0] = tem + '0';
    }
    tem = (uint8_t)(crctemp >> 8) % 16;
    if (tem > 9)
    {
        out[1] = tem - 10 + 'A';
    }
    else
    {
        out[1] = tem + '0';
    }
    tem = (uint8_t)(crctemp & 0xff) / 16;
    if (tem > 9)
    {
        out[2] = tem - 10 + 'A';
    }
    else
    {
        out[2] = tem + '0';
    }
    tem = (uint8_t)(crctemp & 0xff) % 16;
    if (tem > 9)
    {
        out[3] = tem - 10 + 'A';
    }
    else
    {
        out[3] = tem + '0';
    }
    out[4] = '\0';
}

uint32_t toNumber(uint8_t *target, uint16_t startIndex, uint16_t endIndex)
{
    uint32_t val = 0;
    uint32_t ten_power = 1;
    for (size_t i = endIndex - 1; i >= startIndex; i--)
    {
        val += ten_power * (target[i] - '0');
        ten_power *= 10;
    }
    return val;
}

float toFloat(uint8_t *target, uint16_t startIndex, uint16_t endIndex)
{
    uint32_t num = 0;
    size_t i = 0;
    for (i = startIndex; i < endIndex; i++)
    {
        if(target[i]=='.'){
            break;
        }
        num *= 10;
        num += target[i] - 0x30;
    }
    i++;
    uint32_t val = 0;
    uint32_t ten_power = 1;
    for (; i < endIndex; i++)
    {
        val *= 10;
        val += target[i] - 0x30;
        ten_power *= 10;
    }
    // printf("%d %d %d  =>  %d",num,val,ten_power,num+val/(ten_power+0.0));
    return num+val/(ten_power+0.0);
}

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
 	buf[a] = '\0';
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

void assignISOTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec, uint8_t* buf,uint16_t *index) {
    uint16_t a= *index;
    buf[a++] ='2';
    buf[a++] ='0';
    buf[a++] = (year / 10) + '0';
	buf[a++] = (year % 10) + '0';
	buf[a++] =  '-';
	buf[a++] = (month / 10) + '0';
	buf[a++] = (month % 10) + '0';
	buf[a++] =  '-';
    buf[a++] = (date / 10) + '0';
 	buf[a++] = (date % 10) + '0';
	buf[a++] =  ' ';
 	buf[a++] = (hour / 10) + '0';
 	buf[a++] = (hour % 10) + '0';
	buf[a++] =  ':';
    buf[a++] = (min / 10) + '0';
 	buf[a++] = (min % 10) + '0';
	buf[a++] =  ':';
    buf[a++] = (sec / 10) + '0';
 	buf[a++] = (sec % 10) + '0';
    *index= a;
}

uint8_t convertUTCString(uint8_t *utcString, datetime_t *date)
{
    size_t i;
    switch (utcString[0])
    {
        case 'M':
            date->dotw = 1;
            break;
        case 'T':
            if (utcString[1] == 'u')
            {
                date->dotw = 2;
            }
            else
            {
                date->dotw = 4;
            }
            break;
        case 'W':
            date->dotw = 3;
            break;
        case 'F':
            date->dotw = 5;
            break;
        case 'S':
            if (utcString[1] == 'a')
            {
                date->dotw = 6;
            }
            else
            {
                date->dotw = 0;
            }
            break;
        default:
            break;
    }
    date->day = 10 * (utcString[5] - '0') + (utcString[6] - '0');
    switch (utcString[8])
    {
        case 'J':
            if (utcString[9] == 'a')
            {
                date->month = 1;
            }
            else if (utcString[10] == 'n')
            {
                date->month = 6;
            }else{
                date->month = 7;
            }
            break;
        case 'F':
            date->month = 2;
            break;
        case 'M':
            if (utcString[10] == 'r')
            {
                date->month = 3;
            }
            else
            {
                date->month = 5;
            }
            break;
        case 'A':
            if (utcString[9] == 'p')
            {
                date->month = 4;
            }
            else
            {
                date->month = 8;
            }
            break;
        case 'S':
            date->month = 9;
            break;
        case 'O':
            date->month = 10;
            break;
        case 'N':
            date->month = 11;
            break;
        case 'D':
            date->month = 12;
            break;
        default:
            break;
    }
    date->year = 1000 * (utcString[12] - '0') + 100 * (utcString[13] - '0') + 10 * (utcString[14] - '0') + (utcString[15] - '0');
    date->hour = 10 * (utcString[17] - '0') + (utcString[18] - '0');
    date->min = 10 * (utcString[20] - '0') + (utcString[21] - '0');
    date->sec = 10 * (utcString[23] - '0') + (utcString[24] - '0');
    if (utcString[26] == 'G')
    {
        date->hour += TIMEZONE;
    }
    // printf("DataTime:%d-%d-%d %d:%d:%d\r\n", date->year, date->month, date->day, date->hour, date->min, date->sec);
    // printf(":%s\r\n", utcString);
    return 1;
}

uint32_t getKeyWordValue(uint8_t* key, uint8_t* msg) {
    uint32_t len = _len_(msg);
    uint32_t keyLen = _len_(key);
    uint32_t startIndex = 0;
    uint32_t endIndex = 0;
    uint8_t newKey = 1;
    uint32_t j = 0;
    uint16_t msg_skip_spaceShift = 0;
    uint16_t key_skip_spaceShift = 0;
    for (size_t i = 0; i < len; i++)
    {
        if ((msg[i] == '\r' || msg[i] == '\n') && newKey) {
            newKey = 0;
            endIndex = i;
            msg_skip_spaceShift = 0;
            key_skip_spaceShift = 0;
            for (j = startIndex; j + msg_skip_spaceShift < endIndex; j++)
            {
                if (j - startIndex + key_skip_spaceShift == keyLen) {
                    endIndex = j + msg_skip_spaceShift;
                    break;
                }
                while (msg[j + msg_skip_spaceShift] == ' ') {
                    msg_skip_spaceShift++;
                }
                while (key[j - startIndex + key_skip_spaceShift] == ' ') {
                    key_skip_spaceShift++;
                }
                if (msg[j + msg_skip_spaceShift] != key[j - startIndex + key_skip_spaceShift]) {
                    break;
                }
            }
            if (j + msg_skip_spaceShift == endIndex) {
                startIndex = j + msg_skip_spaceShift;
                endIndex = len;
                for (j = startIndex; j < endIndex; j++)
                {
                    if (msg[j] >= '0' && msg[j] <= '9') {
                        startIndex = j;
                        for (; j < endIndex; j++)
                        {
                            if (msg[j] < '0' || msg[j]>'9') {
                                endIndex = j;
                                return toNumber(msg, startIndex, endIndex);
                            }
                        }
                    }
                }
            }
        }
        else if (msg[i] != '\r' && msg[i] != '\n') {
            if (!newKey) {
                startIndex = i;
                newKey = 1;
            }
            else if (i == len - 1) {
                endIndex = len;
                msg_skip_spaceShift = 0;
                key_skip_spaceShift = 0;
                for (j = startIndex; j + msg_skip_spaceShift < endIndex; j++)
                {
                    if (j - startIndex + key_skip_spaceShift == keyLen) {
                        endIndex = j + msg_skip_spaceShift;
                        break;
                    }
                    while (msg[j + msg_skip_spaceShift] == ' ') {
                        msg_skip_spaceShift++;
                    }
                    while (key[j - startIndex + key_skip_spaceShift] == ' ') {
                        key_skip_spaceShift++;
                    }
                    if (msg[j + msg_skip_spaceShift] != key[j - startIndex + key_skip_spaceShift]) {
                        break;
                    }
                }
                if (j + msg_skip_spaceShift == endIndex) {
                    startIndex = j + msg_skip_spaceShift;

                    for (j = startIndex; j < len; j++)
                    {
                        if (msg[j] >= '0' && msg[j] <= '9') {
                            startIndex = j;
                            for (; j < len; j++)
                            {
                                if (msg[j] < '0' || msg[j]>'9') {
                                    endIndex = j;
                                    return toNumber(msg, startIndex, endIndex);
                                }
                                else if (j == len - 1) {
                                    endIndex = len;
                                    return toNumber(msg, startIndex, endIndex);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int64_t duringTime(uint16_t year0, uint8_t month0, uint8_t day0, uint8_t hour0, uint8_t minute0, uint8_t second0,
                    uint16_t year1, uint8_t month1, uint8_t day1, uint8_t hour1, uint8_t minute1, uint8_t second1) {
    time_t t0;
    time_t t1;
    struct tm tm0;
    struct tm tm1;

    tm0.tm_year = year0 - 1900;
    tm0.tm_mon = month0 - 1;
    tm0.tm_mday = day0;
    tm0.tm_hour = hour0;
    tm0.tm_min = minute0;
    tm0.tm_sec = second0;
    tm0.tm_isdst = -1;
    t0 = mktime(&tm0);
    tm1.tm_year = year1 - 1900;
    tm1.tm_mon = month1 - 1;
    tm1.tm_mday = day1;
    tm1.tm_hour = hour1;
    tm1.tm_min = minute1;
    tm1.tm_sec = second1;
    tm1.tm_isdst = -1;
    t1 = mktime(&tm1);
    return t0 - t1;
};


uint8_t containKeyWords(uint8_t *key, uint8_t *msg)
{
    uint32_t len = _len_(msg);
    return containKeyWordsWithLen(key, msg, len);
}

uint8_t containWords(uint8_t *key, uint8_t *msg){
    return strlen(key) == strspn(key, msg);
}

uint8_t containKeyWordsWithLen(uint8_t *key, uint8_t *msg, uint32_t len)
{
    uint32_t keyLen = _len_(key);
    uint32_t startIndex = 0;
    uint32_t endIndex = 0;
    uint8_t newKey = 1;
    uint32_t j = 0;
    uint16_t msg_skip_spaceShift = 0;
    uint16_t key_skip_spaceShift = 0;
    uint16_t last_msg_skip_spaceShift = 0;
    uint16_t last_key_skip_spaceShift = 0;
    uint32_t last_j = 0;
    uint32_t last_startIndex=0;
    uint8_t last_having = 0;
    for (size_t i = 0; i < len; i++)
    {
        if ((msg[i] == '\r' || msg[i] == '\n') && newKey) {
            newKey = 0;
            endIndex = i;
            msg_skip_spaceShift = 0;
            key_skip_spaceShift = 0;
            for (j = startIndex; j + msg_skip_spaceShift < endIndex; j++)
            {
                if (j - startIndex + key_skip_spaceShift == keyLen) {
                    endIndex = j + msg_skip_spaceShift;
                    break;
                }
                while (msg[j + msg_skip_spaceShift] == ' ') {
                    msg_skip_spaceShift++;
                }
                while (key[j - startIndex + key_skip_spaceShift] == ' ') {
                    key_skip_spaceShift++;
                }
                if (msg[j + msg_skip_spaceShift] != key[j - startIndex + key_skip_spaceShift]) {
                    if (j == startIndex) {
                        startIndex++;
                    }
                    else {
                        if (j + msg_skip_spaceShift == endIndex && j - startIndex + key_skip_spaceShift == keyLen)
                        {
                            if(endIndex-keyLen >= 0){
                                return endIndex-keyLen + 1;
                            }
                            return 1;
                        }else{
                            msg_skip_spaceShift = last_msg_skip_spaceShift;
                            key_skip_spaceShift = last_key_skip_spaceShift;
                            j = last_j;
                            startIndex = last_startIndex;
                            startIndex++;
                            last_having = 0;
                        }
                    }
                }else{
                    if(!last_having){
                        last_msg_skip_spaceShift = msg_skip_spaceShift;
                        last_key_skip_spaceShift = key_skip_spaceShift;
                        last_j = j;
                        last_startIndex = startIndex;
                        last_having = 1;
                    }

                }
            }
            if (j + msg_skip_spaceShift == endIndex && j - startIndex + key_skip_spaceShift == keyLen) {
                if(endIndex-keyLen >= 0){
                    return endIndex-keyLen + 1;
                }
                return 1;
            }
        }
        else if (msg[i] != '\r' && msg[i] != '\n') {
            if (!newKey) {
                startIndex = i;
                newKey = 1;
            }
            else if(i==len-1){
                endIndex = len;
                msg_skip_spaceShift = 0;
                key_skip_spaceShift = 0;
                for (j = startIndex; j + msg_skip_spaceShift < endIndex; j++)
                {
                    if (j - startIndex + key_skip_spaceShift == keyLen) {
                        endIndex = j + msg_skip_spaceShift;
                        break;
                    }
                    while (msg[j + msg_skip_spaceShift] == ' ') {
                        msg_skip_spaceShift++;
                    }
                    while (key[j - startIndex + key_skip_spaceShift] == ' ') {
                        key_skip_spaceShift++;
                    }
                    if (msg[j + msg_skip_spaceShift] != key[j - startIndex + key_skip_spaceShift]) {
                        if (j ==startIndex) {
                            startIndex++;
                        }
                        else {
                            break;
                        }
                    }
                }
                if (j + msg_skip_spaceShift == endIndex&& j - startIndex + key_skip_spaceShift == keyLen) {
                    if(endIndex-keyLen >= 0){
                        return endIndex-keyLen + 1;
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}
