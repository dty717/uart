#include "header/gprs.h"
#include "header/common/handler.h"
#include "math.h"
#include <string.h>
// #include "usart.h"
// #include "error.h"
// #include "common/LinkedList.h"
// #include "common/handler.h"

volatile int sendMode;
int uploadInitState;
volatile int currentIndex;
volatile uint16_t sendWithNoReply;
volatile uint8_t handleState;
volatile uint8_t finishedState;

// AT_STATE *AT_state;

// Client client[clientsNumbers] = {
    // {0,NOState,"212.64.11.234",TCP,28888, {99,0,20,99,'\r','\n'},4,NOState,NOState}
    // {1,NOState,"144.202.5.178",TCP,19000, {99,0,20,99,'\r','\n'},4,NOState,NOState},
    // {0,NOState,"144.202.5.178",TCP,19000, {99,0,20,99,'\r','\n'},4,NOState,NOState},
    // {1,NOState,"212.64.11.234",TCP,9009, {99,0,20,99,'\r','\n'},4,NOState,NOState},

    // {2,NOState,"144.202.5.178",TCP,8884, {99,0,20,99,'\r','\n'},4,NOState,NOState},
    // {3,NOState,"144.202.5.178",TCP,8884, {99,0,20,99,'\r','\n'},4,NOState,NOState},
    // {4,NOState,"144.202.5.178",TCP,8884, {99,0,20,99,'\r','\n'},4,NOState,NOState}
// };

int gprs_add_RXData(GPRS_t *gprs,uint8_t ch){
    if(gprs->recLen==1024){
        gprs->recIndex = 0;
        gprs->recLen = 0;
        return 0;
    }
    gprs->rec[gprs->recLen] = ch;
    gprs->recLen++;
    return 1;
}
int gprs_flush(GPRS_t *gprs){
    gprs->recIndex = 0;
    gprs->recLen = 0;
    return 1;
}

#define wait_delay(time, times) \
    waitTimes = times;\
    while (handleState || recFlag ||!finishedState)\
    {\
        if(waitTimes--<=0){\
            break;\
        }\
        sleep_ms(time);\
    }

