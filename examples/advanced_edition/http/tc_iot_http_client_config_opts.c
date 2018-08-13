#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
#include <getopt.h>

static int _log_level = TC_IOT_LOG_LEVEL_DEBUG;

static struct option long_options[] =
{
    {"verbose",      no_argument,          &_log_level, TC_IOT_LOG_LEVEL_WARN},
    {"trace",        no_argument,          &_log_level, TC_IOT_LOG_LEVEL_TRACE},
    {"product",      optional_argument,    0, 't'},
    {"secret",       optional_argument,    0, 's'},
    {"device",       optional_argument,    0, 'd'},
    {"help",         optional_argument,    0, '?'},
    {0, 0, 0, 0}
};

const char * command_help =
"Usage: %s [-d device_name] [-s device_secret]\r\n"
"                     [-d] [--trace]\r\n"
"\r\n"
"       %s --help\r\n"
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
" --verbose\r\n"
"     don't print info and debug, trace messages.\r\n"
"\r\n"
" --trace\r\n"
"     print all system messages, for debug or problem tracing.\r\n"
"  -?\r\n"
" --help\r\n"
"     display this message.\r\n"
;

void parse_command(tc_iot_device_info * p_device, int argc, char ** argv) {
    int c;
    int option_index = 0;

    while (1)
    {
        c = getopt_long (argc, argv, "s:d:t:?:", long_options, &option_index);
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
            case 's':
                if (optarg) {
                    strncpy(p_device->secret, optarg, TC_IOT_MAX_SECRET_LEN);
                    tc_iot_hal_printf ("secret=%s\n", optarg);
                }
                break;
            case 't':
                if (optarg) {
                    strncpy(p_device->product_id, optarg, TC_IOT_MAX_PRODUCT_ID_LEN);
                    tc_iot_hal_printf ("product id=%s\n", p_device->product_id);
                }
                break;
            case 'd':
                if (optarg) {
                    strncpy(p_device->device_name, optarg, TC_IOT_MAX_DEVICE_NAME_LEN);
                    tc_iot_hal_printf ("device name=%s\n", p_device->device_name);
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

