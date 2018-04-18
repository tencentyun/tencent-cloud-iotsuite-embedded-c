#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"
#define HTTPS_PREFIX "https"
#define HTTPS_PREFIX_LEN (sizeof(HTTPS_PREFIX) - 1)
typedef struct _tc_iot_down_helper{
    int fd;
    const char * digest;
    tc_iot_md5_t md5_context;
}tc_iot_download_helper;

int my_http_download_callback(const void * context, const char * data, int data_len, int offset, int total) {
    tc_iot_download_helper * helper = (tc_iot_download_helper *)context;
    /* tc_iot_hal_printf("\n[%d/%d]\n->%s", offset+data_len, total, data); */
    tc_iot_hal_printf("%d/%d\n", offset+data_len, total);
    write(helper->fd,data,data_len);
    tc_iot_md5_update(&helper->md5_context, data, data_len);
    /* tc_iot_hal_printf("%s", data); */
}

int main(int argc, char** argv) {
    int i = 0;
    int ret = 0;
    int fd = -1;
    const char * download_url = NULL;
    const char * filename = NULL;
    unsigned char file_md5_digest[TC_IOT_MD5_DIGEST_SIZE];
    char md5str[TC_IOT_MD5_DIGEST_SIZE*2 +1];
    tc_iot_download_helper helper;
    memset(file_md5_digest, 0, sizeof(file_md5_digest));
    memset(&helper, 0, sizeof(helper));

    if (argc <= 0) {
print_help:
        printf("%s http://localhost/test.dat [-w filename]\n", argv[0]);
        return 0 ;
    } else {
        if (argc == 2) {
            download_url = argv[1];
        } else {
            for (i = 1; i < argc; i++) {
                if (strncmp(argv[i],"http", 4) == 0) {
                    download_url = argv[i];
                } else if (strcmp(argv[i], "-w") == 0) {
                    if (i < argc-1) {
                        i++;
                        filename = argv[i];
                    }
                }
                if (download_url && filename) {
                    break;
                }
            }
        }
    }

    if (!download_url) {
        goto print_help;
    }

    if (!filename) {
        filename = download_url + strlen(download_url);
        while(filename > download_url) {
            filename--;
            if (*filename == '/') {
                filename++;
                if (strlen(filename) == 0) {
                    filename = "default.dat";
                }
                break;
            }
        }
    }

    fd=open(filename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd != -1){
        tc_iot_hal_printf("%s file is created.\n", filename);
    }

    helper.fd = fd;
    tc_iot_md5_init(&helper.md5_context);
    ret = tc_iot_do_download(download_url, my_http_download_callback, &helper);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("ERROR: %s download as %s failed.\n", download_url, filename);
    } else {
        tc_iot_md5_finish(&helper.md5_context, file_md5_digest);
        tc_iot_hal_printf("%s download as %s success\n", download_url, filename);
        tc_iot_hal_printf("md5=%s\n", tc_iot_util_byte_to_hex(file_md5_digest, sizeof(file_md5_digest), md5str, sizeof(md5str)));
    }

    close(fd);
}