void uploadInit(GPRS_t *gprs , uart_inst_t *uart)
{
    int8_t tryTimes=tryTimesConst;
    resetATState(gprs);
    gprs->state = NOState;
    uint32_t index=0;
    uint16_t replyChance=0;
    int waitTimes =0;
    uint8_t type = 0;
    uint8_t *msg;
    ok:
        replyChance++;
        if (replyChance > 5)
        {
            goto resetSIM;
        }
        sendATCommondMsg(AT_send_message(AT),uart);
        sleep_ms(100);
        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("ok msg:%s,state:%d\r\n",msg,gprs->AT_state.state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if(type != AT || gprs->AT_state.state!=OK){
            goto ok;
        }
    checkIfOpen:
        sendATCommondMsg(AT_send_message(AT_check_if_open),uart);
        sleep_ms(100);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("checkIfOpen msg:%s,function_state:%d\r\n",msg,gprs->AT_state.function_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if(type != AT_check_if_open){
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }else{
            if(gprs->AT_state.function_state == function_open){
                goto checkForSignal;
            }else if(gprs->AT_state.function_state == function_close){
                goto open;
            }else{
                goto close;
            }
        }
    close:
        sendATCommondMsg(AT_send_message(AT_close),uart);
        sleep_ms(800);
        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("close msg:%s,function_state:%d\r\n",msg,gprs->AT_state.function_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if(type != AT_close || gprs->AT_state.function_state != function_close){
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
    open:
        sendATCommondMsg(AT_send_message(AT_open),uart);
        sleep_ms(12000);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("open msg:%s,function_state:%d\r\n",msg,gprs->AT_state.function_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if(type != function_open){
            sleep_ms(1000);
            resetATState(gprs);
            printf("go to ok\r\n");
            goto ok;
        }else{
            if(gprs->AT_state.function_state != function_open){
                goto checkIfOpen;
            }
        }
    checkForSignal:
        sendATCommondMsg(AT_send_message(AT_check_for_signal),uart);
        sleep_ms(100);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("msg:%s,signal_state:%d\r\n",msg,gprs->AT_state.signal_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if(type != AT_check_for_signal){
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }else{
            if(gprs->AT_state.signal_state<=0){
                sleep_ms(1000);
                goto checkIfOpen;
            }
        }
    checkIfConnected:
        sendATCommondMsg(AT_send_message(AT_check_if_connected),uart);
        sleep_ms(100);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("msg:%s,GPRS_state:%d\r\n",msg,gprs->AT_state.GPRS_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_check_if_connected)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.GPRS_state == detach_from_GPRS)
            {
                sleep_ms(3000);
                goto attachToGPRS;
            }
            else if (gprs->AT_state.GPRS_state == attach_to_GPRS)
            {
                goto enableMultiConnection;
            }
            else if (gprs->AT_state.GPRS_state == attach_to_GPRS_error || gprs->AT_state.GPRS_state == detach_from_GPRS_error)
            {
                sleep_ms(10000);
                printf("gprs error:%d\r\n",gprs->AT_state.GPRS_state);
                resetATState(gprs);
                goto close;
            }else{
                sleep_ms(10000);
                resetATState(gprs);
                goto ok;
            }
        }
    detactFromGPRS:
        sendATCommondMsg(AT_send_message(AT_detach_from_GPRS),uart);
        sleep_ms(100);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("detactFromGPRS:msg:%s,GPRS_state:%d\r\n",msg,gprs->AT_state.GPRS_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_detach_from_GPRS)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.GPRS_state == attach_to_GPRS)
            {
                goto enableMultiConnection;
            }
            else if (gprs->AT_state.GPRS_state == detach_from_GPRS)
            {
                goto attachToGPRS;
            }
            else
            {
                resetATState(gprs);
                goto close;
            }
        }
    attachToGPRS:
        sendATCommondMsg(AT_send_message(AT_attach_to_GPRS),uart);
        sleep_ms(100);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("attachToGPRS:msg:%s,GPRS_state:%d\r\n",msg,gprs->AT_state.GPRS_state);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_attach_to_GPRS)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.GPRS_state == attach_to_GPRS)
            {
                goto enableMultiConnection;
                // goto resetIP;
            }
            else if (gprs->AT_state.GPRS_state == detach_from_GPRS)
            {
                goto attachToGPRS;
            }
            else
            {
                sleep_ms(1000);
                resetATState(gprs);
                goto close;
            }
        }
    enableMultiConnection:
        sendATCommondMsg(AT_send_message(AT_enable_multi_connection),uart);
        sleep_ms(300);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("attachToGPRS:msg:%s,GPRS_state:%d\r\n",msg,gprs->AT_state.connection_mode);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_enable_multi_connection)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.connection_mode == enable_multi_connection)
            {
                // goto resetIP;
            }else if (gprs->AT_state.connection_mode == enable_multi_connection_error)
            {
                // sleep_ms(1000);
                // goto detactFromGPRS;
                goto resetSIM;
            }
            else
            {
                sleep_ms(1000);
                goto enableMultiConnection;
            }
        }
    ping:
        sendATCommondMsg(AT_send_message(AT_ping),uart);
        sleep_ms(300);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("attachToGPRS:msg:%s,ping_state:%d\r\n",msg,gprs->AT_state.ping_state);
        sleep_ms(3000);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_ping)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.ping_state == ping_state_success)
            {
                // goto resetIP;
            }
            else
            {
                sleep_ms(1000);
                resetATState(gprs);
                goto detactFromGPRS;
            }
        }

    bringUpWirelessConnection:
        sendATCommondMsg(AT_send_message(AT_bring_up_wireless_connection),uart);
        sleep_ms(5000);
        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_bring_up_wireless_connection)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.bring_up_wireless_connection_state == bring_up_wireless_connection_state_success)
            {
                // goto resetIP;
            }else if (gprs->AT_state.bring_up_wireless_connection_state == bring_up_wireless_connection_state_error)
            {
                sleep_ms(1000);
                goto detactFromGPRS;
            }
            else
            {
                sleep_ms(1000);
                resetATState(gprs);
                goto close;
            }
        }
    localIPAddress$first_is_state$:
        sendATCommondMsg(AT_send_message(AT_get_local_IP_address),uart);
        sleep_ms(4800);

        msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
        AT_Message_Handle(gprs,type,msg);
        printf("attachToGPRS:msg:%s,GPRS_state:%d\r\n",msg,gprs->AT_state.connection_mode);
        gprs_flush(gprs);
        if(strcmp("",msg)){
            free(msg);
        }
        if (type != AT_get_local_IP_address)
        {
            sleep_ms(1000);
            resetATState(gprs);
            goto ok;
        }
        else
        {
            if (gprs->AT_state.local_IP_address_$first_is_state$[0]!=NOState)
            {
                // goto resetIP;
            }
            else
            {
                sleep_ms(1000);
                resetATState(gprs);
                goto detactFromGPRS;
            }
        }
    gprs->state = Handle_idle;
    return;
    resetSIM:
        resetATState(gprs);
        gprs->state = NOState;
        gpio_init(resetSIM_PIN);
        gpio_set_dir(resetSIM_PIN, GPIO_OUT);
        gpio_put(resetSIM_PIN, 0);
        sleep_ms(200);
        gpio_deinit(resetSIM_PIN);
        sleep_ms(5000);
        return;
}

