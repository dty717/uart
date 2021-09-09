#include "../header/common/handler.h"
#include <math.h>

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
    uint32_t len = _len_(target);
    uint32_t lenSource = _len_(source);
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
void floatToByteArray(float f, uint8_t *arrarVal)
{
    unsigned int asInt = *((int *)&f);
    int i;
    for (i = 0; i < 4; i++)
    {
        arrarVal[i] = (asInt >> 8 * i) & 0xFF;
    }
}
void AppendfloatToU8Array(float f, uint8_t *arrarVal, uint16_t index)
{
    unsigned int asInt = *((int *)&f);
    int i;
    for (i = 0; i < 4; i++)
    {
        arrarVal[index + i] = (asInt >> 8 * i) & 0xFF;
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
    int16_t numValue = (int16_t)num;

    for (int i = size; i >= 0; i--)
    {
        uint8_t tem = numValue % 10;
        target[a + i] = tem + '0';
        numValue = (numValue - tem) / 10;
    }
    a += size + 1;
    *index = a;
}

void appendFloatToStr(float num, uint8_t *target, uint16_t *index)
{
    if (num < 0)
    {
        target[*index++] = '-';
        num = -num;
    }
    uint8_t size = log10(num);
    uint16_t a = *index;
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
