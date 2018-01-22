#ifndef IOT_HMAC_01091643_H
#define IOT_HMAC_01091643_H

#define TC_IOT_SHA256_HASH_SIZE 32
#define TC_IOT_SHA256_BUFFER_SIZE 16
#define TC_IOT_SHA256_STATE_SIZE 8
#define TC_IOT_SHA256_LENGTH_SIZE 8

typedef struct _tc_iot_sha256_t {
    uint8_t hash[TC_IOT_SHA256_HASH_SIZE];
    uint32_t buffer[TC_IOT_SHA256_BUFFER_SIZE];
    uint32_t state[TC_IOT_SHA256_STATE_SIZE];
    uint8_t len[TC_IOT_SHA256_LENGTH_SIZE];
} tc_iot_sha256_t;

#define TC_IOT_SHA256_DIGEST_SIZE TC_IOT_SHA256_HASH_SIZE
#define TC_IOT_SHA256_KEY_SIZE 64

typedef struct _tc_iot_hmac_sha256_t {
    uint8_t digest[TC_IOT_SHA256_DIGEST_SIZE];
    uint8_t key[TC_IOT_SHA256_KEY_SIZE];
    tc_iot_sha256_t sha;
} tc_iot_hmac_sha256_t;

void tc_iot_hmac_sha256(unsigned char* buffer, int buffer_len,
                        const unsigned char* secret, int secret_len,
                        unsigned char* digest);

#endif /* end of include guard */