void setUpConnection(Client client, GPRS_t *gprs, uart_inst_t *uart)
{
    uint8_t type = 0;
    uint8_t *msg;
    gprs_flush(gprs);
    sendATCommondMsg(
        start_multi_channel_connection(client.channel,client.networkType,client.addr,client.port)
    ,uart);
    sleep_ms(1000);
    msg = gprs_readBuf(gprs->rec, gprs->recLen, &type);
    if(type == AT_start_connection){
        if(containKeyWords("ERROR",msg)){
            gprs_flush(gprs);
            sendATCommondMsg(AT_send_message(AT_check_if_connected),uart);
            sleep_ms(100);
            msg = gprs_readBuf(gprs->rec,gprs->recLen,&type);
            AT_Message_Handle(gprs,type,msg);
            gprs_flush(gprs);
            if (strcmp("", msg))
            {
                free(msg);
            }
            if (type != AT_check_if_connected)
            {
                sleep_ms(1000);
                resetATState(gprs);
                return;
            }
            else
            {
                if (gprs->AT_state.GPRS_state == detach_from_GPRS||gprs->AT_state.GPRS_state == attach_to_GPRS_error || gprs->AT_state.GPRS_state == detach_from_GPRS_error)
                {
                    sleep_ms(10000);
                    resetATState(gprs);
                    return;
                }
            }
        }
    }
    AT_Message_Handle(gprs, type, msg);
    gprs_flush(gprs);
    if (strcmp("", msg))
    {
        free(msg);
    }
    sleep_ms(18000);
    if (gprs->recLen)
    {
        msg = gprs_readBuf(gprs->rec, gprs->recLen, &type);
        AT_Message_Handle(gprs, type, msg);
        gprs_flush(gprs);
        if (strcmp("", msg))
        {
            free(msg);
        }
    }

}

void resetATState(GPRS_t *gprs){
    gprs->AT_state.state = NOState;
    gprs->AT_state.function_state = NOState;
    gprs->AT_state.signal_state = NOState;
    gprs->AT_state.GPRS_state = NOState;
    gprs->AT_state.reset_state = NOState;
    gprs->AT_state.connection_mode = NOState;
    gprs->AT_state.bring_up_wireless_connection_state = NOState;
    gprs->AT_state.ping_state = NOState;
    gprs->AT_state.local_IP_address_$first_is_state$[0] = NOState;

    for (size_t i = 0; i < clientsNumbers; i++)
    {
        gprs->client[i].state = NOState;
        gprs->client[i].connectState = NOState;
        gprs->client[i].sendState = NOState;
    }
    sendMode = 0;
}

uint8_t sendTCPHeader(uint8_t *header, uint8_t channel,uint16_t len,uint16_t *headerBufferIndex){
    appendArray(AT_request_initiation_for_data_sending_Commond "=",header,headerBufferIndex);
    appendNumberToStr(channel,header,headerBufferIndex);
    appendArray(",",header,headerBufferIndex);
    appendNumberToStr(len,header,headerBufferIndex);
    appendArray("\r\n",header,headerBufferIndex);
    header[*headerBufferIndex]='\0';
}

