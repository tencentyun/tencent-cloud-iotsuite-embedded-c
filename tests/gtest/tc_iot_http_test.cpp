extern "C" {
#include "tc_iot_inc.h"
}
#include "gtest/gtest.h"

TEST(tc_iot_http_utils_testcases, tc_iot_calc_sign)
{
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];

    char buffer[1024];
    int buffer_len = sizeof(buffer);
    char buffer_new[1024];
    int buffer_new_len = sizeof(buffer_new);

    const char * secret = "test";
    int ret = 0;

    ret = tc_iot_calc_sign(sha256_digest,
            sizeof(sha256_digest),
            secret, 
            "clientId=%s&deviceName=%s&expire=%d&nonce=%d&productId=%s&timestamp=%d", 
            "mqtt-5ns8xh714@light001",
            "light001",
            5184000,
            823189986,
            "iot-7hjcfc6k",
            1530865330
            );
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer, buffer_len);
    buffer[ret] = '\0';

    ret = tc_iot_calc_sign(sha256_digest,
            sizeof(sha256_digest),
            secret, 
            "clientId=%s&deviceName=%s&expire=%d&nonce=%d&productId=%s&timestamp=%d", 
            "mqtt-5ns8xh714@light001",
            "light001",
            5184000,
            823189986,
            "iot-7hjcfc6k",
            1530865330
            );
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer, buffer_len);
    buffer[ret] = '\0';

    // printf("%s\n", buffer);

    tc_iot_hal_snprintf(buffer_new, buffer_new_len, 
            "clientId=%s&deviceName=%s&expire=%d&nonce=%d&productId=%s&timestamp=%d", 
            "mqtt-5ns8xh714@light001",
            "light001",
            5184000,
            823189986,
            "iot-7hjcfc6k",
            1530865330
            );
    tc_iot_hmac_sha256((unsigned char *)buffer_new, strlen(buffer_new), (const unsigned char *)secret, strlen(secret), (unsigned char *)sha256_digest);
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer_new, buffer_new_len);
    buffer_new[ret] = '\0';

    ASSERT_STREQ(buffer, buffer_new);
}


