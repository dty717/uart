#include "header/modbusRTU.h"
#include "header/crc.h"

/* Define the slave ID of the remote device to talk in master mode or set the
 * internal slave ID in slave mode */
static int _modbus_set_slave(modbus_t *ctx, int slave)
{
    /* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
    if (slave >= 0 && slave <= 247) {
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/* Builds a RTU request header */
static int _rtu_modbus_build_request_basis(modbus_t *ctx, int function,
                                           int addr, int nb,
                                           uint8_t *req)
{
    assert(ctx->slave != -1);
    req[0] = ctx->slave;
    req[1] = function;
    req[2] = addr >> 8;
    req[3] = addr & 0x00ff;
    req[4] = nb >> 8;
    req[5] = nb & 0x00ff;

    return _rtu_modbus_PRESET_REQ_LENGTH;
}

static int _rtu_modbus_flush(modbus_t *ctx){
    modbus_rtu_t *ctx_rtu = ctx->backend_data;
    ctx_rtu->chars_rxed = 0;
    ctx_rtu->chars_rx_index = 0;
    ctx_rtu->chars_rx_server_index = 0;
    return 1;
}


/* Builds a RTU response header */
static int _rtu_modbus_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    /* In this case, the slave is certainly valid because a check is already
     * done in _rtu_modbus_listen */
    rsp[0] = sft->slave;
    rsp[1] = sft->function;

    return _rtu_modbus_PRESET_RSP_LENGTH;
}


static int _rtu_modbus_prepare_response_tid(const uint8_t *req, int *req_length)
{
    (*req_length) -= _rtu_modbus_CHECKSUM_LENGTH;
    /* No TID */
    return 0;
}

static int _rtu_modbus_send_msg_pre(uint8_t *req, int req_length)
{
    uint16_t crc = crc16(req, 0, req_length);
    req[req_length++] = crc >> 8;
    req[req_length++] = crc & 0x00FF;

    return req_length;
}

static ssize_t _rtu_modbus_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    modbus_rtu_t *ctx_rtu = ctx->backend_data;
    gpio_put(ctx_rtu->uart_en_pin, 1);
    _rtu_modbus_flush(ctx);
    sleep_us(50);
    if (uart_is_writable(ctx_rtu->uart))
    {
        uart_write_blocking(ctx_rtu->uart,req,req_length);
    }else{
        if(ctx->debug){
            printf("err:uart%s not writable\n",ctx_rtu->device);
        }
    }
    if (ctx->debug)
    {
        printf("uart baud:%d,req_length:%d\n", ctx_rtu->baud,req_length);
    }
    sleep_us(5152 * 1000 / ctx_rtu->baud * (req_length + 1));
    gpio_put(ctx_rtu->uart_en_pin, 0);
    // sleep_ms(2);
    return req_length;

	// return write(ctx->s, req, req_length);
}

static int _rtu_modbus_receive(modbus_t *ctx, uint8_t *req)
{
    int rc;
    modbus_rtu_t *ctx_rtu = ctx->backend_data;
    
    if (ctx_rtu->confirmation_to_ignore) {
        _modbus_receive_msg(ctx, req, MSG_CONFIRMATION);
        /* Ignore errors and reset the flag */
        ctx_rtu->confirmation_to_ignore = FALSE;
        rc = 0;
        if (ctx->debug) {
            printf("Confirmation to ignore\n");
        }
    } else {
        rc = _modbus_receive_msg(ctx, req, MSG_INDICATION);
        if (rc == 0&&ctx->debug) {
            printf("MSG_INDICATION\r\n");
            /* The next expected message is a confirmation to ignore */
            ctx_rtu->confirmation_to_ignore = TRUE;
        }
    }
    if(ctx_rtu->isServer){
        if(rc>0){
            ctx_rtu->chars_rx_server_index += rc;
        }else{
            if(ctx_rtu->chars_rx_server_index+1<ctx_rtu->chars_rxed){
              ctx_rtu->chars_rx_server_index++;
            }
        }
        if( ctx_rtu->chars_rx_server_index>800){
            for (size_t i = ctx_rtu->chars_rx_server_index; i < ctx_rtu->chars_rxed; i++)
            {
                ctx_rtu->ch[i - ctx_rtu->chars_rx_server_index] = ctx_rtu->ch[i];
            }
            // ctx_rtu->chars_rx_server_index-=600;
            // ctx_rtu->chars_rxed-=600;
            ctx_rtu->chars_rxed -= ctx_rtu->chars_rx_server_index;
            ctx_rtu->chars_rx_server_index = 0;
        }
        ctx_rtu->chars_rx_index = ctx_rtu->chars_rx_server_index;
    }
    return rc;
}