uint8_t AT_Message_HandleRecive(GPRS_t *gprs)
{
    uint8_t type = 0;
    uint8_t *msg;
    uint16_t keyIndex = containKeyWordsWithLen(AT_receive_Commond, gprs->rec, gprs->recLen);
    if (keyIndex)
    {
        // gprs_flush(gprs);
        // uart_write_blocking(uart, header, headerBufferIndex);
        // sleep_ms(2000);
        // uart_puts(uart1,"AT_Message_HandleRecive\r\n");
        if (gprs->recLen > keyIndex - 1)
        {
            msg = gprs_readBuf(gprs->rec + (keyIndex - 1), gprs->recLen - (keyIndex - 1), &type);
            if(AT_Message_Handle(gprs, type, msg)){
                gprs_flush(gprs);
            }
        }
    }
}

uint8_t AT_Message_Handle(GPRS_t *gprs, uint8_t type,uint8_t *msg){
    size_t i = 0;
    uint8_t addrIndex = 1;
    uint8_t value = 0;
    switch (type)
    {
    case AT:
        if(startWith("OK",msg)){
            gprs->AT_state.state =OK;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.state =Error;
        }else{
            return 0;
        }
        break;
    case AT_check_if_open:
        if(containKeyWords("+CFUN:",msg)){
            switch (getKeyWordValue("+CFUN",msg))
            {
            case 0:
                gprs->AT_state.function_state = function_close;
                break;
            case 1:
                gprs->AT_state.function_state = function_open;
                break;
            default:
                return 0;
            }
        }else{
            return 0;
        }
        break;
    case AT_close:
        if(startWith("OK",msg)||containKeyWords("+CPIN:NOT READY",msg)){
            gprs->AT_state.function_state = function_close;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.function_state = function_close_error;
        }else{
            return 0;
        }
        break;
    case AT_open:
        if (startWith("OK", msg) || containKeyWords("+CPIN:READY", msg))
        {
            gprs->AT_state.function_state = function_open;
        }
        else if (startWith("ERROR", msg))
        {
            gprs->AT_state.function_state = function_open_error;
        }
        else
        {
            return 0;
        }
        break;
    case AT_check_if_connected:
        if (containKeyWords("+CGATT:", msg) && endWith("OK", msg))
        {
            switch (getKeyWordValue("+CGATT", msg))
            {
            case 0:
                gprs->AT_state.GPRS_state = detach_from_GPRS;
                break;
            case 1:
                gprs->AT_state.GPRS_state = attach_to_GPRS;
                break;
            default:
                return 0;
            }
        }
        else if (startWith("ERROR", msg))
        {
            gprs->AT_state.GPRS_state = attach_to_GPRS_error;
        }
        else
        {
            return 0;
        }
        break;
    case AT_check_for_signal:
        //"AT+CSQ\r\r\n+CSQ: 19,0\r\n\r\nOK\r\n"
        if(startWith("+CSQ:",msg)&&endWith("OK",msg)){
            gprs->AT_state.signal_state = getKeyWordValue("+CSQ:",msg);
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.signal_state = NOState;
        }else{
            return 0;
        }
        break;
    case AT_reset_IP:
        if(startWith("OK",msg)||containKeyWords("SHUT OK",msg)){
            gprs->AT_state.reset_state = resetComfirm;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.reset_state = resetComfirm_error;
        }else{
            return 0;
        }
        break;
    case AT_detach_from_GPRS:
        if(startWith("OK",msg)){
            gprs->AT_state.GPRS_state = detach_from_GPRS;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.GPRS_state = detach_from_GPRS_error;
        }else{
            return 0;
        }
        break;
    case AT_attach_to_GPRS:
        if(startWith("OK",msg)){
            gprs->AT_state.GPRS_state = attach_to_GPRS;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.GPRS_state = attach_to_GPRS_error;
        }else{
            return 0;
        }
        break;
    case AT_enable_single_connection:
        if(startWith("OK",msg)){
            gprs->AT_state.connection_mode = enable_single_connection;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.connection_mode = enable_single_connection_error;
        }else{
            return 0;
        }
        break;
    case AT_enable_multi_connection:
        if(startWith("OK",msg)){
            gprs->AT_state.connection_mode = enable_multi_connection;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.connection_mode = enable_multi_connection_error;
        }else{
            return 0;
        }
        break;
    case AT_ping:
        if(endWith("OK",msg)){
            //handle ping
            gprs->AT_state.ping_state = ping_state_success;
            // client[0].connectState = pingConnect;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.ping_state = ping_state_error;
            //gprs->AT_state.connection_mode = enable_multi_connection_error;
        }else{
            return 0;
        }
        break;
    case AT_bring_up_wireless_connection:
        if(startWith("OK",msg)){
            gprs->AT_state.bring_up_wireless_connection_state = bring_up_wireless_connection_state_success;
        }else if(startWith("ERROR",msg)){
            gprs->AT_state.bring_up_wireless_connection_state = bring_up_wireless_connection_state_error;
        }else{
            return 0;
        }
        break;
    case AT_get_local_IP_address:
        if(_len_(msg)==0){
            return 0;
        }
        gprs->AT_state.local_IP_address_$first_is_state$[0] = 1;
        for (i = 0; i < _len_(msg); i++)
        {
            if (msg[i] <= '9' && msg[i] >= '0')
            {
                value *= 10;
                value += msg[i] - '0';
            }
            else
            {
                gprs->AT_state.local_IP_address_$first_is_state$[addrIndex++] = value;
                value = 0;
                if (addrIndex == 5)
                {
                    break;
                }
            }
        }
        if (addrIndex <= 4)
        {
            gprs->AT_state.local_IP_address_$first_is_state$[addrIndex] = value;
        }
        break;
    case AT_start_connection:
        gprs->AT_state.local_IP_address_$first_is_state$[0] = 1;
        addrIndex = containKeyWords(", CONNECT OK", msg);
        value = 0;
        if (!addrIndex)
        {
            addrIndex = containKeyWords(", ALREADY CONNECT", msg);
        }
        if (addrIndex)
        {
            value = msg[addrIndex - 1 - 1];
            if (value >= '0' && value <= '9')
            {
                if (value - '0' < clientsNumbers)
                {
                    gprs->client[value - '0'].connectState = connected;
                }
            }
        }
        else if (startWith("ERROR", msg))
        {
        }
        else
        {
            return 0;
        }
        break;
    case AT_request_initiation_for_data_sending:
        if(containKeyWords("ERROR",msg)||containKeyWords("FAIL",msg)){
            gprs->client[0].sendState = sendError;
        }else{
            gprs->client[0].sendState = sendAble;
        }
        break;
    case AT_request_data_sending_error:
        if (msg[0] >= '0' && msg[0] <= '9')
        {
            gprs->client[msg[0] - '0'].sendState = sendError;
        }
        break;
    case AT_receive:
        return handleReceive(gprs, msg);
    case AT_network_connnect:
        if (msg[0] >= '0' && msg[0] <= '9')
        {
            gprs->client[msg[0] - '0'].connectState = connected;
        }
        break;
    case AT_network_fail:
        if (msg[0] >= '0' && msg[0] <= '9')
        {
            gprs->client[msg[0] - '0'].connectState = closed;
        }
        //uploadInitState = 0;
        break;
    case AT_error:
        uploadInitState = 2;
        sendMode = 0;
        break;
    default:
        return 0;
    }
    return 1;
}
uint8_t handleReceive(GPRS_t *gprs, uint8_t *msg){
    // uart_puts(uart1,"handleReceive\r\n");
    // return 1;
    #define checkLen     \
        if (index > len) \
        {                \
            return 0;    \
        }

    uint8_t clientChannel = 0;
    uint8_t _char;
    uint16_t recLen = 0;
    uint16_t index = 0;
    uint16_t len = _len_(msg);
    if (msg[index++] == ',')
    {
        clientChannel = msg[index++] - '0';
        checkLen
        _char = msg[index++];
        checkLen 
        while (_char != ',')
        {
            clientChannel = clientChannel * 10 +( _char - '0');
            _char = msg[index++];
            checkLen
        }
        recLen = msg[index++] - '0';
        checkLen
        _char = msg[index++];
        checkLen 
        while (_char != ':')
        {
            recLen = recLen * 10 + _char - '0';
            _char = msg[index++];
            checkLen
        }
        while (msg[index] == '\r' || msg[index] == '\n')
        {
            index++;
            checkLen
        }
        if (len - index >= recLen)
        {
            uint8_t *_msg;
            _msg = (uint8_t *)malloc(recLen);
            for (size_t j = 0; j < recLen; j++)
            {
                _msg[j] = msg[j + index];
            }
            _msg[recLen] = '\0';            
            gprs->client[clientChannel].handleRec(_msg);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 1;
    }
}
#define check(C) rec_buf[i++]==C
#define endCheck (rec_buf[rec_len-2]=='\r'&&rec_buf[rec_len-1]=='\n')

#define condition(_condition) (startWith(AT_reply_message(_condition),rec_buf)&&rec_len>_len_(AT_reply_message(_condition))&&endCheck){\
        *type = _condition;\
        i=_len_(AT_reply_message(_condition));\
        msg = (uint8_t *)malloc(rec_len-2-i);\
        for (size_t j = i; j < rec_len-2; j++)\
        {\
            msg[j-i]= rec_buf[j];\
        }\
        msg[rec_len-2-i] = '\0';\
        return msg;\
    }


