#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

#define TEST_NUM 10
#define TIMEZONE 8

#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define BAUD_RATE2 115200

#define nb_points 0x3B

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART0_EN_PIN 2
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_EN_PIN 3
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

#define LED1_PIN 11
#define LED2_PIN 12
#define LED3_PIN 13
#define LED4_PIN 14

#define KEY1_PIN 6
#define KEY2_PIN 7
#define KEY3_PIN 8
#define KEY4_PIN 9

#define PH_ADC_PIN    26
#define PH_ADC         0
#define TUR_ADC_PIN   28
#define TUR_ADC        2

#define InputDetect_PIN 21

#define HistroySaveAddr 150

#define usingSIM

#define DollarString        "$"
#define AsteriskString      "*"
#define Semicolon             ;
#define Colon                 :
#define LeftBracket           {
#define RightBracket          }
#define CommaString         ","
#define ApostropheString    "'"
#define QuotationString    "\""
#define ReturnString     "\r\n"



#define resetSIM_PIN 10
#define clientsNumbers 1

// #define UART
                            #ifndef UART
// #define UART_TEST
                            #ifndef UART_TEST
// #define UART_TEST_GPRS
                            #ifndef UART_TEST_GPRS
// #define UART_TEST2
                            #ifndef UART_TEST2
// #define UART_KUNSHAN
                            #ifndef UART_KUNSHAN
// #define UART_SUZHOU
                            #ifndef UART_SUZHOU
// #define UART_CLO3
                            #ifndef UART_CLO3
// #define UART_JIANGNING
                            #ifndef UART_JIANGNING
// #define UART_HUBEI
                            #ifndef UART_HUBEI
#define UART_TIBET
                            #ifndef UART_TIBET
// #define UART_DRONE
                            #ifndef UART_DRONE
#define UART_AS_PIO
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif
                            #endif

#define PH
#define Temp
#define O2
#define Tur
#define Ele

#ifdef UART_DRONE
    #define PH_Index             0
    #define PH_Code       "w01001"
    #define PH_Addr           0x01
    #define PH_ValueAddr      0x02
    #define Temp_Index           1
    #define Temp_Code     "w01010"
    #define Temp_Addr         0x01
    #define Temp_ValueAddr    0x04
    #define O2_Index             2
    #define O2_Code       "w01009"
    #define O2_Addr           0x04
    #define O2_ValueAddr      0x02
    #define Tur_Index            3
    #define Tur_Code      "w01003"
    #define Tur_Addr          0x03
    #define Tur_ValueAddr     0x02
    #define Ele_Index            4
    #define Ele_Code      "w01014"
    #define Ele_Addr          0x02
    #define Ele_ValueAddr     0x02
    #define USING_BOARD_SmallPicoWithStepperDriver
#endif

#ifdef UART_TIBET
    #define usingBeidou
    #ifdef usingBeidou
        #define beidouReceiverCardID  15950044
        #define beidouChannel                2
        #define beidouNeedComfirm            1
        #define beidouCodeTpye               2
        #define beidouSendFrequency          0
    #else
        #define _api_password             "12"
    #endif

#endif

#ifdef UART_SUZHOU
    #define remainingPollutionNums 0
    
    #define usingMultiDevice

    #ifdef usingMultiDevice
        #define usingLEDScreen
        #define readPHFromADC
        #ifdef readPHFromADC
            #define remainingPollutionNums 2
        #else
            #define remainingPollutionNums 0
        #endif
    #else
        #define remainingPollutionNums 0
    #endif
#else
    #define remainingPollutionNums 0
#endif


#ifdef USING_BOARD_SmallPicoWithStepperDriver
    #define PIO_BAUD_RATE    9600
    #define PIO_RX_PIN         19
    #define DETECT_3_3V_PIN    26
    #define DETECT_3_3V         0
    #define POWER_PIN          22
#else
    #define PIO_BAUD_RATE    9600
    #define PIO_RX_PIN         20
#endif

#endif

