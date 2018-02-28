#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

#define TC_IOT_SHA256_I_S0(x) \
    (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3)
#define TC_IOT_SHA256_I_S1(x) \
    (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10)

#define TC_IOT_SHA256_M_S0(x) \
    (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10)
#define TC_IOT_SHA256_M_S1(x) \
    (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7)

static const uint32_t sha256_initial_state[TC_IOT_SHA256_STATE_SIZE] = {
    0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
    0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19,
};

static const uint32_t k[64] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1,
    0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174, 0xE49B69C1, 0xEFBE4786,
    0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
    0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85, 0xA2BFE8A1, 0xA81A664B,
    0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A,
    0x5B9CCA4F, 0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
};

static void tc_iot_sha256_init(tc_iot_sha256_t *p_sha) {
    if (!p_sha) {
        return;
    }

    memset(p_sha, 0, sizeof(tc_iot_sha256_t));
    memcpy(p_sha->state, &sha256_initial_state[0],
           TC_IOT_SHA256_STATE_SIZE * sizeof(uint32_t));
}

static void tc_iot_sha256_update(tc_iot_sha256_t *p_sha, const uint8_t *buffer,
                                 uint32_t len) {
    int i, j, t;
    int index;
    int bits;
    uint32_t w[64], a, b, c, d, e, f, g, h;
    uint32_t is0, is1, ms0, ms1;
    uint32_t ch, temp1, temp2, maj;

    if ((!p_sha) || (!buffer)) {
        return;
    }

    for (i = 7; i >= 0; --i) {
        if (i == 7) {
            bits = len << 3;
        } else if (i == 0 || i == 1 || i == 2) {
            bits = 0;
        } else {
            bits = len >> (53 - 8 * i);
        }
        bits &= 0xff;
        if (p_sha->len[i] + bits > 0xff) {
            for (j = i - 1; j >= 0 && p_sha->len[j]++ == 0xff; --j)
                ;
        }
        p_sha->len[i] += bits;
    }

    while (len > 0) {
        index = p_sha->len[6] % 2 * 32 + p_sha->len[7] / 8;
        index = (index + 64 - len % 64) % 64;
        for (; len > 0 && index < 64; buffer++, index++, len--) {
            p_sha->buffer[index / 4] |= *buffer << (24 - index % 4 * 8);
        }
        if (index == 64) {
            memcpy(&w[0], &(p_sha->buffer[0]), 16 * sizeof(uint32_t));
            memset(&(p_sha->buffer[0]), 0, 16 * sizeof(uint32_t));

            for (t = 16; t < 64; t++) {
                is0 = TC_IOT_SHA256_I_S0(w[t - 15]);
                is1 = TC_IOT_SHA256_I_S1(w[t - 2]);
                w[t] = (is1 + w[t - 7] + is0 + w[t - 16]) & 0xFFFFFFFFU;
            }
            a = p_sha->state[0];
            b = p_sha->state[1];
            c = p_sha->state[2];
            d = p_sha->state[3];
            e = p_sha->state[4];
            f = p_sha->state[5];
            g = p_sha->state[6];
            h = p_sha->state[7];
            for (t = 0; t < 64; t++) {
                ms1 = TC_IOT_SHA256_M_S1(e);
                ch = ((e & f) ^ (~e & g));
                temp1 = h + ms1 + ch + k[t] + w[t];

                ms0 = TC_IOT_SHA256_M_S0(a);
                maj = ((a & b) ^ (a & c) ^ (b & c));
                temp2 = ms0 + maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }
            p_sha->state[0] = (p_sha->state[0] + a) & 0xFFFFFFFFU;
            p_sha->state[1] = (p_sha->state[1] + b) & 0xFFFFFFFFU;
            p_sha->state[2] = (p_sha->state[2] + c) & 0xFFFFFFFFU;
            p_sha->state[3] = (p_sha->state[3] + d) & 0xFFFFFFFFU;
            p_sha->state[4] = (p_sha->state[4] + e) & 0xFFFFFFFFU;
            p_sha->state[5] = (p_sha->state[5] + f) & 0xFFFFFFFFU;
            p_sha->state[6] = (p_sha->state[6] + g) & 0xFFFFFFFFU;
            p_sha->state[7] = (p_sha->state[7] + h) & 0xFFFFFFFFU;
        }
    }
}

