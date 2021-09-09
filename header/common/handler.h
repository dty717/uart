#ifndef __HANDLER_H
#define __HANDLER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t *twoString(int val);
    float _4_20mvTofloat(float I, float Imin, float Imax);

    float bytesToFloat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    void floatToByteArray(float f, uint8_t *arrarVal);
    void *AppendfloatToU16Array(float f, uint16_t *target, uint16_t index);
    void AppendfloatToU8Array(float f, uint8_t *arrarVal, uint16_t index);
    int hexToInt(int hex);
    int intToHex(int hex);
    void _append(uint8_t *target, uint16_t targetSize, uint8_t *origin, uint16_t *index);
    void appendChar(uint8_t target, uint8_t *origin, uint16_t *index);
    void appendFloatToStr(float num, uint8_t *target, uint16_t *index);
    void appendNumberToStr(int32_t num, uint8_t *target, uint16_t *index);
    uint32_t strToNum(uint8_t *target, uint16_t start, uint16_t end);
    void lenStr(int len, int data, uint8_t *str);
    void intToHexStr(unsigned int crctemp, uint8_t *out);
    uint32_t _len_(uint8_t *buf);
    uint8_t startWith(uint8_t *target, uint8_t *source);
    uint8_t startWithIndex(uint8_t *target, uint8_t *source, uint16_t start, uint16_t end);
    uint8_t endWith(uint8_t *target, uint8_t *source);
    uint32_t toNumber(uint8_t *target, uint16_t startIndex, uint16_t endIndex);
    float toFloat(uint8_t *target, uint16_t startIndex, uint16_t endIndex);
    uint8_t containKeyWords(uint8_t *key, uint8_t *msg);
#define appendArray(target, origin, _index) _append(target, _len_(target), origin, _index)
#define STRINGIFY(x) #x
#define _STRINGIFY(x) STRINGIFY(x)
#define str_3(A, B, C) \
    STRINGIFY(A)       \
    STRINGIFY(B)       \
    STRINGIFY(C)
#define _str_3(A, B, C) \
    _STRINGIFY(A)       \
    _STRINGIFY(B)       \
    _STRINGIFY(C)
#define str_2(A, B) \
    STRINGIFY(A)    \
    STRINGIFY(B)
#define _str_2(A, B) \
    _STRINGIFY(A)    \
    _STRINGIFY(B)
#define __str_2(A, B) A##B
#ifdef __cplusplus
}
#endif

#endif