uint8_t* gprs_readBuf(uint8_t *rec_buf,uint16_t rec_len,uint8_t *type){
    uint8_t *msg;
    int32_t i=0;
    *type = NOState;

    if(startWith(AT_reply_message(AT),rec_buf)&&endCheck){
        *type = AT; 
        i=_len_(AT_reply_message(AT));
        msg = (uint8_t *)malloc(rec_len-2-i);
        for (size_t j = i; j < rec_len-2; j++) { 
            msg[j-i]= rec_buf[j]; 
        }
        msg[rec_len-2-i] = '\0';
        return msg;
    }
    else if(startWith(AT_receive_Commond,rec_buf)){
        *type = AT_receive;
        i=_len_(AT_receive_Commond);
        msg = (uint8_t *)malloc(rec_len-i);
        for (size_t j = i; j < rec_len; j++) { 
            msg[j-i]= rec_buf[j]; 
        }
        msg[rec_len-i] = '\0';
        return msg;        
    }
    else if(startWith(AT_reply_message(AT_receive),rec_buf)){
        *type = AT_receive;
        i=_len_(AT_reply_message(AT_receive));
        msg = (uint8_t *)malloc(rec_len-i);
        for (size_t j = i; j < rec_len; j++) { 
            msg[j-i]= rec_buf[j]; 
        }
        msg[rec_len-i] = '\0';
        return msg; 
    }
    else if condition(AT_check_for_signal)
    else if condition(AT_check_if_open)
    else if condition(AT_close)
    else if condition(AT_open)
    else if condition(AT_reset_IP)
    else if condition(AT_check_if_connected)
    else if condition(AT_detach_from_GPRS)
    else if condition(AT_attach_to_GPRS)
    else if condition(AT_enable_single_connection)
    else if condition(AT_enable_multi_connection)
    else if condition(AT_ping)
    else if condition(AT_bring_up_wireless_connection)
    else if condition(AT_get_local_IP_address)
    else if (startWith(AT_start_connection_Commond, rec_buf))
        {
            *type = 15;
            msg = (uint8_t *)malloc(rec_len - 2 - i);
            i = _len_(AT_reply_message(AT_start_connection));
            for (size_t j = i; j < rec_len - 2; j++)
            {
                msg[j - i] = rec_buf[j];
            }
            msg[rec_len - 2 - i] = '\0';
            return msg;
        }
    else if(startWith(AT_request_initiation_for_data_sending_Commond,rec_buf)){
        
        *type = AT_request_initiation_for_data_sending; 
        i = 0;
        msg = (uint8_t *)malloc(rec_len-i);
        for (size_t j = i; j < rec_len; j++) { 
            msg[j-i]= rec_buf[j]; 
        }
        msg[rec_len-i] = '\0';
        return msg;
    }else if(endWithWithLen("SEND FAIL\r\n", rec_buf, rec_len)){
        *type = AT_request_data_sending_error; 
        msg = (uint8_t *)malloc(1);
        msg[0] = 'e';
        for (size_t i = 0; i < rec_len; i++)
        {
            if(rec_buf[i]>='0'&&rec_buf[i]<='9'){
                msg[0] = rec_buf[i];
                break;
            }
        }
        return msg;
    }else if(endWithWithLen("SEND OK\r\n",rec_buf, rec_len)||endWithWithLen("SEND OK\r\n\r\n",rec_buf, rec_len)){
        *type = AT_request_data_sending_success; 
        msg = (uint8_t *)malloc(rec_len);
        for (size_t j = 0; j < rec_len; j++) { 
            msg[j]= rec_buf[j]; 
        }
        msg[rec_len] = '\0';
        return msg;
    }else if(endWithWithLen(", CONNECT OK\r\n",rec_buf, rec_len)){
        *type = AT_network_connnect;
        msg = (uint8_t *)malloc(1);
        msg[0] = 'e';
        for (size_t i = 0; i < rec_len; i++)
        {
            if(rec_buf[i]>='0'&&rec_buf[i]<='9'){
                msg[0] = rec_buf[i];
                break;
            }
        }
        return msg;
    }else if(endWithWithLen(", CONNECT FAIL\r\n",rec_buf, rec_len)){
        *type = AT_network_fail;
        msg = (uint8_t *)malloc(1);
        msg[0] = 'e';
        for (size_t i = 0; i < rec_len; i++)
        {
            if(rec_buf[i]>='0'&&rec_buf[i]<='9'){
                msg[0] = rec_buf[i];
                break;
            }
        }
        return msg;
    }
    else if(endWithWithLen(", CLOSED\r",rec_buf, rec_len)||endWithWithLen(", CLOSED\r\r\n",rec_buf, rec_len)||endWithWithLen(", CLOSED\r\n",rec_buf, rec_len)){
        *type = AT_network_fail;
        msg = (uint8_t *)malloc(1);
        for (size_t i = 0; i < rec_len; i++)
        {
            if(rec_buf[i]>='0'&&rec_buf[i]<='9'){
                msg[0] = rec_buf[i];
                break;
            }
        }
        return msg;
    }else if(startWith("ERROR\r\n",rec_buf)){
        *type = AT_error;
        msg = (uint8_t *)malloc(1);
        return msg;
    }

    else
        return "";
}

