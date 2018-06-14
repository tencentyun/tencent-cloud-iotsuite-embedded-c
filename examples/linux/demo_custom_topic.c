#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

#define BINARY_PROCOCOL_VERSION 0x13

typedef enum _binary_protocol_cmd_e {
    CMD_INVALID,
    CMD_GET,
    CMD_SET,
    CMD_REPORT,
    CMD_UPGRADE,
    CMD_BUTT,
}binary_protocol_cmd_e;

#pragma pack(1)
typedef struct _binary_protocol {
    unsigned char  version;
    unsigned char  cmd;
    int            seqno;
    short var_array_len;
    char  var_array[16];
}binary_protocol;
#pragma pack()

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);

bool use_static_token = false;

void my_default_msg_handler(tc_iot_message_data*);
void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx);
void _refresh_token();

int run_mqtt(tc_iot_mqtt_client_config* p_client_config);

tc_iot_mqtt_client_config g_client_config = {
    {
        /* device info*/
        TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
        TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
    },
    TC_IOT_CONFIG_SERVER_HOST,
    TC_IOT_CONFIG_SERVER_PORT,
    TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
    TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS,
    TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
    TC_IOT_CONFIG_CLEAN_SESSION,
    TC_IOT_CONFIG_USE_TLS,
    TC_IOT_CONFIG_AUTO_RECONNECT,
    TC_IOT_CONFIG_ROOT_CA,
    TC_IOT_CONFIG_CLIENT_CRT,
    TC_IOT_CONFIG_CLIENT_KEY,
    my_disconnect_handler,
    my_default_msg_handler,
};

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SUB_TOPIC_DEF;
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_PUB_TOPIC_DEF;

int main(int argc, char** argv) {
    int ret = 0;
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    _refresh_token();

    /* 用户自定义 Topic ，格式一般固定以 prodcut id 和 device name为前缀， */
    /* 格式为：{$product_id}/{$device_name}/xxx/yyy/zzz .... */
    /* 具体后缀根据实际业务情况来定义，自行创建。 
     * 本demo运行需要创建的Topic为： {$product_id}/{$device_name}/cmd
     * */
    const char * custom_topic = "%s/%s/cmd";
    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN,  custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    binary_protocol binary_data = {BINARY_PROCOCOL_VERSION,CMD_INVALID,0,0,{0}};
    binary_protocol * p_net_data = (binary_protocol *)message->payload;

    binary_data.version = p_net_data->version;
    binary_data.cmd = p_net_data->cmd;
    binary_data.seqno = ntohl(p_net_data->seqno);
    binary_data.var_array_len = ntohs(p_net_data->var_array_len) & 0xFF;
    memcpy(binary_data.var_array, p_net_data->var_array, binary_data.var_array_len);
    tc_iot_hal_printf("[s->c]len=%d,version=0x%x,cmd_no=%d,int=%d,array(len=%d)=%s\n",
            (int)message->payloadlen,
            binary_data.version,
            binary_data.cmd,
            binary_data.seqno,
            binary_data.var_array_len,
            binary_data.var_array
            );
}

void _refresh_token() {
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);

    if (!use_static_token) {
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token(
                TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                timestamp, nonce,
                &p_client_config->device_info);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }
}

void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx) {
    /* 自动刷新 password*/
    if (!use_static_token && tc_iot_mqtt_get_auto_reconnect(c)) {
        /* _refresh_token(); */
    }
}

void my_default_msg_handler(tc_iot_message_data * md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("UNHANDLED [s->c] %s\n", (char*)message->payload);
}


static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}

void do_sim_data(tc_iot_mqtt_client * p_client) {
    int ret = 0;
    static char cmd = CMD_GET;
    binary_protocol binary_data = {BINARY_PROCOCOL_VERSION,CMD_INVALID,0,0,{0}};
    char binary_buffer[sizeof(binary_protocol)];
    char encode_buffer[128];
    binary_protocol * p_net_data = (binary_protocol *)binary_buffer;
    binary_data.version = BINARY_PROCOCOL_VERSION;
    binary_data.cmd = cmd;
    cmd++;
    binary_data.cmd %= CMD_BUTT;
    binary_data.seqno = tc_iot_hal_random() & 0xFFFFFFFF;
    switch(binary_data.cmd) {
        case CMD_INVALID:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD_INVALID");
            break;
        case CMD_GET:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD_GET");
            break;
        case CMD_SET:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD_SET");
            break;
        case CMD_REPORT:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD_REPORT");
            break;
        case CMD_UPGRADE:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD_UPGRADE");
            break;
        default:
            tc_iot_hal_snprintf(binary_data.var_array, sizeof( binary_data.var_array ), "CMD unkown");
            break;

    }
    binary_data.var_array_len = strlen(binary_data.var_array);

    p_net_data->version = binary_data.version;
    p_net_data->cmd = binary_data.cmd;
    p_net_data->seqno = htonl(binary_data.seqno);
    /* p_net_data->var_array_len = htons(binary_data.var_array_len | 0xFF00); */
    p_net_data->var_array_len = htons(binary_data.var_array_len);
    memcpy(p_net_data->var_array, binary_data.var_array, binary_data.var_array_len);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, '\0', sizeof(pubmsg));
    pubmsg.payload = binary_buffer;
    pubmsg.payloadlen = sizeof(binary_buffer)-(sizeof(binary_data.var_array)-binary_data.var_array_len);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    tc_iot_hal_printf("[c->s]len=%d:version=0x%x,cmd_no=%d,int=%d,array(len=%d)=%s\n",
            (int)pubmsg.payloadlen,
            binary_data.version,
            binary_data.cmd,
            binary_data.seqno,
            binary_data.var_array_len,
            binary_data.var_array
            );

    tc_iot_util_byte_to_hex(pubmsg.payload, pubmsg.payloadlen, encode_buffer, sizeof(encode_buffer));
    tc_iot_hal_printf("payload hex    : %s\n", encode_buffer);
    ret = tc_iot_base64_encode(pubmsg.payload, pubmsg.payloadlen, encode_buffer, sizeof(encode_buffer));
    encode_buffer[ret] = '\0';
    tc_iot_hal_printf("payload base64 : %s\n",encode_buffer);
    ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
            tc_iot_hal_printf("publish to topic %s failed, trouble shooting guide: " "%s#%d\n", 
                    pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        } else {
            tc_iot_hal_printf("publish to topic %s failed, try reconnecting, "
                    "or visit trouble shooting guide: " "%s#%d\n", pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        }
    }

}

int run_mqtt(tc_iot_mqtt_client_config* p_client_config) {
    int ret;
    int i;
    char * action_get;
    int timeout = 200;
    int delay_ms = 5000;
    int delay_counter = delay_ms;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("connect MQTT server failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }
    ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, TC_IOT_QOS1,
                                           _on_message_received, NULL);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("subscribe topic %s failed, trouble shooting guide: " "%s#%d\n", sub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    while (!stop) {
        if (delay_counter >= delay_ms) {
            do_sim_data(p_client);
            delay_counter = 0;
        }
        ret = tc_iot_mqtt_client_yield(p_client, timeout);
        if (ret != TC_IOT_SUCCESS) {
            TC_IOT_LOG_ERROR("yield return=%d", ret);
        }
        delay_counter += timeout;
    }

    tc_iot_mqtt_client_disconnect(p_client);
}

