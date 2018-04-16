#ifndef TC_IOT_HAL_NETWORK_H
#define TC_IOT_HAL_NETWORK_H

#include "tc_iot_inc.h"

typedef struct tc_iot_network_t tc_iot_network_t;

#ifdef ENABLE_TLS


/**
 * @brief TLS 相关配置信息
 */
typedef struct tc_iot_tls_config_t {
    const char* root_ca_in_mem; /**< 内存中已加载的根证书*/
    const char* root_ca_location; /**< 文件系统中对应根证书路径*/
    const char* device_cert_location; /**< 设备证书路径*/
    const char* device_private_key_location; /**< 设备私钥路径*/
    uint32_t timeout_ms; /**< TLS 超时设置*/
    char verify_server; /**< TLS 连接时，是否验证服务器证书 */
} tc_iot_tls_config_t;

/**< TLS动态运行数据 */
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

/**
 * @brief 网络连接参数初始数据
 */
typedef struct {
    uint16_t use_tls; /**< 是否基于 TLS 方式通讯*/
    char* host; /**< 服务器域名或地址*/
    uint16_t port; /**< 服务器端口*/
    int fd; /**< 连接句柄fd */
    int is_connected; /**< 是否网络已连接*/
    void * extra_context; /**< 平台相关网络数据*/

#ifdef ENABLE_TLS
    tc_iot_tls_config_t tls_config; /**< TLS 配置*/
#endif

} tc_iot_net_context_init_t;

/**
 * @brief 网络连接参数及过程会话数据
 */
typedef struct {
    uint16_t use_tls; /**< 是否基于 TLS 方式通讯*/
    char* host; /**< 服务器域名或地址*/
    uint16_t port; /**< 服务器端口*/
    int fd; /**< 连接句柄fd */
    int is_connected; /**< 是否网络已连接*/

    void * extra_context; /**< 平台相关网络数据*/

#ifdef ENABLE_TLS
    tc_iot_tls_config_t tls_config; /**< TLS 配置*/
    tc_iot_tls_data_t tls_data; /**< TLS 运行数据*/
#endif

} tc_iot_net_context_t;


/**
 * @brief 网络连接对象，包括相关参数及回调函数
 */
struct tc_iot_network_t {
    int (*do_read)(struct tc_iot_network_t* network, unsigned char* read_buf,
                   int read_buf_len, int timeout_ms); /**< 接收对端网络发送数据*/
    int (*do_write)(struct tc_iot_network_t* network, const unsigned char* write_buf,
                    int write_buf_len, int timeout_ms); /**< 发送指定数据到对端网络*/
    int (*do_connect)(tc_iot_network_t* network, const char* host, uint16_t port); /**< 连接远程服务器*/
    int (*do_disconnect)(tc_iot_network_t* network); /**< 断开网络连接*/
    int (*is_connected)(tc_iot_network_t* network); /**< 判断网络是否已连接*/
    int (*do_destroy)(tc_iot_network_t* network);/**< 销毁网络连接对象*/

    tc_iot_net_context_t net_context; /**< 网络连接参数及过程会话数据*/
} ;