#define caseATSend(AT) case AT:\
    return AT##_Commond AT_send_end_Commond

uint8_t* AT_send_message(uint8_t AT_CPOMMOND){
    
    switch (AT_CPOMMOND)
    {
        caseATSend(AT);
        caseATSend(AT_check_for_signal);
        caseATSend(AT_check_if_open);
        caseATSend(AT_close);
        caseATSend(AT_open);
        caseATSend(AT_reset_IP);
        caseATSend(AT_ping);
        caseATSend(AT_check_if_connected);
        caseATSend(AT_detach_from_GPRS);
        caseATSend(AT_attach_to_GPRS);
        caseATSend(AT_enable_single_connection);
        caseATSend(AT_enable_multi_connection);
        caseATSend(AT_bring_up_wireless_connection);
        caseATSend(AT_get_local_IP_address);
        caseATSend(AT_request_initiation_for_data_sending);
    default:
        break;
    }
    return "";
}
#define caseATReply(AT) case AT:\
    return AT##_Commond AT_reply_end_Commond

uint8_t* AT_reply_message(uint8_t AT_CPOMMOND){
    switch (AT_CPOMMOND)
    {
        caseATReply(AT);
        caseATReply(AT_check_for_signal);
        caseATReply(AT_check_if_open);
        caseATReply(AT_close);
        caseATReply(AT_open);
        caseATReply(AT_reset_IP);
        caseATReply(AT_check_if_connected);
        caseATReply(AT_detach_from_GPRS);
        caseATReply(AT_attach_to_GPRS);
        caseATReply(AT_enable_single_connection);
        caseATReply(AT_enable_multi_connection);
        case AT_ping:
            return AT_ping_Commond;
        caseATReply(AT_bring_up_wireless_connection);
        caseATReply(AT_get_local_IP_address);
        caseATReply(AT_start_connection);
        case AT_request_initiation_for_data_sending:
            return AT_request_initiation_for_data_sending_Commond;
        case AT_receive:
            return AT_send_end_Commond AT_receive_Commond;
    default:
        break;
    }
    return "error";
}

