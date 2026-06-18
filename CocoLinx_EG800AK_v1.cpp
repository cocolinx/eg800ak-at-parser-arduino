#include "CocoLinx_EG800AK_v1.h"

CocoLinx_EG800AK::CocoLinx_EG800AK() { }

// Customize this part according to your MCU or OS.
void CocoLinx_EG800AK::uart_write(const uint8_t *data, size_t size)
{
    Serial1.write(data, size);
}

int32_t CocoLinx_EG800AK::uart_read(uint8_t *buf, size_t bufsize)
{
    int cnt = 0;
    while(Serial1.available())
    {
        buf[cnt++] = Serial1.read();
        if(cnt >= bufsize) break;
    }
    return cnt;
}

void CocoLinx_EG800AK::uart_flush()
{
    while(Serial1.available()) Serial1.read();
}

uint32_t CocoLinx_EG800AK::get_ms()
{
    return millis();
}

void CocoLinx_EG800AK::sleep_ms(uint32_t ms)
{
    delay(ms);
}

bool CocoLinx_EG800AK::begin()
{
    Serial1.begin(115200);
    sleep_ms(500);

    at_parser_init();

    memset(_pktbuf, 0, sizeof(_pktbuf));
  
    uint32_t start_ms = get_ms();

    while((get_ms() - start_ms) <= 5000)
    {
        transfer_pkt("AT", 16, 1000);
        if(_parser.resp_code == AT_OK) return true;
        sleep_ms(500);
    }

    return false;
}



void CocoLinx_EG800AK::at_parser_init()
{
	memset(&_parser, 0, sizeof(AtParser));
	_parser.at = _pktbuf;
    _parser.resp_code = AT_ERROR;
}


void CocoLinx_EG800AK::trim_cr_lf(const char **str)
{
	while ((*str)[0] == CR || (*str)[0] == LF) {
		(*str)++;
	}
}

void CocoLinx_EG800AK::trim_left_space(const char **str)
{
    while ((*str)[0] == SPACE) {
        (*str)++;
    }
}

void CocoLinx_EG800AK::trim_quote(const char **str, int32_t *len)
{
    if (str == nullptr || len == nullptr) return;
    if (*str == nullptr) return;
    if (*len < 2) return;

    if ((*str)[0] == QUOTE) {
        (*str)++;
        (*len)--;
    }

    if ((*str)[*len - 1] == QUOTE) {
        (*len)--;
    }
}

bool CocoLinx_EG800AK::is_resp(const char *str)
{
    trim_cr_lf(&str);

    if(str[0] == NULL_TERMINATOR) return false;

    if (strncmp(str, "OK\r\n", 4) == 0) {
        return true;
    }

    if (strncmp(str, "ERROR\r\n", 7) == 0) {
        return true;
    }

    if (strncmp(str, "+CME ERROR:", 11) == 0) {
        return true;
    }

    if (strncmp(str, "+CMS ERROR:", 11) == 0) {
        return true;
    }

    return false;
}


bool CocoLinx_EG800AK::is_end_response(const char *str)
{
    return str[0] == CR && str[1] == LF && str[2] == NULL_TERMINATOR;
}

bool CocoLinx_EG800AK::is_end_line(const char *str)
{
	return str[0] == CR && str[1] == LF;
}

bool CocoLinx_EG800AK::is_colon_or_comma(const char *str)
{
    return str[0] == COLON || str[0] == COMMA;
}

bool CocoLinx_EG800AK::is_quote(const char *str)
{
    return str[0] == QUOTE;
}

int32_t CocoLinx_EG800AK::find_prefix_token(const char *str, int32_t start_idx)
{
    if(str == nullptr) return -1;

    int32_t count = _parser.count;

    for(int32_t idx = start_idx; idx < count; idx++)
    {
        if(strncmp(_parser.tokens[idx].buf, str, _parser.tokens[idx].len) == 0) return idx;
    }

    return -1;
}

