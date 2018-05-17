#ifndef TC_IOT_CERTS_H
#define TC_IOT_CERTS_H

#if defined(ENABLE_TLS)
/**
 * @brief 预置的 HTTPS 默认根证书
 */
extern const char * g_tc_iot_https_root_ca_certs;

/**
 * @brief 预置的 MQTT TLS 默认根证书
 */
extern const char * g_tc_iot_mqtt_root_ca_certs;
#endif

#if defined(ENABLE_DTLS)
/**
 * @brief 预置的 CoAP DTLS 默认根证书
 */
extern const char * g_tc_iot_coap_root_ca_certs;
#endif

#endif /* end of include guard */
