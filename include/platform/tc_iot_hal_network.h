#ifndef SYS_NETWORK_01091042_H
#define SYS_NETWORK_01091042_H

#include "tc_iot_inc.h"

typedef struct tc_iot_network_t tc_iot_network_t;

#ifdef ENABLE_TLS
typedef struct tc_iot_tls_config_t {
    const char* root_ca_in_mem;
    const char* root_ca_location;
    const char* device_cert_location;
    const char* device_private_key_location;
    uint32_t timeout_ms;
    char verify_server;
} tc_iot_tls_config_t;

typedef struct tc_iot_tls_data_t {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl_context;
    mbedtls_ssl_config conf;
    uint32_t flags;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
    mbedtls_net_context ssl_fd;
} tc_iot_tls_data_t;

#endif

typedef struct {
    uint16_t use_tls;
    char* host;
    uint16_t port;
    int fd;
    int is_connected;

#ifdef STM32
    sim800l_t *eth; //for STM32 porting
#endif

#ifdef ENABLE_TLS
    tc_iot_tls_config_t tls_config;
    tc_iot_tls_data_t tls_data;
#endif

} tc_iot_net_context_t;

typedef struct tc_iot_network_t {
    int (*do_read)(struct tc_iot_network_t* network, unsigned char* read_buf,
                   int read_buf_len, int timeout_ms);
    int (*do_write)(struct tc_iot_network_t* network, unsigned char* write_buf,
                    int write_buf_len, int timeout_ms);
    int (*do_connect)(tc_iot_network_t* network, char* host, uint16_t port);
    int (*do_disconnect)(tc_iot_network_t* network);
    int (*is_connected)(tc_iot_network_t* network);
    int (*do_destroy)(tc_iot_network_t* network);

    tc_iot_net_context_t net_context;
} tc_iot_network_t;

int tc_iot_hal_net_init(tc_iot_network_t* network,
        tc_iot_net_context_t* net_context);

int tc_iot_hal_net_connect(tc_iot_network_t* network, char* host,
                             uint16_t port);
int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                          int len, int timeout_ms);
int tc_iot_hal_net_write(tc_iot_network_t* network, unsigned char* buffer,
                           int len, int timeout_ms);
int tc_iot_hal_net_is_connected(tc_iot_network_t* network);
int tc_iot_hal_net_disconnect(tc_iot_network_t* network);
int tc_iot_hal_net_destroy(tc_iot_network_t* network);

#ifdef ENABLE_TLS
int tc_iot_hal_tls_init(tc_iot_network_t* network,
        tc_iot_net_context_t* net_context);
int tc_iot_hal_tls_connect(tc_iot_network_t* network, char* host,
                               uint16_t port);
int tc_iot_hal_tls_read(tc_iot_network_t* network, unsigned char* buffer,
        int len, int timeout_ms) ;
int tc_iot_hal_tls_write(tc_iot_network_t* network, unsigned char* buffer,
        int len, int timeout_ms);
int tc_iot_hal_tls_is_connected(tc_iot_network_t* network);
int tc_iot_hal_tls_disconnect(tc_iot_network_t* network);
int tc_iot_hal_tls_destroy(tc_iot_network_t* network);

#endif

#endif /* end of include guard */