uint8_t* AT_ANP(uint8_t* url){
    uint16_t len = _len_(url);
    uint8_t *pingStr;
    pingStr=malloc(sizeof("AT+CSTT=\"\"\r\n")+len);
    uint16_t index=0;
    appendArray("AT+CSTT=\"",pingStr,&index);
    _append(url,len,pingStr,&index);
    appendArray("\"\r\n",pingStr,&index);
    return pingStr;
}

uint8_t* start_multi_channel_connection(uint8_t channel,uint8_t *netWorkType,uint8_t* url,uint16_t port){
     uint16_t lenUrl = _len_(url);
     uint8_t lenNetWorkType = _len_(netWorkType);
     uint16_t size = 1 + (uint8_t)log10(port)+lenNetWorkType+lenUrl+sizeof("AT+CIPSTART=,\"\",\"\",\r\n")+1;
     uint8_t* pingStr;
     pingStr = (uint8_t*)malloc(size);
     uint16_t index = 0;
     appendArray("AT+CIPSTART=" , pingStr, &index);
     appendNumberToStr(channel, pingStr, &index);
     appendArray(",\"" , pingStr, &index);
     _append(netWorkType, lenNetWorkType, pingStr, &index);
     appendArray("\",\"" , pingStr, &index);
     _append(url, lenUrl, pingStr, &index);
     appendArray("\",", pingStr, &index);
     appendNumberToStr(port, pingStr, &index);
     appendArray("\r\n", pingStr, &index);
     pingStr[index] = '\0';
    return pingStr;
}
// AT+CIPSTART=0,"TCP",server.delinapi.top,28888
void sendATCommond(uint8_t AT_CPOMMOND,uart_inst_t *uart){
    sendWithNoReply++;
    uint8_t* AT_msg=AT_send_message(AT_CPOMMOND); 
    sendATCommondMsg(AT_msg,uart);
    // LIST_InsertHeadNode(&listHead, HAL_GetTick(), AT_msg, AT_CPOMMOND, 0);
}