static void tc_iot_sha256_finish(tc_iot_sha256_t *p_sha, const uint8_t *buffer,
                                 uint32_t len) {
    int i;
    uint8_t terminator[64 + 8] = {0x80};
    if (!p_sha) {
        return;
    }

    if (!buffer) {
        return;
    }

    if (len > 0) {
        tc_iot_sha256_update(p_sha, buffer, len);
    }
    len = 64 - p_sha->len[6] % 2 * 32 - p_sha->len[7] / 8;
    if (len < 9) {
        len += 64;
    }
    for (i = 0; i < 8; i++) {
        terminator[len - 8 + i] = p_sha->len[i];
    }
    tc_iot_sha256_update(p_sha, terminator, len);
    for (i = 0; i < TC_IOT_SHA256_HASH_SIZE; i++) {
        p_sha->hash[i] = (p_sha->state[i / 4] >> (24 - 8 * (i % 4))) & 0xff;
    }
}

static void tc_iot_hmac_sha256_init(tc_iot_hmac_sha256_t *hmac,
                                    const uint8_t *key, int len) {
    int i;
    if (len <= TC_IOT_SHA256_KEY_SIZE) {
        for (i = 0; i < len; i++) {
            hmac->key[i] = key[i] ^ 0x36;
        }
        for (; i < TC_IOT_SHA256_KEY_SIZE; i++) {
            hmac->key[i] = 0x36;
        }
    } else {
        tc_iot_sha256_init(&(hmac->sha));
        tc_iot_sha256_finish(&(hmac->sha), key, len);
        for (i = 0; i < TC_IOT_SHA256_HASH_SIZE; i++) {
            hmac->key[i] = hmac->sha.hash[i] ^ 0x36;
        }

        for (; i < TC_IOT_SHA256_KEY_SIZE; i++) {
            hmac->key[i] = 0x36;
        }
    }

    tc_iot_sha256_init(&(hmac->sha));
    tc_iot_sha256_update(&(hmac->sha), hmac->key, TC_IOT_SHA256_KEY_SIZE);
}


static void tc_iot_hmac_sha256_finish(tc_iot_hmac_sha256_t *hmac,
                                      const uint8_t *buffer, int len) {
    int i;
    tc_iot_sha256_finish(&(hmac->sha), buffer, len);
    for (i = 0; i < TC_IOT_SHA256_DIGEST_SIZE; i++) {
        hmac->digest[i] = hmac->sha.hash[i];
    }

    for (i = 0; i < TC_IOT_SHA256_KEY_SIZE; i++) {
        hmac->key[i] ^= (0x36 ^ 0x5c);
    }

    tc_iot_sha256_init(&(hmac->sha));
    tc_iot_sha256_update(&(hmac->sha), hmac->key, TC_IOT_SHA256_KEY_SIZE);
    tc_iot_sha256_finish(&(hmac->sha), hmac->digest, TC_IOT_SHA256_DIGEST_SIZE);

    memcpy(hmac->digest, hmac->sha.hash, TC_IOT_SHA256_DIGEST_SIZE);
}

void tc_iot_hmac_sha256(unsigned char *buffer, int buffer_len,
                        const unsigned char *secret, int secret_len,
                        unsigned char *digest) {
    tc_iot_hmac_sha256_t hmac;
    tc_iot_hmac_sha256_init(&hmac, secret, secret_len);
    tc_iot_hmac_sha256_finish(&hmac, buffer, buffer_len);
    memcpy(digest, hmac.digest, TC_IOT_SHA256_DIGEST_SIZE);
}

#ifdef __cplusplus
}
#endif