// send -> recv
// return ack code(negative sign)
int32_t CocoLinx_EG800AK::transfer_pkt(const uint8_t *cmd, int32_t cmd_size, int32_t max_tokens, uint32_t timeout_ms)
{
    if(cmd == nullptr) return -(ACK_ERR_ARG);
    if(max_tokens > TOKEN_SIZE_MAX) return -(ACK_ERR_ARG);

    if(timeout_ms < 1000) timeout_ms = 1000;

    sleep_ms(10);

    // _parser init
    at_parser_init();

    // flush rx buffer if exist
    uart_flush();

    // send at command
    uart_write(cmd, cmd_size);
    uart_write((const uint8_t *)"\r\n", 2);

    // read at response
    uint32_t rxcnt = 0;    
    bool rxdone = false;
    uint32_t start_ms = get_ms();
    uint32_t idx;

    while((get_ms() - start_ms) <= timeout_ms)
    {
        if(rxdone == true) break;

        rxcnt += uart_read((uint8_t *)&_pktbuf[rxcnt], RESPONSE_DATA_SIZE_MAX - rxcnt);
        if(rxcnt == 0) continue;

        if(rxcnt >= RESPONSE_DATA_SIZE_MAX) return -(ACK_ERR_RESP_OVERFLOW);

        if(_pktbuf[rxcnt-1] == LF)
        {
            if(rxcnt >= 6)
                if(strncmp(&_pktbuf[rxcnt-6], "\r\nOK\r\n", 6) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_OK;
                    rxdone = true;
                    break;
                }
            if(rxcnt >= 9)
                if(strncmp(&_pktbuf[rxcnt-9], "\r\nERROR\r\n", 9) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_ERROR;
                    rxdone = true;
                    break;
                }
            if(rxcnt >= 13) 
            {
                idx = 3;
                while(idx <= rxcnt)
                {
                    if(_pktbuf[rxcnt-idx] == LF) break;
                    if(_pktbuf[rxcnt-idx] == '+') 
                    {
                        if(strncmp(&_pktbuf[rxcnt-idx], "\r\n+CME ERROR:", 13) == 0) 
                        {
                           _pktbuf[rxcnt] = '\0';
                            _parser.resp_code = AT_CME_ERROR;
                            rxdone = true;
                            break;
                        }
                        if(strncmp(&_pktbuf[rxcnt-idx], "\r\n+CMS ERROR:", 13) == 0)
                        {
                            _pktbuf[rxcnt] = '\0';
                            _parser.resp_code = AT_CMS_ERROR;
                            rxdone = true;
                            break;
                        }
                    }
                    idx++;
                }
            }
        }
    }

    if(rxdone == false) return -(ACK_ERR_TIMEOUT);

    if(rxcnt > cmd_size) return at_pkt_parser(max_tokens, cmd, cmd_size);
    return at_pkt_parser(max_tokens, NULL, cmd_size);
}

int32_t CocoLinx_EG800AK::transfer_pkt(const char *cmd, int32_t max_tokens, uint32_t timeout_ms)
{
    return transfer_pkt((uint8_t *)cmd, strlen(cmd), max_tokens, timeout_ms);
}

int32_t CocoLinx_EG800AK::transfer_pkt_data(const char *cmd, int32_t cmd_size, const uint8_t *data, int32_t data_size, uint32_t timeout_ms)
{
    if(data == nullptr) return -(ACK_ERR_ARG);

    if(timeout_ms < 1000) timeout_ms = 1000;

    sleep_ms(10);

    // _parser init
    at_parser_init();

    // flush rx buffer if exist
    uart_flush();

    // send cmd
    uart_write((const uint8_t *)cmd, cmd_size);
    uart_write((const uint8_t *)"\r\n", 2);

    sleep_ms(100);

    // send data
    uart_write(data, data_size);

    uint32_t rxcnt = 0;    
    bool rxdone = false;
    uint32_t start_ms = get_ms();

    while((get_ms() - start_ms) <= timeout_ms)
    {
        if(rxdone == true) break;

        rxcnt += uart_read((uint8_t *)&_pktbuf[rxcnt], RESPONSE_DATA_SIZE_MAX - rxcnt);
        if(rxcnt == 0) continue;

        if(rxcnt >= RESPONSE_DATA_SIZE_MAX) return -(ACK_ERR_RESP_OVERFLOW);

        if(_pktbuf[rxcnt-1] == LF)
        {
            if(rxcnt >= 6)
            {
                if(strncmp(&_pktbuf[rxcnt-6], "\r\nOK\r\n", 6) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_OK;
                    rxdone = true;
                    break;
                }
            }
            if(rxcnt >= 9)
            {
                if(strncmp(&_pktbuf[rxcnt-9], "\r\nERROR\r\n", 9) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_ERROR;
                    rxdone = true;
                    break;
                }
            }
            if(rxcnt >= 11) 
            {
                if(strncmp(&_pktbuf[rxcnt-11], "\r\nSEND OK\r\n", 11) == 0) 
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_OK;
                    rxdone = true;
                    break;
                }
            }
            if(rxcnt >= 13)
            {
                if(strncmp(&_pktbuf[rxcnt-13], "\r\nSEND FAIL\r\n", 13) == 0)
                {
                    _pktbuf[rxcnt] = '\0';
                    _parser.resp_code = AT_ERROR;
                    rxdone = true;
                    break;
                }
            }
        }
    }

    if(!rxdone) return -(ACK_ERR_TIMEOUT);

    return ACK_OKAY;
}