static ssize_t _rtu_modbus_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    modbus_rtu_t *ctx_rtu = ctx->backend_data;
    int8_t waitTimes = ctx_rtu->isServer?MODBUS_RTU_MAX_RESPONSE_TIME:MODBUS_RTU_MIN_RESPONSE_TIME;

    while((ctx_rtu->chars_rxed-ctx_rtu->chars_rx_index)<rsp_length){
        if (!ctx_rtu->isServer) {
            waitTimes--;
            sleep_ms(MODBUS_RTU_MAX_RESPONSE_TIME*rsp_length);
            if (waitTimes <= 0)
            {
                ctx_rtu->chars_rxed = 0;
                ctx_rtu->chars_rx_index = 0;
                return -1;
            }
        }else{
            waitTimes--;
            sleep_ms(MODBUS_RTU_MIN_RESPONSE_TIME*rsp_length);
            if (waitTimes <= 0)
            {
                return -1;
            }
        }
    }
    for (size_t i = 0; i < rsp_length; i++)
    {
        rsp[i] = ctx_rtu->ch[i+ctx_rtu->chars_rx_index];
    }
    // ctx_rtu->chars_rxed = 0;
    ctx_rtu->chars_rx_index += rsp_length;
    return rsp_length;//read(ctx->s, rsp, rsp_length);
}

static ssize_t _rtu_modbus_reRecv(modbus_t *ctx, uint8_t *rsp, int rsp_length, int rsp_shift)
{
    modbus_rtu_t *ctx_rtu = ctx->backend_data;

    for (size_t i = 0; i < rsp_length; i++)
    {
        rsp[i] = ctx_rtu->ch[i + ctx_rtu->chars_rx_index - rsp_shift];
    }
    // ctx_rtu->chars_rxed = 0;
    ctx_rtu->chars_rx_index += rsp_length - rsp_shift;
    return rsp_length; // read(ctx->s, rsp, rsp_length);
}

static int _rtu_modbus_pre_check_confirmation(modbus_t *ctx, const uint8_t *req,
                                              const uint8_t *rsp, int rsp_length)
{
    /* Check responding slave is the slave we requested (except for broacast
     * request) */
    if (req[0] != rsp[0] && req[0] != MODBUS_BROADCAST_ADDRESS) {
        if (ctx->debug) {
            printf("err:The responding slave %d isn't the requested slave %d\n",
                    rsp[0], req[0]);
        }
        errno = EMBBADSLAVE;
        return -1;
    } else {
        return 0;
    }
}

/* The check_crc16 function shall return 0 if the message is ignored and the
   message length if the CRC is valid. Otherwise it shall return -1 and set
   errno to EMBBADCRC. */
static int _rtu_modbus_check_integrity(modbus_t *ctx, uint8_t *msg,
                                       const int msg_length)
{
    uint16_t crc_calculated;
    uint16_t crc_received;
    int slave = msg[0];

    /* Filter on the Modbus unit identifier (slave) in RTU mode to avoid useless
     * CRC computing. */
    if (slave != ctx->slave && slave != MODBUS_BROADCAST_ADDRESS) {
        if (ctx->debug) {
            printf("Request for slave %d ignored (not %d)\n", slave, ctx->slave);
        }
        /* Following call to check_confirmation handles this error */
        return -1;
    }

    crc_calculated = crc16(msg, 0,msg_length - 2);
    crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];
    //delete test
    // if(1){
    //     return msg_length;
    // }
    /* Check CRC of msg */
    if (crc_calculated == crc_received) {
        return msg_length;
    } else {
        if (ctx->debug) {
            printf("err:ERROR CRC received 0x%0X != CRC calculated 0x%0X\n",
                    crc_received, crc_calculated);
        }

        if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
            _rtu_modbus_flush(ctx);
        }
        errno = EMBBADCRC;
        return -1;
    }
}


/* Sets up a serial port for RTU communications */
static int _rtu_modbus_connect(modbus_t *ctx)
{
    int flags;
    modbus_rtu_t *ctx_rtu = ctx->backend_data;

    if (ctx->debug) {
        printf("Opening %s at %d bauds (%c, %d, %d)\n",
               ctx_rtu->device, ctx_rtu->baud, ctx_rtu->parity,
               ctx_rtu->data_bit, ctx_rtu->stop_bit);
    }
    return 0;
}

static void _rtu_modbus_close(modbus_t *ctx)
{
    /* Restore line settings and close file descriptor in RTU mode */
    modbus_rtu_t *ctx_rtu = ctx->backend_data;


    if (ctx->s != -1) {
        // tcsetattr(ctx->s, TCSANOW, &ctx_rtu->old_tios);
        // close(ctx->s);
        ctx->s = -1;
    }
}

static int _rtu_modbus_select(modbus_t *ctx, fd_set *rset,
                              struct timeval *tv, int length_to_read)
{
    // int s_rc;
    // while ((s_rc = select(ctx->s+1, rset, NULL, NULL, tv)) == -1) {
    //     if (errno == EINTR) {
    //         if (ctx->debug) {
    //             printf("err:A non blocked signal was caught\n");
    //         }
    //         /* Necessary after an error */
    //         FD_ZERO(rset);
    //         FD_SET(ctx->s, rset);
    //     } else {
    //         return -1;
    //     }
    // }

    int s_rc=1;
    if (s_rc == 0) {
        /* Timeout */
        errno = ETIMEDOUT;
        return -1;
    }

