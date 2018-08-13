#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
#include <getopt.h>

static int _log_level = TC_IOT_LOG_LEVEL_INFO;
static int show_version = 0;

static struct option long_options[] =
{
    {"verbose",      no_argument,          &_log_level, TC_IOT_LOG_LEVEL_WARN},
    {"trace",        no_argument,          &_log_level, TC_IOT_LOG_LEVEL_TRACE},
    {"version",        no_argument,        &show_version, 1},
    {"host",         optional_argument,    0, 'h'},
    {"port",         optional_argument,    0, 'p'},
    {"product",      optional_argument,    0, 't'},
    {"secret",       optional_argument,    0, 's'},
    {"device",       optional_argument,    0, 'd'},
    {"client",       optional_argument,    0, 'i'},
    {"cafile",       optional_argument,    0, 'a'},
    {"clifile",      optional_argument,    0, 'c'},
    {"clikey",       optional_argument,    0, 'k'},
    {"help",         optional_argument,    0, '?'},
    {0, 0, 0, 0}
};

const char * command_help =
"Usage: %s [-h host] [-p port] [-i client_id]\r\n"
"                     [-d] [--trace]\r\n"
"                     [-u username [-P password]]\r\n"
"                     [{--cafile file | --capath dir} [--cert file] [--key file]\r\n"
"\r\n"
"       %s --help\r\n"
"\r\n"
" -h coap_server\r\n"
" --host=coap_server\r\n"
"     CoAP server host to connect to. Defaults to localhost.\r\n"
" \r\n"
" -p port\r\n"
" --port=port\r\n"
"     network port to connect to. Defaults to 5683, or set to 5684 for tls is MQTT server supports.\r\n"
"\r\n"
" -i client_id\r\n"
" --client=client_id\r\n"
"     id to use for this client. \r\n"
"\r\n"
" -t product_id\r\n"
" --product=product_id\r\n"
"     product_id to use for this client. \r\n"
"\r\n"
" -d device_name\r\n"
" --device=device_name\r\n"
"     device_name to use for this client. \r\n"
"\r\n"
" -s secret\r\n"
" --secret=secret\r\n"
"     secret for dynamic token, it not using fixed usename and password.\r\n"
"\r\n"
" -a path_to_ca_crt\r\n"
" --cafile=path_to_ca_certificates\r\n"
" path to a file containing trusted CA certificates to enable encrypted\r\n"
"            communication.\r\n"
"\r\n"
" -c path_to_client_crt\r\n"
" --clifile=path_to_client_crt\r\n"
"     client certificate for authentication, if required by server.\r\n"
"\r\n"
" -k path_to_client_key\r\n"
" --clikey=path_to_client_key\r\n"
"     client private key for authentication, if required by server.\r\n"
" \r\n"
" --verbose\r\n"
"     don't print info and debug, trace messages.\r\n"
"\r\n"
" --trace\r\n"
"     print all system messages, for debug or problem tracing.\r\n"
"  -?\r\n"
" --help\r\n"
"     display this message.\r\n"
;

void parse_command(tc_iot_coap_client_config * config, int argc, char ** argv) {
    int c;
    int option_index = 0;
    bool device_name_changed = false;
    bool client_id_changed = false;
    int left = 0;
    int i = 0;

    while (1)
    {
        c = getopt_long (argc, argv, "s:h:p:t:d:i:u:P:c:a:k:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) {
                    break;
                }
                tc_iot_hal_printf ("option %s", long_options[option_index].name);
                if (optarg) {
                    tc_iot_hal_printf (" with arg %s", optarg);
                }
                tc_iot_hal_printf ("\n");
                break;
            case 'h':
                if (optarg) {
                    config->host =  optarg;
                    tc_iot_hal_printf ("host=%s\n", optarg);
                }
                break;
            case 'p':
                if (optarg) {
                    config->port =  atoi(optarg);
                    if (config->port == 5684) {
                        config->use_tls = 1;
                        tc_iot_hal_printf ("port=%s\n", optarg);
                    } else if (config->port == 5683) {
                        config->use_tls = 0;
                        tc_iot_hal_printf ("port=%s\n", optarg);
                    } else if (config->port == 0){
                        tc_iot_hal_printf ("invalid port=%s\n", optarg);
                        exit(0);
                    } else {
                        config->use_tls = (config->port > 5684);
                        tc_iot_hal_printf ("WARNING: unknown port=%d\n", (int)config->port);
                    }
                }
                break;
            case 's':
                if (optarg) {
                    strncpy(config->device_info.secret, optarg, TC_IOT_MAX_SECRET_LEN);
                    tc_iot_hal_printf ("secret=%s\n", optarg);
                }
                break;
#if defined(ENABLE_DTLS)
            case 'a':
                if (optarg) {
                    config->p_root_ca = optarg;
                    tc_iot_hal_printf ("root ca=%s\n", config->p_root_ca);
                }
                break;
            case 'c':
                if (optarg) {
                    config->p_client_crt = optarg;
                    tc_iot_hal_printf ("client crt=%s\n", config->p_client_crt);
                }
                break;
            case 'k':
                if (optarg) {
                    config->p_client_key = optarg;
                    tc_iot_hal_printf ("client key=%s\n", config->p_client_key);
                }
                break;
#endif
            case 't':
                if (optarg) {
                    strncpy(config->device_info.product_id, optarg, TC_IOT_MAX_PRODUCT_ID_LEN);
                    tc_iot_hal_printf ("product id=%s\n", config->device_info.product_id);
                }
                break;
            case 'd':
                if (optarg) {
                    strncpy(config->device_info.device_name, optarg, TC_IOT_MAX_DEVICE_NAME_LEN);
                    device_name_changed = true;
                    tc_iot_hal_printf ("device name=%s\n", config->device_info.device_name);
                }
                break;
            case 'i':
                if (optarg) {
                    client_id_changed = true;
                    strncpy(config->device_info.client_id, optarg, TC_IOT_MAX_CLIENT_ID_LEN);
                    tc_iot_hal_printf ("client id=%s\n", config->device_info.client_id);
                }
                break;
	    case '?':
		tc_iot_hal_printf (command_help, argv[0]);
                exit(0);
		break;

            default:
                tc_iot_hal_printf("option: %c\n", (char)c);
                break;
        }
    }

    if (device_name_changed && !client_id_changed) {
        for (i = 0; i < TC_IOT_MAX_CLIENT_ID_LEN; i++) {
            if (config->device_info.client_id[i] == '@') {
                left = TC_IOT_MAX_CLIENT_ID_LEN - i- 2;
                strncpy(config->device_info.client_id+i+1, config->device_info.device_name, left);
                config->device_info.client_id[TC_IOT_MAX_CLIENT_ID_LEN-1] = '\0';
                break;
            }
        }
    }

    tc_iot_set_log_level(_log_level);

    if (optind < argc)
    {
        tc_iot_hal_printf ("non-option ARGV-elements: ");
        while (optind < argc) {
            tc_iot_hal_printf ("%s ", argv[optind++]);
        }
        tc_iot_hal_printf ("\n");
    }

    if (show_version) {
        tc_iot_hal_printf ("%s based on Tencent Cloud Io TSuite C SDK Version %s\n", argv[0], TC_IOT_SDK_VERSION);
        exit(0);
    }
}

