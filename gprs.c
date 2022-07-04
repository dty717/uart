#include "header/gprs.h"
#include "header/common/handler.h"
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

}

// void initNet(uart_inst_t *uart){
// 	if (wait_for_send(upload_device_id_priority)) {
// 		return;
// 	}

//     uint8_t header[18];
//     uint8_t *init = client[0].beatData;
//     uint16_t headerBufferIndex = 0;
//     int _tryTimes = tryTimesConst;

//     okTest:
//         sendATCommondMsg(AT_send_message(AT), U6);
//         AT_state->state = NOState;
//         sleep_ms(100);
//         while (1)
//         {
//             sleep_ms(200);
//             if (AT_state->state == OK)
//             {
//                 break;
//             }
//             else
//             {
//                 if (_tryTimes-- < 0)
//                 {
//                     sleep_ms(500);
//                     if (AT_state->state == OK)
//                     {
//                         break;
//                     }
//                     resetATState();
//                     return 0;
//                 }
//                 else
//                 {
//                     sleep_ms(1000);
//                     goto okTest;
//                 }
//             }
//         }
//     sleep_ms(1000);
//     sendTCPHeader(header, client[0].beatDataLen, &headerBufferIndex);
//     sleep_ms(1000);
//     while (handleState)
//     {
//         sleep_ms(100);
//     }
//     client[0].sendState = sendIdle;
//     HAL_UART_Transmit(&huart6, header, headerBufferIndex, 1000);

//     int tryTimes = tryTimesConst;
//     int waitTimes = 0;
//     while (1)
//     {
//         sleep_ms(100);
//         if (client[0].sendState == sendAble)
//         {
//             sleep_ms(800);
//             break;
//         }
//         else
//         {
//             if (tryTimes-- < 0)
//             {
//                 sleep_ms(200);
//                 wait_delay(10, 200);
//                 if (client[0].sendState == sendAble)
//                 {
//                     break;
//                 }
//                 if(tryTimes<-50){
//                     break;
//                 }
//             }
//         }
//     }
//     sendMode = 1;
//     #ifdef Debug
//         #if Debug_send
//             sendDebugWithLen(header, headerBufferIndex);
//         #endif
//     #endif
//     recBufDelayTimes = 600;

//     while (handleState)
//     {
//         sleep_ms(100);
//     }
//     sleep_ms(200);

//     HAL_UART_Transmit(&huart6, init, client[0].beatDataLen + 2, 1000);
//     if (client[0].sendState == sendError)
//     {
//         return 0;
//     }
//     if(uploadInitState == 2){
//         return 0;
//     }
//     recBufDelayTimes = 1000;
//     sleep_ms(200);
//     client[0].sendState = sendIdle;
//     wait_delay(10, 2000);
//     sendMode = 0;
//     if(uploadInitState == 2){
//         return 0;
//     }

// #ifdef Debug
// #if Debug_send
//     HAL_UART_Transmit(&huart5, init, client[0].beatDataLen + 2, 1000);
// #endif
// #endif

//     resetWithBit(upload_device_info_priority);
// 	resetWithBit(upload_device_id_priority);

//     if (client[0].sendState == sendError)
//     {
//         return 0;
//     }
//     return 1;

// }

void initATState(GPRS_t *gprs){
}
void resetATState(GPRS_t *gprs){
}

uint8_t sendTCPHeader(uint8_t *header,uint16_t len,uint16_t *headerBufferIndex){
    // appendArray(AT_request_initiation_for_data_sending_Commond "=",header,headerBufferIndex);
    // appendNumberToStr(client[0].channel,header,headerBufferIndex);
    // appendArray(",",header,headerBufferIndex);
    // appendNumberToStr(len,header,headerBufferIndex);
    // appendArray("\r\n",header,headerBufferIndex);
    // header[*headerBufferIndex]='\0';
}
uint8_t AT_Message_Handle(GPRS_t *gprs, uint8_t type,uint8_t *msg){
    
}
uint8_t handleReceive(uint8_t *msg){
    
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

uint8_t* clearBuf(uint8_t *rec_buf,uint32_t *recLen,uint8_t *type){
    uint32_t rec_len=*recLen;
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
        i=_len_(AT_request_initiation_for_data_sending);
        msg = (uint8_t *)malloc(rec_len-i);
        for (size_t j = i; j < rec_len; j++) { 
            msg[j-i]= rec_buf[j]; 
        }
        msg[rec_len-i] = '\0';
        return msg;
    }else if(endWith("SEND FAIL\r\n",rec_buf)){
        *type = AT_request_data_sending_error; 
        msg = (uint8_t *)malloc(1);
        msg[0] = 'e\0';
        return msg;
    }else if(endWith(", CONNECT OK\r\n",rec_buf)){
        *type = AT_network_connnect;
        msg = (uint8_t *)malloc(1);
        for (size_t i = 0; i < rec_len; i++)
        {
            if(rec_buf[i]>='0'&&rec_buf[i]<='9'){
                msg[0] = rec_buf[i];
                break;
            }
        }
        return msg;
    }else if(endWith(", CLOSED\r",rec_buf)||endWith(", CLOSED\r\r\n",rec_buf)||endWith(", CLOSED\r\n",rec_buf)){
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
    /*
    if(startWith(AT_reply_message(AT),rec_buf)&&endCheck){
        *type = AT;
        i=sizeof("AT\r\r\n");
        for (size_t j = i; j < rec_len-2; j++)
        {
            msg[j-i]= rec_buf[j];
        }
        return;
    }else if(startWith("AT+CFUN=0\r\r\n",rec_buf)&&endCheck){
        *type = AT_close;
        i=sizeof("AT+CFUN=0\r\r\n");
        for (size_t j = i; j < rec_len-2; j++)
        {
            msg[j-i]= rec_buf[j];
        }
        return;
    }else if(startWith("AT+CFUN=1\r\r\n",rec_buf)&&endCheck){
        *type = AT_open;
        i=sizeof("AT+CFUN=1\r\r\n");
        for (size_t j = i; j < rec_len-2; j++)
        {
            msg[j-i]= rec_buf[j];
        }
        return;
    }else if(startWith("AT+CSQ\r\r\n",rec_buf)&&endCheck){
        *type = AT_open;
        i=sizeof("AT+CSQ\r\r\n");
        for (size_t j = i; j < rec_len-2; j++)
        {
            msg[j-i]= rec_buf[j];
        }
        return;
    }
    */
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

uint8_t* AT_Ping(uint8_t* url){
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

void sendATCommond(uint8_t AT_CPOMMOND,uart_inst_t *uart){
    sendWithNoReply++;
    uint8_t* AT_msg=AT_send_message(AT_CPOMMOND); 
    sendATCommondMsg(AT_msg,uart);
    // LIST_InsertHeadNode(&listHead, HAL_GetTick(), AT_msg, AT_CPOMMOND, 0);
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
                    if (j == startIndex) {
                        startIndex++;
                    }
                    else {
                        break;
                    }
                }
            }
            if (j + msg_skip_spaceShift == endIndex && j - startIndex + key_skip_spaceShift == keyLen) {
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
                    return 1;
                }
            }
        }
    }
    return 0;
}