// ',' ':' 로 파싱
int32_t CocoLinx_EG800AK::at_pkt_parser(int32_t max_tokens, const uint8_t *cmd, int32_t cmd_size)
{
    if(max_tokens <= 0) return -(ACK_ERR_ARG);
    if(max_tokens > TOKEN_SIZE_MAX) return -(ACK_ERR_ARG);

    bool ret;
    uint8_t token_idx = 0;
    int32_t token_len = 0;
    const char *cursor = _parser.at;
    
    trim_cr_lf(&cursor);
    
    // echo trim
    if(cmd != nullptr)
    {
        if(memcmp(cursor, cmd, cmd_size) == 0) // memcmp 로 교체
        {
            cursor += cmd_size;
            trim_cr_lf(&cursor);
        }
    }

    while(*cursor != NULL_TERMINATOR)
    {
        token_len = 0;

        // end line
        if(is_resp(cursor))
        {
            if(_parser.resp_code == AT_CME_ERROR || _parser.resp_code == AT_CMS_ERROR)
            {
                cursor += 11;
                trim_left_space(&cursor);
                _parser.tokens[token_idx].buf = cursor;
                while(!is_end_response(cursor)) 
                {
                    cursor++;
                    token_len++;
                }
                _parser.tokens[token_idx].len = token_len;
                _parser.count++;

                // put at error code
                ret = char_to_int32(&_parser.tokens[token_idx], &_parser.at_error_code);
                if(!ret) return -(ACK_ERR_PARSE);
            }
            break;
        }

        if(token_idx >= max_tokens) return -(ACK_ERR_PARSE);
        // token parsing
        _parser.tokens[token_idx].buf = cursor;

        if(token_idx >= max_tokens - 1) // put all
        {
            while(!is_end_line(cursor))
            {
                cursor++;
                token_len++;
            }
        }
        else
        {
            while(!is_colon_or_comma(cursor) && !is_end_line(cursor))
            {
                cursor++;
                token_len++;
            }
        }

        _parser.tokens[token_idx].len = token_len;
        token_idx++;

        _parser.count++;    
        cursor++;

        trim_left_space(&cursor);
        trim_cr_lf(&cursor);
    }

    return ACK_OKAY;
}

bool CocoLinx_EG800AK::char_to_int32(const char *str, int32_t len, int32_t *resp)
{
    if (str == nullptr || resp == nullptr) return false;
    if (len <= 0) return false;

    int32_t result = 0;
    bool is_negative = false;

    if (*str == '-')
    {
        is_negative = true;
        str++;
        len--;

        if (len <= 0) return false;
    }

    while (len--)
    {
        if (*str >= '0' && *str <= '9')
        {
            result = result * 10 + (*str - '0');
            str++;
        }
        else if(*str == '.') str++;
        else return false;
    }

    *resp = is_negative ? -result : result;

    return true;
}


bool CocoLinx_EG800AK::char_to_int32(const AtToken *tok, int32_t *resp)
{
    if(tok == nullptr) return false;
    return char_to_int32(tok->buf, tok->len, resp);
}

int32_t CocoLinx_EG800AK::get_at_error_ack()
{
    if(_parser.resp_code == AT_ERROR) return ACK_AT_ERROR;
    if(_parser.resp_code == AT_CME_ERROR) return ACK_AT_CME_ERROR;
    if(_parser.resp_code == AT_CMS_ERROR) return ACK_AT_CMS_ERROR;
    return ACK_ERR_PARSE;
}

int32_t CocoLinx_EG800AK::get_at_error_code()
{
    return _parser.at_error_code;
}

