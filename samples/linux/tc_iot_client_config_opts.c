#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
#include <getopt.h>

static int _log_level = TC_IOT_LOG_DEBUG;
static int request_token = 1;
static int use_tls = TC_IOT_CONFIG_USE_TLS;

static struct option long_options[] =
{
    {"verbose",      no_argument,          &_log_level, TC_IOT_LOG_WARN},
    {"trace",        no_argument,          &_log_level, TC_IOT_LOG_TRACE},
    {"disable_tls",  no_argument,          &use_tls, 0},
    {"host",         optional_argument,    0, 'h'},
    {"port",         optional_argument,    0, 'p'},
    {"product",      optional_argument,    0, 't'},
    {"secret",       optional_argument,    0, 's'},
    {"device",       optional_argument,    0, 'd'},
    {"client",       optional_argument,    0, 'i'},
    {"username",     optional_argument,    0, 'u'},
    {"password",     optional_argument,    0, 'P'},
    {"cafile",       optional_argument,    0, 'a'},
    {"clifile",      optional_argument,    0, 'c'},
    {"clikey",       optional_argument,    0, 'k'},
    {"help",         optional_argument,    0, '?'},
    {0, 0, 0, 0}
};

void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) {
    int c;
    int option_index = 0;
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
                    if (config->port == 8883) {
                        config->use_tls = 1;
                    } else if (config->port == 1883) {
                        config->use_tls = 0;
                    }
                    tc_iot_hal_printf ("port=%s\n", optarg);
                }
                break;
            case 's':
                if (optarg) {
                    strncpy(config->device_info.secret, optarg, TC_IOT_MAX_SECRET_LEN);
                    tc_iot_hal_printf ("secret=%s\n", optarg);
                }
                break;
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
            case 'u':
                if (optarg) {
                    strncpy(config->device_info.username, optarg, TC_IOT_MAX_USER_NAME_LEN);
                    tc_iot_hal_printf ("username=%s\n", config->device_info.username);
                    request_token = 0;
                }
                break;
            case 'P':
                if (optarg) {
                    strncpy(config->device_info.password, optarg, TC_IOT_MAX_PASSWORD_LEN);
                    tc_iot_hal_printf ("password=%s\n", config->device_info.password);
                    request_token = 0;
                }
                break;
            case 't':
                if (optarg) {
                    strncpy(config->device_info.product_id, optarg, TC_IOT_MAX_PRODUCT_ID_LEN);
                    tc_iot_hal_printf ("product id=%s\n", config->device_info.product_id);
                }
                break;
            case 'd':
                if (optarg) {
                    strncpy(config->device_info.device_name, optarg, TC_IOT_MAX_DEVICE_NAME_LEN);
                    tc_iot_hal_printf ("device name=%s\n", config->device_info.device_name);
                }
                break;
            case 'i':
                if (optarg) {
                    strncpy(config->device_info.client_id, optarg, TC_IOT_MAX_CLIENT_ID_LEN);
                    tc_iot_hal_printf ("client id=%s\n", config->device_info.client_id);
                }
                break;

            default:
                tc_iot_hal_printf("option: %c\n", (char)c);
                break;
        }
    }

    config->use_tls =  (char)use_tls;

    tc_iot_set_log_level(_log_level);

    if (optind < argc)
    {
        tc_iot_hal_printf ("non-option ARGV-elements: ");
        while (optind < argc) {
            tc_iot_hal_printf ("%s ", argv[optind++]);
        }
        tc_iot_hal_printf ("\n");
    }
}