    return s_rc;
}


static void _rtu_modbus_free(modbus_t *ctx) {
    if (ctx->backend_data) {
        free(((modbus_rtu_t *)ctx->backend_data)->device);
        free(ctx->backend_data);
    }

    free(ctx);
}

static int _rtu_modbus_add_RXData(modbus_t *ctx, uint8_t ch) {
    if (ctx->backend_data) {
        modbus_rtu_t  *ctx_rtu = (modbus_rtu_t *)ctx->backend_data;
        ctx_rtu->ch[ctx_rtu->chars_rxed] = ch;
        ctx_rtu->chars_rxed++;
        // ctx_rtu->ch[ctx_rtu->chars_rxed] = '\0';
        if(ctx_rtu->chars_rxed==1024){
            ctx_rtu->chars_rxed=0;
            ctx_rtu->chars_rx_index = 0;
        }
        return 1;
    }
    return 0;
}

static int _rtu_modbus_header(modbus_t *ctx, uint8_t *msg) {
    return msg[0];
}

const modbus_backend_t _rtu_modbus_backend = {
    _MODBUS_BACKEND_TYPE_RTU,
    _rtu_modbus_HEADER_LENGTH,
    _rtu_modbus_CHECKSUM_LENGTH,
    MODBUS_RTU_MAX_ADU_LENGTH,
    _modbus_set_slave,
    _rtu_modbus_build_request_basis,
    _rtu_modbus_build_response_basis,
    _rtu_modbus_prepare_response_tid,
    _rtu_modbus_send_msg_pre,
    _rtu_modbus_send,
    _rtu_modbus_receive,
    _rtu_modbus_recv,
    _rtu_modbus_reRecv,
    _rtu_modbus_check_integrity,
    _rtu_modbus_pre_check_confirmation,
    _rtu_modbus_connect,
    _rtu_modbus_close,
    _rtu_modbus_flush,
    _rtu_modbus_select,
    _rtu_modbus_free,
    _rtu_modbus_add_RXData,
    _rtu_modbus_header
};


modbus_t* modbus_new_rtu(const char *device,
                         int baud, char parity, int data_bit,
                         int stop_bit,uart_inst_t *uart,uint8_t uart_en_pin,uint8_t isServer)
{
    modbus_t *ctx;
    modbus_rtu_t *ctx_rtu;
    printf("new modbusRTU\r\n");
    /* Check device argument */
    if (device == NULL || *device == 0) {
        printf("err:The device string is empty\n");
        errno = EINVAL;
        return NULL;
    }

    /* Check baud argument */
    if (baud == 0) {
        printf("err:The baud rate value must not be zero\n");
        errno = EINVAL;
        return NULL;
    }

    ctx = (modbus_t *)malloc(sizeof(modbus_t));
    if (ctx == NULL) {
        return NULL;
    }

    _modbus_init_common(ctx);
    ctx->backend = &_rtu_modbus_backend;
    ctx->backend_data = (modbus_rtu_t *)malloc(sizeof(modbus_rtu_t));

    if (ctx->backend_data == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }


    ctx_rtu = (modbus_rtu_t *)ctx->backend_data;

    /* Device name and \0 */
    ctx_rtu->device = (char *)malloc((strlen(device) + 1) * sizeof(char));
    ctx_rtu->uart = uart;
    ctx_rtu->uart_en_pin = uart_en_pin;
    ctx_rtu->ch = malloc(1024);
    ctx_rtu->ch[0] = '\0';
    ctx_rtu->chars_rxed = 0;
    ctx_rtu->chars_rx_index = 0;
    ctx_rtu->chars_rx_server_index = 0;
    ctx_rtu->isServer = isServer;
    
    if (ctx_rtu->device == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    strcpy(ctx_rtu->device, device);

    ctx_rtu->baud = baud;
    if (parity == 'N' || parity == 'E' || parity == 'O') {
        ctx_rtu->parity = parity;
    } else {
        modbus_free(ctx);
        errno = EINVAL;
        return NULL;
    }
    ctx_rtu->data_bit = data_bit;
    ctx_rtu->stop_bit = stop_bit;

#if HAVE_DECL_TIOCSRS485
    /* The RS232 mode has been set by default */
    ctx_rtu->serial_mode = MODBUS_RTU_RS232;
#endif

#if HAVE_DECL_TIOCM_RTS
    /* The RTS use has been set by default */
    ctx_rtu->rts = MODBUS_RTU_RTS_NONE;

    /* Calculate estimated time in micro second to send one byte */
    ctx_rtu->onebyte_time = 1000000 * (1 + data_bit + (parity == 'N' ? 0 : 1) + stop_bit) / baud;

    /* The internal function is used by default to set RTS */
    ctx_rtu->set_rts = _rtu_modbus_ioctl_rts;

    /* The delay before and after transmission when toggling the RTS pin */
    ctx_rtu->rts_delay = ctx_rtu->onebyte_time;
#endif

    ctx_rtu->confirmation_to_ignore = FALSE;

    return ctx;
}