/**************************************** AT set command ****************************************/
/* General Commands */
int32_t CocoLinx_EG800AK::set_factory_reset()
{
    char cmd[16];

    strcpy(cmd, "AT&F");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_ate(int32_t value)
{
    char cmd[16];

    snprintf(cmd, 16, "ATE%d", value);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cfun(int32_t fun)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CFUN=%d", fun);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cmee(int32_t n)
{
    char cmd[16];

    if(n >= 2) return -(ACK_ERR_ARG); // 2 not supported

    snprintf(cmd, 16, "AT+CMEE=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Serial Interface Control Commands */
int32_t CocoLinx_EG800AK::set_baudrate(int32_t baudrate)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+IPR=%d", baudrate);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* SIM Related Commands */
int32_t CocoLinx_EG800AK::set_cpin(int32_t code)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CPIN=%d", code);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Network Service Commands */
int32_t CocoLinx_EG800AK::set_cops(int32_t oper)
{
    char cmd[32];

    snprintf(cmd, 32, "AT+COPS=1,2,\"%d\"", oper);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_auto_cops()
{
    char cmd[16];

    strcpy(cmd, "AT+COPS=0");

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_creg(int32_t n)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CREG=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_rtc_auto_local_time()
{
    char cmd[16];

    strcpy(cmd, "AT+CTZU=3");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Packet Domain Commands */
int32_t CocoLinx_EG800AK::set_cgatt(int32_t state)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CGATT=%d", state);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 60);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cgdcont(int32_t cid, char *pdp_type, char *apn)
{
    if(pdp_type == nullptr || apn == nullptr) return -(ACK_ERR_ARG);

    char cmd[64];

    snprintf(cmd, 64, "AT+CGDCONT=%d,\"%s\",\"%s\"", cid, pdp_type, apn);

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cgact(int32_t state, int32_t cid)
{
    char cmd[32];

    snprintf(cmd, 32, "AT+CGACT=%d,%d", state, cid);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 60 * 3);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cereg(int32_t n)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+CEREG=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* Hardware Related Commands */
int32_t CocoLinx_EG800AK::set_qpowd(int32_t n)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+QPOWD=%d", n);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::set_cclk(char *time)
{
    if(time == nullptr) return -(ACK_ERR_ARG);

    char cmd[64];

    snprintf(cmd, 64, "AT+CCLK=\"%s\"", time);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/* MQTT Related AT Commands */
int32_t CocoLinx_EG800AK::cx_mqtt_cfg(const char *config_key, int32_t *params, int32_t param_count)
{
    if(config_key == nullptr) return -(ACK_ERR_ARG);
    if(params == nullptr && param_count > 0) return -(ACK_ERR_ARG);
    if(param_count <= 0) return -(ACK_ERR_ARG);

    char cmd[128];
    int32_t offset = 0;

    offset = snprintf(cmd, 128, "AT+QMTCFG=\"%s\"", config_key);

    if(offset < 0) return -(ACK_ERR_ARG);

    for(int32_t i = 0; i < param_count; i ++)
    {
        int32_t n = snprintf(&cmd[offset], 128 - offset, ",%d", params[i]);
        if(n < 0 || (offset + n) >= 128) return -(ACK_ERR_ARG);
        offset += n;
    }

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_cfg_version(int32_t client_idx, int32_t vsn)
{
    int32_t params[] = {client_idx, vsn};
    return cx_mqtt_cfg("version", params, 2);
}

int32_t CocoLinx_EG800AK::cx_mqtt_cfg_pcpcid(int32_t client_idx, int32_t cid)
{
    int32_t params[] = {client_idx, cid};
    return cx_mqtt_cfg("pdpcid", params, 2);
}

int32_t CocoLinx_EG800AK::cx_mqtt_cfg_keepalive(int32_t client_idx, int32_t keep_alive_time)
{
    int32_t params[] = {client_idx, keep_alive_time};
    return cx_mqtt_cfg("keepalive", params, 2);
}

int32_t CocoLinx_EG800AK::cx_mqtt_cfg_session(int32_t client_idx, int32_t clean_session)
{
    int32_t params[] = {client_idx, clean_session};
    return cx_mqtt_cfg("session", params, 2);
}

int32_t CocoLinx_EG800AK::cx_mqtt_cfg_recvmode(int32_t client_idx, int32_t recv_mode, int32_t len_enable)
{
    int32_t params[] = {client_idx, recv_mode, len_enable};
    return cx_mqtt_cfg("recv/mode", params, 3);
}

int32_t CocoLinx_EG800AK::cx_mqtt_open(int32_t client_idx, char *host_name, int32_t port)
{
    char cmd[128];

    snprintf(cmd, 128, "AT+QMTOPEN=%d,\"%s\",%d", client_idx, host_name, port);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 60 * 2);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_close(int32_t client_idx)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+QMTCLOSE=%d", client_idx);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_conn(int32_t client_idx, char *client_id, char *user_name, char *password)
{
    char cmd[128];

    if(user_name == nullptr) snprintf(cmd, 128, "AT+QMTCONN=%d,\"%s\"", client_idx, client_id);
    else snprintf(cmd, 128, "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", client_idx, client_id, user_name, password);

    int32_t ack = transfer_pkt(cmd, 16, 10000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_disconn(int32_t client_idx)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+QMTDISC=%d", client_idx);

    int32_t ack = transfer_pkt(cmd, 16, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_sub(int32_t client_idx, int32_t msg_id, const char *topic, int32_t qos)
{
    char cmd[128];

    snprintf(cmd, 128, "AT+QMTSUB=%d,%d,\"%s\",%d", client_idx, msg_id, topic, qos);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    
    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_unsub(int32_t client_idx, int32_t msg_id, const char *topic)
{
    char cmd[128];

    if(topic == nullptr) return -(ACK_ERR_ARG);
    
    snprintf(cmd, 128, "AT+QMTUNS=%d,%d,\"%s\"", client_idx, msg_id, topic);

    int32_t ack = transfer_pkt(cmd, 16, 20000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    
    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_pub(int32_t client_idx, int32_t msg_id, int32_t qos, int32_t retain, const char *topic, const uint8_t *msg, int32_t length)
{
    char cmd[256];

    snprintf(cmd, 256, "AT+QMTPUBEX=%d,%d,%d,%d,\"%s\",%d", client_idx, msg_id, qos, retain, topic, length);

    int32_t ack = transfer_pkt_data(cmd, strlen(cmd), msg, length, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_pub(int32_t client_idx, int32_t msg_id, int32_t qos, int32_t retain, const char *topic, const char *msg, int32_t length)
{
    return cx_mqtt_pub(client_idx, msg_id, qos, retain, topic, (const uint8_t *)msg, length);
}

/* Socket AT commands */
int32_t CocoLinx_EG800AK::cx_socket_cfg(const char *config_key, int32_t *params, int32_t param_count)
{
    if(config_key == nullptr) return -(ACK_ERR_ARG);
    if(params == nullptr && param_count > 0) return -(ACK_ERR_ARG);
    if(param_count <= 0) return -(ACK_ERR_ARG);

    char cmd[128];
    int32_t offset = 0;

    offset = snprintf(cmd, 128, "AT+QICFG=\"%s\"", config_key);

    if(offset < 0) return -(ACK_ERR_ARG);

    for(int32_t i = 0; i < param_count; i ++)
    {
        int32_t n = snprintf(&cmd[offset], 128 - offset, ",%d", params[i]);
        if(n < 0 || (offset + n) >= 128) return -(ACK_ERR_ARG);
        offset += n;
    }

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_socket_cfg_data_format(int32_t send_data_format, int32_t recv_data_format)
{
    int32_t params[] = {send_data_format, recv_data_format};
    return cx_socket_cfg("dataformat", params, 2);
}

int32_t CocoLinx_EG800AK::cx_socket_cfg_tcp_keepalive(int32_t enable, int32_t idle_time, int32_t interval_time, int32_t probe_cnt)
{
    int32_t params[] = {enable, idle_time, interval_time, probe_cnt};
    return cx_socket_cfg("tcp/keepalive", params, 4);
}

int32_t CocoLinx_EG800AK::cx_socket_cfg_send_info(int32_t send_view_mode)
{
    int32_t params[] = {send_view_mode};
    return cx_socket_cfg("sendinfo", params, 1);
}

int32_t CocoLinx_EG800AK::cx_set_context(int32_t context_id, int32_t context_type, const char *apn, const char *username, const char *password, int32_t authentication, int32_t cdma_pwd)
{
    char cmd[400];

    snprintf(cmd, 400, "AT+QICSGP=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d", context_id, context_type, apn, 
        (username == nullptr)? "" : username, (password == nullptr)? "" : password, authentication, cdma_pwd);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_pdp_activate(int32_t context_id)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+QIACT=%d", context_id);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 150);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_pdp_deactivate(int32_t context_id)
{
    char cmd[16];

    snprintf(cmd, 16, "AT+QIDEACT=%d", context_id);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 40);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}
        
int32_t CocoLinx_EG800AK::cx_socket(int32_t context_id, int32_t connect_id, const char *service_type, const char *ip_address, int32_t remote_port)
{
    int32_t val;
    char cmd[64];

    snprintf(cmd, 64, "AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d", context_id, connect_id, service_type, ip_address, remote_port);

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 150); // 150 secs
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_close(int32_t connect_id)
{
    int32_t val;
    char cmd[16];

    snprintf(cmd, 16, "AT+QICLOSE=%d", connect_id);

    int32_t ack = transfer_pkt(cmd, 16, 10000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_send(int32_t connect_id, const uint8_t *data, int32_t send_length)
{
    char cmd[32];

    snprintf(cmd, 32, "AT+QISEND=%d,%d", connect_id, send_length);

    int32_t ack = transfer_pkt_data(cmd, strlen(cmd), data, send_length, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_send(int32_t connect_id, const char *data, int32_t send_length)
{
    return cx_send(connect_id, (const uint8_t *)data, send_length);
}

int32_t CocoLinx_EG800AK::cx_send_hex(int32_t connect_id, const uint8_t *data, int32_t send_length)
{
    char cmd[600];
    char data_str[512];

    int32_t ret = bin_to_hex(data, send_length, data_str, 512);
    if(ret == 0) return -(ACK_ERR_ARG);

    snprintf(cmd, 600, "AT+QISENDEX=%d,\"%s\"", connect_id, data_str);

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());

    return ACK_OKAY;
}

/**************************************** AT read command ****************************************/

/* General */
int32_t CocoLinx_EG800AK::get_cgmi(char *manufacturer, int32_t max_size)
{
    char cmd[16];
    
    strcpy(cmd, "AT+CGMI");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(manufacturer != nullptr)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(manufacturer, _parser.tokens[0].buf, len);
        manufacturer[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cgmm(char *object_id, int32_t max_size)
{
    char cmd[16];
    
    strcpy(cmd, "AT+CGMM");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(object_id != nullptr)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(object_id, _parser.tokens[0].buf, len);
        object_id[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cgmr(char *revision, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CGMR");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(revision != nullptr)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(revision, _parser.tokens[0].buf, len);
        revision[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_imei(char *imei, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CGSN");

    int32_t ack = transfer_pkt(cmd, 1, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(imei != nullptr)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(imei, _parser.tokens[0].buf, len);
        imei[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cfun(int32_t *fun)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CFUN?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CFUN");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);    

    if(fun != nullptr) *fun = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cmee(int32_t *n)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CMEE?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CMEE");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(n != nullptr) *n = val;

    return ACK_OKAY;
}

/* Serial Interface Control Commands */
int32_t CocoLinx_EG800AK::get_baudrate(int32_t *baudrate)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+IPR?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+IPR");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(baudrate != nullptr) *baudrate = val;

    return ACK_OKAY;
}

/* SIM Related Commands */
int32_t CocoLinx_EG800AK::get_imsi(char *imsi, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CIMI");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 1) return -(ACK_ERR_PARSE);

    if(imsi != nullptr)
    {
        int32_t len = _parser.tokens[0].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(imsi, _parser.tokens[0].buf, len);
        imsi[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cpin(char *code, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CPIN?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CPIN");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(code != nullptr)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(code, _parser.tokens[index + 1].buf, len);
        code[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_iccid(char *iccid, int32_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+QCCID");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QCCID");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(iccid != nullptr)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(iccid, _parser.tokens[index + 1].buf, len);
        iccid[len] = '\0';
    }

    return ACK_OKAY;
}

/* Network Service Commands */
int32_t CocoLinx_EG800AK::get_cops(int32_t *oper)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+COPS?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 5) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+COPS");
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    int32_t len = _parser.tokens[index + 3].len;
    trim_quote(&_parser.tokens[index + 3].buf, &len);

    ret = char_to_int32(_parser.tokens[index + 3].buf, len, &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(oper != nullptr) *oper = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_creg(int32_t *stat)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CREG?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CREG");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(stat != nullptr) *stat = val;
    
    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_rssi(int32_t *rssi)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CSQ");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CSQ");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(rssi != nullptr)
    {
        if(val == 99)
        {
            *rssi = val;
            return ACK_OKAY;
        }

        if(val >= 0 && val <= 31) *rssi = (val * 2) - 113;
        else if(val >= 100 && val <= 191) *rssi = val - 216;
        else return ACK_ERR_PARSE;
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cesq(int32_t *rsrq, int32_t *rsrp)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CESQ");

    int32_t ack = transfer_pkt(cmd, 16, 2000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 7) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CESQ");
    if(index < 0 || index + 6 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 5], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(rsrq != nullptr) *rsrq = (val - 40) / 2;

    ret = char_to_int32(&_parser.tokens[index + 6], &val);
    if(!ret) return -(ACK_ERR_PARSE);       

    if(rsrp != nullptr) *rsrp = val - 141;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_get_network_local_time(char *time, int32_t max_size)
{
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+QLTS=2");

    int32_t ack = transfer_pkt(cmd, 2, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QLTS");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(time != nullptr)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        memcpy(time, _parser.tokens[index + 1].buf, len-2);
        time[len-2] = '\0';
    }

    return ACK_OKAY;
}

/* Packet Domain Commands */
int32_t CocoLinx_EG800AK::get_cgatt(int32_t *state)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CGATT?");

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGATT");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);   

    if(state != nullptr) *state = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cgdcont(int32_t *cid, char *pdp_type, char *apn, int32_t pdp_max_size, int32_t apn_max_size)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CGDCONT?");

    int32_t ack = transfer_pkt(cmd, 16, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 4) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGDCONT");
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(cid != nullptr) *cid = val;

    if(pdp_type != nullptr)
    {
        int32_t len = _parser.tokens[index + 2].len;
        if(pdp_max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 2].buf, &len);
        memcpy(pdp_type, _parser.tokens[index + 2].buf, len);
        pdp_type[len] = '\0';
    }

    if(apn != nullptr)
    {
        int32_t len = _parser.tokens[index + 3].len;
        if(apn_max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 3].buf, &len);
        memcpy(apn, _parser.tokens[index + 3].buf, len);
        apn[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cgact(int32_t *cid, int32_t *state)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CGACT?");

    int32_t ack = transfer_pkt(cmd, 16, 1000 * 60);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CGACT");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(cid != nullptr) *cid = val;

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(state != nullptr) *state = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::get_cereg(int32_t *stat)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+CEREG?");

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CEREG");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(stat != nullptr) *stat = val;
    
    return ACK_OKAY;
}


int32_t CocoLinx_EG800AK::get_cclk(char *time, uint8_t max_size)
{
    char cmd[16];

    strcpy(cmd, "AT+CCLK?");

    int32_t ack = transfer_pkt(cmd, 2, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 2) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+CCLK");
    if(index < 0 || index + 1 >= _parser.count) return -(ACK_ERR_PARSE);

    if(time != nullptr)
    {
        int32_t len = _parser.tokens[index + 1].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 1].buf, &len);
        memcpy(time, _parser.tokens[index + 1].buf, len);
        time[len] = '\0';
    }

    return ACK_OKAY;
}

/* Mobile termination control and status commands */

/* MQTT Related AT Commands */
int32_t CocoLinx_EG800AK::cx_get_mqtt_open(int32_t *client_idx, char *host_name, int32_t max_size, int32_t *port)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+QMTOPEN?");

    int32_t ack = transfer_pkt(cmd, 16, 15000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 4) return -(ACK_ERR_PARSE);
    
    int32_t index = find_prefix_token("+QMTOPEN");
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(client_idx != nullptr) *client_idx = val;

    if(host_name != nullptr)
    {
        int32_t len = _parser.tokens[index + 2].len;
        if(max_size <= len) return -(ACK_ERR_ARG);
        trim_quote(&_parser.tokens[index + 2].buf, &len);
        memcpy(host_name, _parser.tokens[index + 2].buf, len);
        host_name[len] = '\0';
    }

    ret = char_to_int32(&_parser.tokens[index + 3], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(port != nullptr) *port = val;

    return ACK_OKAY;
}


int32_t CocoLinx_EG800AK::cx_get_mqtt_con(int32_t *client_idx, int32_t *state)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+QMTCONN?");

    int32_t ack = transfer_pkt(cmd, 16, 10000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);
    
    int32_t index = find_prefix_token("#XMQTTCON");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(client_idx != nullptr) *client_idx = val;

    ret = char_to_int32(&_parser.tokens[index + 2], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(state != nullptr) *state = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_recv_info(int32_t *client_idx, int32_t *store_status, int32_t max_size)
{
    if(max_size < 5) return -(ACK_ERR_ARG);
    if(store_status == nullptr) return -(ACK_ERR_ARG);

    char cmd[16];
    bool ret;
    int32_t val;

    strcpy(cmd, "AT+QMTRECV?");

    int32_t ack = transfer_pkt(cmd, 32, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 7) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QMTRECV");
    if(index < 0) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(client_idx != nullptr) *client_idx = val;

    for(int32_t n = 0; n < 5; n++)
    {
        ret = char_to_int32(&_parser.tokens[index + n + 2], &store_status[n]);
        if(!ret) return -(ACK_ERR_PARSE);
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_mqtt_recv(int32_t client_idx, int32_t recv_id, char *topic, int32_t topic_size, uint8_t *data, int32_t data_size)
{
    int32_t val;
    char cmd[32];
    bool ret;

    snprintf(cmd, 32, "AT+QMTRECV=%d,%d", client_idx, recv_id);

    int32_t ack = transfer_pkt(cmd, 6, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 6) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QMTRECV");
    if(index < 0) return -(ACK_ERR_PARSE);

    if(topic != nullptr)
    {
        int32_t len = _parser.tokens[index + 3].len;
        trim_quote(&_parser.tokens[index + 3].buf, &len);
        if(topic_size <= len) return -(ACK_ERR_ARG);
        memcpy(topic, _parser.tokens[index + 3].buf, len);
        topic[len] = '\0';
    }

    ret = char_to_int32(&_parser.tokens[index + 4], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(data != nullptr)
    {
        int32_t len = _parser.tokens[index + 5].len;
        trim_quote(&_parser.tokens[index + 5].buf, &len);
        if(data_size < len) return -(ACK_ERR_ARG);
        memcpy(data, _parser.tokens[index + 5].buf, len);
    }

    return val;
}

int32_t CocoLinx_EG800AK::cx_mqtt_recv(int32_t client_idx, int32_t recv_id, char *topic, int32_t topic_size, char *data, int32_t data_size)
{
    return cx_mqtt_recv(client_idx, recv_id, topic, topic_size, (uint8_t *)data, data_size);
}

/* Socket AT commands */
int32_t CocoLinx_EG800AK::cx_get_context(int32_t context_id, int32_t *context_type, char *apn, int32_t max_size)
{
    int32_t val;
    char cmd[16];
    bool ret;

    snprintf(cmd, 16, "AT+QICSGP=%d", context_id);

    int32_t ack = transfer_pkt(cmd, 16, 2000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 6) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QICSGP");
    if(index < 0 || index + 5 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(context_type != nullptr) *context_type = val;

    if(apn != nullptr)
    {
        int32_t len = _parser.tokens[index + 2].len;
        trim_quote(&_parser.tokens[index + 2].buf, &len);
        if(max_size <= len) return -(ACK_ERR_ARG);
        memcpy(apn, _parser.tokens[index + 2].buf, len);
        apn[len] = '\0';
    }

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_get_pdp_state(int32_t context_id, int32_t *context_state, int32_t *context_type)
{
    int32_t val;
    char cmd[16];
    bool ret;

    strcpy(cmd, "AT+QIACT?");

    int32_t ack = transfer_pkt(cmd, 32, 30000);
    if(ack != 0) return ack;

    if(_parser.resp_code == AT_OK && _parser.count < 1) return ACK_NO_PDP;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 4) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QIACT");
    if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_PARSE);

    while(index + 3 < _parser.count)
    {
        ret = char_to_int32(&_parser.tokens[index + 1], &val);
        if(!ret) return -(ACK_ERR_PARSE);
    
        if(val != context_id) 
        {
            index = find_prefix_token("+QIACT", index + 3);
            if(index < 0 || index + 3 >= _parser.count) return -(ACK_ERR_ARG);
            continue;
        }
    
        ret = char_to_int32(&_parser.tokens[index + 2], &val);
        if(!ret) return -(ACK_ERR_PARSE);
    
        if(context_state != nullptr) *context_state = val;
    
        ret = char_to_int32(&_parser.tokens[index + 3], &val);
        if(!ret) return -(ACK_ERR_PARSE);
    
        if(context_type != nullptr) *context_type = val;

        return ACK_OKAY;
    }

    return -(ACK_ERR_ARG);
}

int32_t CocoLinx_EG800AK::cx_get_socket(int32_t connect_id, char *service_type, int32_t service_type_size, char *ip_address, int32_t ip_address_size, int32_t *remote_port)
{
    int32_t val;
    char cmd[32];
    bool ret;

    snprintf(cmd, 32, "AT+QISTATE=1,%d", connect_id);

    int32_t ack = transfer_pkt(cmd, 16, 1000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 5) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QISTATE");
    if(index < 0 || index + 4 >= _parser.count) return -(ACK_ERR_PARSE);

    if(service_type != nullptr)
    {
        int32_t len = _parser.tokens[index + 2].len;
        trim_quote(&_parser.tokens[index + 2].buf, &len);
        if(service_type_size <= len) return -(ACK_ERR_ARG);
        memcpy(service_type, _parser.tokens[index + 2].buf, len);
        service_type[len] = '\0';
    }

    if(ip_address != nullptr)
    {
        int32_t len = _parser.tokens[index + 3].len;
        trim_quote(&_parser.tokens[index + 3].buf, &len);
        if(ip_address_size <= len) return -(ACK_ERR_ARG);
        memcpy(ip_address, _parser.tokens[index + 3].buf, len);
        ip_address[len] = '\0';
    }

    ret = char_to_int32(&_parser.tokens[index + 4], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(remote_port != nullptr) *remote_port = val;

    return ACK_OKAY;
}

int32_t CocoLinx_EG800AK::cx_recv(int32_t connect_id, uint8_t *data, int32_t max_size)
{
    int32_t val;
    char cmd[32];
    bool ret;

    snprintf(cmd, 32, "AT+QIRD=%d", connect_id);

    int32_t ack = transfer_pkt(cmd, 5, 5000);
    if(ack != 0) return ack;

    if(_parser.resp_code != AT_OK) return -(get_at_error_ack());
    if(_parser.count < 3) return -(ACK_ERR_PARSE);

    int32_t index = find_prefix_token("+QIRD");
    if(index < 0 || index + 2 >= _parser.count) return -(ACK_ERR_PARSE);

    ret = char_to_int32(&_parser.tokens[index + 1], &val);
    if(!ret) return -(ACK_ERR_PARSE);

    if(data != nullptr)
    {
        uint16_t len = _parser.tokens[index + 2].len;
        if(max_size < len) return -(ACK_ERR_ARG);
        memcpy(data, _parser.tokens[index + 2].buf, len);
    }

    return val;
}

int32_t CocoLinx_EG800AK::cx_recv(int32_t connect_id, char *data, int32_t max_size)
{
    return cx_recv(connect_id, (uint8_t *)data, max_size);
}

/* Return The length of the converted string, or 0 if an error occurred. */
int32_t CocoLinx_EG800AK::bin_to_hex(const uint8_t *buf, int32_t buflen, char *hex, int32_t hexlen)
{
    static const char hex_table[] = "0123456789ABCDEF";

    if(buf == NULL) return 0;
    if(hex == NULL) return 0;
    if(buflen < 0) return 0;
    if(hexlen <= 0) return 0;

	if (hexlen < ((buflen * 2) + 1)) {
		return 0;
	}

    uint8_t value;

    for(int32_t i = 0; i < buflen; i++) 
    {
        value = buf[i];

        hex[i * 2]     = hex_table[(value >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_table[value & 0x0F];
    }

	hex[2 * buflen] = '\0';
	return 2 * buflen;
}