uint8_t SIM_send(Client *client, GPRS_t *gprs, uint8_t *data, uart_inst_t *uart)
{
    uint16_t sendBufLen = _len_(data);
    uint8_t header[19];
    uint16_t headerBufferIndex = 0;
    uint8_t type = 0;
    uint8_t *msg;

    sendTCPHeader(header,client->channel, sendBufLen, &headerBufferIndex);
    client->sendState = sendIdle;
    gprs_flush(gprs);
    uart_write_blocking(uart, header, headerBufferIndex);
    sleep_ms(2000);
    msg = gprs_readBuf(gprs->rec, gprs->recLen, &type);
    AT_Message_Handle(gprs, type, msg);
    gprs_flush(gprs);
    if (strcmp("", msg))
    {
        free(msg);
    }
    if (client->sendState == sendError)
    {
        client->connectState = closed;
        return 0;
    }
    sleep_ms(1000);
    uart_write_blocking(uart, data, sendBufLen);
    sleep_ms(4000);
    msg = gprs_readBuf(gprs->rec, gprs->recLen, &type);
    AT_Message_Handle(gprs, type, msg);
    gprs_flush(gprs);
    if (strcmp("", msg))
    {
        free(msg);
    }
    if (client->sendState == sendError)
    {
        client->connectState = closed;
        return 0;
    }
    return 1;
}


void sendATCommondMsg(uint8_t* AT_msg,uart_inst_t *uart){
    while (handleState == Handle_busy)
    {
        // sleep_ms(100);
        sleep_ms(100);
    }
    sendWithNoReply++;
    #if Debug_send
        sendDebug("\r\n         ");
        sendDebug(AT_msg);
        sendDebug("\r\n");
    #endif
    // Enable_Ux(Ux,_send);
    // sleep_ms(10);
    sleep_ms(10);
    finishedState = 0;
    uart_write_blocking(uart,AT_msg,_len_(AT_msg));
    // HAL_UART_Transmit(&huart,AT_msg,_len_(AT_msg),1000);
    // Enable_Ux(Ux,_rec);
}
int checkSingleATCommond(uint8_t ATx){
    return 0;
    // return LIST_FetchNodeByMsgType(&listHead,ATx,NULL,NULL)>=0? 0:1;    
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
uint8_t containKeyWords(uint8_t* key, uint8_t* msg) {
    uint32_t len = _len_(msg);
    return containKeyWordsWithLen(key, msg, len);
}

uint8_t containKeyWordsWithLen(uint8_t* key, uint8_t* msg,uint32_t len) {
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
