#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
#include <getopt.h>

static int _log_level = TC_IOT_LOG_TRACE;
static int request_token = 1;

static struct option long_options[] =
{
    {"verbose",      no_argument,          &_log_level, TC_IOT_LOG_WARN},
    {"host",         optional_argument,    0, 'h'},
    {"port",         optional_argument,    0, 'p'},
    {"product",      optional_argument,    0, 't'},
    {"secret",       optional_argument,    0, 's'},
    {"device",       optional_argument,    0, 'd'},
    {"client",       optional_argument,    0, 'i'},
    {"username",     optional_argument,    0, 'u'},
    {"password",     optional_argument,    0, 'P'},
    {"help",         optional_argument,    0, '?'},
    {0, 0, 0, 0}
};

void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) {
    int c;
    int option_index = 0;
    while (1)
    {
        c = getopt_long (argc, argv, "s:h:p:t:d:i:u:P:", long_options, &option_index);
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
                printf ("option %s", long_options[option_index].name);
                if (optarg) {
                    printf (" with arg %s", optarg);
                }
                printf ("\n");
                break;
            case 'h':
                if (optarg) {
                    config->host =  optarg;
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
                }
                break;
            case 's':
                if (optarg) {
                    strncpy(config->device_info.secret, optarg, TC_IOT_MAX_SECRET_LEN);
                }
                break;
            case 'u':
                if (optarg) {
                    strncpy(config->device_info.username, optarg, TC_IOT_MAX_USER_NAME_LEN);
                    request_token = 0;
                }
                break;
            case 'P':
                if (optarg) {
                    strncpy(config->device_info.password, optarg, TC_IOT_MAX_PASSWORD_LEN);
                    request_token = 0;
                }
                break;
            case 't':
                if (optarg) {
                    strncpy(config->device_info.product_id, optarg, TC_IOT_MAX_PRODUCT_ID_LEN);
                }
                break;
            case 'd':
                if (optarg) {
                    strncpy(config->device_info.device_name, optarg, TC_IOT_MAX_DEVICE_NAME_LEN);
                }
                break;
            case 'i':
                if (optarg) {
                    strncpy(config->device_info.client_id, optarg, TC_IOT_MAX_CLIENT_ID_LEN);
                }
                break;

            default:
                printf("option: %c\n", (char)c);
                break;
        }
    }

    tc_iot_set_log_level(_log_level);

    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc) {
            tc_iot_hal_printf ("%s ", argv[optind++]);
        }
        tc_iot_hal_printf ("\n");
    }
}

