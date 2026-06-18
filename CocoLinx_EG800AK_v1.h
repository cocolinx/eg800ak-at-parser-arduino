#ifndef COCOLINX_EG800AK_V1_H
#define COCOLINX_EG800AK_V1_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class CocoLinx_EG800AK {
    public: 
        typedef enum {
            PLMN_NOT_SET = -1,
            PLMN_AUTO = 0,
            PLMN_SKT = 45005,
            PLMN_KT = 45008,
            PLMN_LGU = 45006
        } LtePlmn;

        typedef enum {
            ACK_OKAY = 0,

            // Common errors
            ACK_ERR_ARG,
            ACK_ERR_TIMEOUT,
            ACK_ERR_PARSE,
            ACK_ERR_RESP_OVERFLOW,

            // CMNG return value
            ACK_NO_MATCH,

            // pdp state return value
            ACK_NO_PDP,

            // AT command response
            ACK_AT_ERROR = 101,
            ACK_AT_CME_ERROR,
            ACK_AT_CMS_ERROR,
        } AckCode;

        CocoLinx_EG800AK();

        bool begin();

        int32_t get_at_error_code();

        /**************************************** AT set command ****************************************/
        /* General Commands */
        int32_t set_factory_reset();
        int32_t set_ate(int32_t value);
        int32_t set_cfun(int32_t fun);
        int32_t set_cmee(int32_t n);

        /* Serial Interface Control Commands */
        int32_t set_baudrate(int32_t baudrate);

        /* SIM Related Commands */
        int32_t set_cpin(int32_t code);

        /* Network Service Commands */
        int32_t set_cops(int32_t oper);
        int32_t set_auto_cops();
        int32_t set_creg(int32_t n);
        int32_t set_rtc_auto_local_time();

        /* Packet Domain Commands */
        int32_t set_cgatt(int32_t state);
        int32_t set_cgdcont(int32_t cid, char *pdp_type, char *apn);
        int32_t set_cgact(int32_t state, int32_t cid);
        int32_t set_cereg(int32_t n);

        /* Hardware Related Commands */
        int32_t set_qpowd(int32_t n);
        int32_t set_cclk(char *time); 

        /* MQTT Related AT Commands */
        int32_t cx_mqtt_cfg(const char *config_key, int32_t *params, int32_t param_count);
        int32_t cx_mqtt_cfg_version(int32_t client_idx, int32_t vsn);
        int32_t cx_mqtt_cfg_pcpcid(int32_t client_idx, int32_t cid);
        int32_t cx_mqtt_cfg_keepalive(int32_t client_idx, int32_t keep_alive_time);
        int32_t cx_mqtt_cfg_session(int32_t client_idx, int32_t clean_session);
        int32_t cx_mqtt_cfg_recvmode(int32_t client_idx, int32_t recv_mode, int32_t len_enable);
        int32_t cx_mqtt_open(int32_t client_idx, char *host_name, int32_t port);
        int32_t cx_mqtt_close(int32_t client_idx);
        int32_t cx_mqtt_conn(int32_t client_idx, char *client_id, char *user_name, char *password);
        int32_t cx_mqtt_disconn(int32_t client_idx);
        int32_t cx_mqtt_sub(int32_t client_idx, int32_t msg_id, const char *topic, int32_t qos);
        int32_t cx_mqtt_unsub(int32_t client_idx, int32_t msg_id, const char *topic);
        int32_t cx_mqtt_pub(int32_t client_idx, int32_t msg_id, int32_t qos, int32_t retain, const char *topic, const uint8_t *msg, int32_t length);
        int32_t cx_mqtt_pub(int32_t client_idx, int32_t msg_id, int32_t qos, int32_t retain, const char *topic, const char *msg, int32_t length);

        /* Socket AT commands */
        int32_t cx_socket_cfg(const char *config_key, int32_t *params, int32_t param_count);
        int32_t cx_socket_cfg_data_format(int32_t send_data_format, int32_t recv_data_format);
        int32_t cx_socket_cfg_tcp_keepalive(int32_t enable, int32_t idle_time, int32_t interval_time, int32_t probe_cnt);
        int32_t cx_socket_cfg_send_info(int32_t send_view_mode);
        int32_t cx_set_context(int32_t context_id, int32_t context_type, const char *apn, const char *username, const char *password, int32_t authentication, int32_t cdma_pwd);
        int32_t cx_pdp_activate(int32_t context_id);
        int32_t cx_pdp_deactivate(int32_t context_id);
        int32_t cx_socket(int32_t context_id, int32_t connect_id, const char *service_type, const char *ip_address, int32_t remote_port);
        int32_t cx_close(int32_t connect_id);
        int32_t cx_send(int32_t connect_id, const uint8_t *data, int32_t send_length);
        int32_t cx_send(int32_t connect_id, const char *data, int32_t send_length);
        int32_t cx_send_hex(int32_t connect_id, const uint8_t *data, int32_t send_length);

        /**************************************** AT read command ****************************************/
        /* General Commands */
        int32_t get_cgmi(char *manufacturer, int32_t max_size);
        int32_t get_cgmm(char *object_id, int32_t max_size);
        int32_t get_cgmr(char *revision, int32_t max_size);
        int32_t get_imei(char *imei, int32_t max_size);
        int32_t get_cfun(int32_t *fun);
        int32_t get_cmee(int32_t *n);

        /* Serial Interface Control Commands */
        int32_t get_baudrate(int32_t *baudrate);

        /* SIM Related Commands */
        int32_t get_imsi(char *imsi, int32_t max_size);
        int32_t get_cpin(char *code, int32_t max_size);
        int32_t get_iccid(char *iccid, int32_t max_size);

        /* Network Service Commands */
        int32_t get_cops(int32_t *oper);
        int32_t get_creg(int32_t *stat);
        int32_t get_rssi(int32_t *rssi);
        int32_t get_cesq(int32_t *rsrq, int32_t *rsrp);
        int32_t cx_get_network_local_time(char *time, int32_t max_size);

        /* Packet Domain Commands */
        int32_t get_cgatt(int32_t *state);
        int32_t get_cgdcont(int32_t *cid, char *pdp_type, char *apn, int32_t pdp_max_size, int32_t apn_max_size);
        int32_t get_cgact(int32_t *cid, int32_t *state);
        int32_t get_cereg(int32_t *stat);

        /* Hardware Related Commands */
        int32_t get_cclk(char *time, uint8_t max_size);

        /* MQTT Related AT Commands */
        int32_t cx_get_mqtt_open(int32_t *client_idx, char *host_name, int32_t max_size, int32_t *port);
        int32_t cx_get_mqtt_con(int32_t *client_idx, int32_t *state);
        int32_t cx_mqtt_recv_info(int32_t *client_idx, int32_t *store_status, int32_t max_size);
        int32_t cx_mqtt_recv(int32_t client_idx, int32_t recv_id, char *topic, int32_t topic_size, uint8_t *data, int32_t data_size);
        int32_t cx_mqtt_recv(int32_t client_idx, int32_t recv_id, char *topic, int32_t topic_size, char *data, int32_t data_size);

        /* Socket AT commands */
        int32_t cx_get_context(int32_t context_id, int32_t *context_type, char *apn, int32_t max_size);
        int32_t cx_get_pdp_state(int32_t context_id, int32_t *context_state, int32_t *context_type);
        int32_t cx_get_socket(int32_t connect_id, char *service_type, int32_t service_type_size, char *ip_address, int32_t ip_address_size, int32_t *remote_port);
        int32_t cx_recv(int32_t connect_id, uint8_t *data, int32_t max_size);
        int32_t cx_recv(int32_t connect_id, char *data, int32_t max_size);

        private:
        #define RESPONSE_DATA_SIZE_MAX 512
        #define AT_TX_DATA_SIZE_MAX    512
        #define AT_TX_BYTES_SIZE_MAX   256
        #define TOKEN_SIZE_MAX         32
        #define CR '\r'
        #define LF '\n'
        #define COMMA ','
        #define COLON ':'
        #define QUOTE '"'
        #define SPACE ' '
        #define NULL_TERMINATOR '\0'

        typedef enum {
            AT_OK = 0,
            AT_ERROR,
            AT_CME_ERROR,
            AT_CMS_ERROR
        } AtRespCode;
        
        typedef struct 
        {
            const char *buf;
            int32_t len;
        } AtToken;

        typedef struct 
        {
            const char *at;
            int32_t count;
            int32_t resp_code;
            int32_t at_error_code;
            AtToken tokens[TOKEN_SIZE_MAX];
        } AtParser;

        char _pktbuf[RESPONSE_DATA_SIZE_MAX+8];
        AtParser _parser;
        
        const uint8_t HW_RX_PIN = 0;
        const uint8_t HW_TX_PIN = 1;
        const uint8_t SW_RX_PIN = 8;
        const uint8_t SW_TX_PIN = 7;
        
        const uint32_t BAUDRATE = 115200;   

        void uart_write(const uint8_t *data, size_t size);
        int32_t uart_read(uint8_t *buf, size_t bufsize);
        void uart_flush();
        uint32_t get_ms();
        void sleep_ms(uint32_t ms);

        void at_parser_init();
        static void trim_cr_lf(const char **str);
        static void trim_left_space(const char **str);
        static void trim_quote(const char **str, int32_t *len);
        static bool is_resp(const char *str);
        static bool is_end_response(const char *str);
        static bool is_end_line(const char *str);
        static bool is_colon_or_comma(const char *str);
        static bool is_quote(const char *str);
        int32_t find_prefix_token(const char *str, int32_t start_idx = 0);
        int32_t get_at_error_ack();
        
        int32_t bin_to_hex(const uint8_t *buf, int32_t buflen, char *hex, int32_t hexlen);

        static bool char_to_int32(const char *str, int32_t len, int32_t *resp);
        static bool char_to_int32(const AtToken *tok, int32_t *resp);
        int32_t transfer_pkt(const uint8_t *cmd, int32_t cmd_size, int32_t max_tokens, uint32_t timeout_ms);
        int32_t transfer_pkt(const char *cmd, int32_t max_tokens, uint32_t timeout_ms);
        int32_t transfer_pkt_data(const char *cmd, int32_t cmd_size, const uint8_t *data, int32_t data_size, uint32_t timeout_ms);

        int32_t at_pkt_parser(int32_t max_tokens, const uint8_t *cmd, int32_t cmd_size);                                                                                                                                                      

};

#endif // COCOLINX_EG800AK_V1_H