/**
 * @brief tc_iot_hal_net_init 初始化 TCP 连接对象，设置相关参数和对应回调等
 *
 * @param network 网络连接对象
 * @param net_context 连接参数
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_init(tc_iot_network_t* network,
        tc_iot_net_context_init_t* net_context);


/**
 * @brief tc_iot_hal_net_connect 连接服务端
 *
 * @param network 网络连接对象
 * @param host 服务器域名或IP地址
 * @param port 服务器端口
 *
 * @return 结果返回码 
 *	成功返回 TC_IOT_SUCCESS (0)
 *	失败返回 TC_IOT_FAILURE(-90) 或者其他具体原因(负数)
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_connect(tc_iot_network_t* network, const char* host,
                             uint16_t port);

/**
 * @brief tc_iot_hal_net_read 接收网络对端发送的数据
 *
 * @param network 网络连接对象
 * @param buffer 接收缓存区
 * @param len 接收缓存区大小
 * @param timeout_ms 最大等待时延，单位ms
 *
 * @return 结果返回码或成功读取字节数
 *	假如timeout_ms超时读取了0字节, 返回 TC_IOT_NET_NOTHING_READ
 *  假如timeout_ms超时读取字节数没有达到 len , 返回TC_IOT_NET_READ_TIMEOUT
 *  假如timeout_ms超时对端关闭连接, 返回 实际读取字节数
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                          int len, int timeout_ms);

/**
 * @brief tc_iot_hal_net_write 发送数据到网络对端
 *
 * @param network 网络连接对象
 * @param buffer 发送缓存区
 * @param len 发送缓存区大小
 * @param timeout_ms 最大发送等待时延，单位ms
 *
 * @return 结果返回码或成功发送字节数
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_write(tc_iot_network_t* network,const  unsigned char* buffer,
                           int len, int timeout_ms);


/**
 * @brief tc_iot_hal_net_is_connected 判断当前是否已成功建立网络连接
 *
 * @param network 网络连接对象
 *
 * @return 1 表示已连接，0 表示未连接
 */
int tc_iot_hal_net_is_connected(tc_iot_network_t* network);


/**
 * @brief tc_iot_hal_net_disconnect 断开网络连接
 *
 * @param network 网络连接对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_disconnect(tc_iot_network_t* network);


/**
 * @brief tc_iot_hal_net_destroy 释放网络相关资源
 *
 * @param network 网络连接对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_net_destroy(tc_iot_network_t* network);

#ifdef ENABLE_TLS

/**
 * @brief tc_iot_hal_tls_init 初始化 TLS 连接数据
 *
 * @param network 网络连接对象
 * @param net_context 连接参数
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_init(tc_iot_network_t* network,
        tc_iot_net_context_init_t* net_context);

/**
 * @brief tc_iot_hal_tls_connect 连接 TLS 服务端并进行相关握手及认证
 *
 * @param network 网络连接对象
 * @param host 服务器域名或IP地址
 * @param port 服务器端口
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_connect(tc_iot_network_t* network, const char* host,
                               uint16_t port);

/**
 * @brief tc_iot_hal_tls_read 接收 TLS 对端发送的数据
 *
 * @param network 网络连接对象
 * @param buffer 接收缓存区
 * @param len 接收缓存区大小
 * @param timeout_ms 最大等待时延，单位ms
 *
 * @return 结果返回码或成功读取字节数
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_read(tc_iot_network_t* network, unsigned char* buffer,
        int len, int timeout_ms) ;

/**
 * @brief tc_iot_hal_tls_write 发送数据到 TLS 对端
 *
 * @param network 网络连接对象
 * @param buffer 发送缓存区
 * @param len 发送缓存区大小
 * @param timeout_ms 最大发送等待时延，单位ms
 *
 * @return 结果返回码或成功发送字节数
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_write(tc_iot_network_t* network, const unsigned char* buffer,
        int len, int timeout_ms);

/**
 * @brief tc_iot_hal_tls_is_connected 判断当前是否已成功建立 TLS 连接
 *
 * @param network 网络连接对象
 *
 * @return 1 表示已连接，0 表示未连接
 */
int tc_iot_hal_tls_is_connected(tc_iot_network_t* network);


/**
 * @brief tc_iot_hal_tls_disconnect 断开 TLS 连接   
 *
 * @param network 网络连接对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_disconnect(tc_iot_network_t* network);


/**
 * @brief tc_iot_hal_tls_destroy 释放 TLS 相关资源
 *
 * @param network 网络连接对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_hal_tls_destroy(tc_iot_network_t* network);

int tc_iot_copy_net_context(tc_iot_net_context_t * dest, tc_iot_net_context_init_t * init);

#endif

#endif /* end of include guard */
