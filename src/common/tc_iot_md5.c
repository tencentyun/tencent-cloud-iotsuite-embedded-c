#include "tc_iot_inc.h" 

/* md5 initial state array */
static const unsigned int md5_initial_state[TC_IOT_MD5_STATE_SIZE] = {
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476,
};

/* precomputed table for:
 * specifies the per-round shift amounts */
static const unsigned char md5_s[TC_IOT_MD5_BLOCK_SIZE] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22 ,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20 ,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23 ,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 ,
};


/* precomputed table for:
 for i from 0 to 63
    if 0 ≤ i ≤ 15 then
        md5_g[i] := i
    else if 16 ≤ i ≤ 31
        md5_g[i] := (5×i + 1) mod 16
    else if 32 ≤ i ≤ 47
        md5_g[i] := (3×i + 5) mod 16
    else if 48 ≤ i ≤ 63
        md5_g[i] := (7×i) mod 16
 end for
 */
static const unsigned char md5_g[TC_IOT_MD5_BLOCK_SIZE] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
    5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
    0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9,
};

/* precomputed table for:
for i from 0 to 63
    md5_k[i] := floor(232 × abs(sin(i + 1)))
end for
*/
static const unsigned int md5_k[TC_IOT_MD5_BLOCK_SIZE] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE ,
    0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501 ,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE ,
    0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821 ,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA ,
    0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8 ,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED ,
    0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A ,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C ,
    0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70 ,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05 ,
    0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665 ,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039 ,
    0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1 ,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1 ,
    0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391 ,
};

/* padding data a single "1" bit following with n bit "0" */
static const unsigned char md5_padding[TC_IOT_MD5_BLOCK_SIZE] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


void tc_iot_md5_init( tc_iot_md5_t *p_md5 )
{
    memset(p_md5, 0, sizeof(tc_iot_md5_t) );
    memcpy(p_md5->state, &md5_initial_state[0], TC_IOT_MD5_STATE_SIZE * sizeof(unsigned int));
}

void tc_iot_md5_free( tc_iot_md5_t *p_md5 )
{
    if ( p_md5 == NULL ) {
        return;
    }

    tc_iot_md5_init(p_md5);
}


/* md5 process fuctions */
#define F(x,y,z)    (z ^ (x & (y ^ z)))
#define G(x,y,z)    F(z,x,y)
#define H(x,y,z)    (x ^ y ^ z)
#define I(x,y,z)    (y ^ (x | ~z))

#define S(x,n)      ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define _STEP(FUNC,a,b,c,d,i)                   \
do {                                            \
    a += FUNC(b,c,d) + X[md5_g[i]] + md5_k[i];  \
    a = S(a,md5_s[i]) + b;                      \
} while (0)                                     \

void tc_iot_md5_process( tc_iot_md5_t *p_md5, const unsigned char data[TC_IOT_MD5_BLOCK_SIZE] )
{
    unsigned int X[TC_IOT_MD5_BLOCK_WORD_COUNT];
    unsigned int A;
    unsigned int B;
    unsigned int C;
    unsigned int D;
    int i = 0;

    if (p_md5 == NULL) {
        return;
    }

    /* memcpy(X, data, TC_IOT_MD5_BLOCK_SIZE); */
    for (i = 0; i < TC_IOT_MD5_BLOCK_WORD_COUNT; i++) {
        X[i] = TC_IOT_ADDR_TO_UINT32_LE(&data[i*4]);
    }
    A = p_md5->state[0];
    B = p_md5->state[1];
    C = p_md5->state[2];
    D = p_md5->state[3];

    _STEP( F, A, B, C, D,  0 );
    _STEP( F, D, A, B, C,  1 );
    _STEP( F, C, D, A, B,  2 );
    _STEP( F, B, C, D, A,  3 );
    _STEP( F, A, B, C, D,  4 );
    _STEP( F, D, A, B, C,  5 );
    _STEP( F, C, D, A, B,  6 );
    _STEP( F, B, C, D, A,  7 );
    _STEP( F, A, B, C, D,  8 );
    _STEP( F, D, A, B, C,  9 );
    _STEP( F, C, D, A, B, 10 );
    _STEP( F, B, C, D, A, 11 );
    _STEP( F, A, B, C, D, 12 );
    _STEP( F, D, A, B, C, 13 );
    _STEP( F, C, D, A, B, 14 );
    _STEP( F, B, C, D, A, 15 );

    _STEP( G, A, B, C, D, 16 );
    _STEP( G, D, A, B, C, 17 );
    _STEP( G, C, D, A, B, 18 );
    _STEP( G, B, C, D, A, 19 );
    _STEP( G, A, B, C, D, 20 );
    _STEP( G, D, A, B, C, 21 );
    _STEP( G, C, D, A, B, 22 );
    _STEP( G, B, C, D, A, 23 );
    _STEP( G, A, B, C, D, 24 );
    _STEP( G, D, A, B, C, 25 );
    _STEP( G, C, D, A, B, 26 );
    _STEP( G, B, C, D, A, 27 );
    _STEP( G, A, B, C, D, 28 );
    _STEP( G, D, A, B, C, 29 );
    _STEP( G, C, D, A, B, 30 );
    _STEP( G, B, C, D, A, 31 );

    _STEP( H, A, B, C, D, 32 );
    _STEP( H, D, A, B, C, 33 );
    _STEP( H, C, D, A, B, 34 );
    _STEP( H, B, C, D, A, 35 );
    _STEP( H, A, B, C, D, 36 );
    _STEP( H, D, A, B, C, 37 );
    _STEP( H, C, D, A, B, 38 );
    _STEP( H, B, C, D, A, 39 );
    _STEP( H, A, B, C, D, 40 );
    _STEP( H, D, A, B, C, 41 );
    _STEP( H, C, D, A, B, 42 );
    _STEP( H, B, C, D, A, 43 );
    _STEP( H, A, B, C, D, 44 );
    _STEP( H, D, A, B, C, 45 );
    _STEP( H, C, D, A, B, 46 );
    _STEP( H, B, C, D, A, 47 );

    _STEP( I, A, B, C, D, 48 );
    _STEP( I, D, A, B, C, 49 );
    _STEP( I, C, D, A, B, 50 );
    _STEP( I, B, C, D, A, 51 );
    _STEP( I, A, B, C, D, 52 );
    _STEP( I, D, A, B, C, 53 );
    _STEP( I, C, D, A, B, 54 );
    _STEP( I, B, C, D, A, 55 );
    _STEP( I, A, B, C, D, 56 );
    _STEP( I, D, A, B, C, 57 );
    _STEP( I, C, D, A, B, 58 );
    _STEP( I, B, C, D, A, 59 );
    _STEP( I, A, B, C, D, 60 );
    _STEP( I, D, A, B, C, 61 );
    _STEP( I, C, D, A, B, 62 );
    _STEP( I, B, C, D, A, 63 );

    p_md5->state[0] += A;
    p_md5->state[1] += B;
    p_md5->state[2] += C;
    p_md5->state[3] += D;
}

void tc_iot_md5_update( tc_iot_md5_t *p_md5, const unsigned char *input, int input_len )
{
    unsigned int left;
    int padding;

    if (p_md5 == NULL) {
        return;
    }

    if (input == NULL) {
        return;
    }

    if ( input_len == 0 ) {
        return;
    }

    left = p_md5->total_low & 0x3F;
    padding = TC_IOT_MD5_BLOCK_SIZE - left;

    p_md5->total_low += (unsigned int) input_len;
    p_md5->total_low &= 0xFFFFFFFF;

    if ( p_md5->total_low < (unsigned int) input_len ) {
        p_md5->total_high++;
    }

    if ( left && (input_len >= padding )) {
        memcpy( p_md5->buffer+left, input, padding);
        tc_iot_md5_process( p_md5, p_md5->buffer );
        input += padding;
        input_len -= padding;
        left = 0;
    }

    while ( input_len >= TC_IOT_MD5_BLOCK_SIZE )
    {
        tc_iot_md5_process( p_md5, input );
        input += TC_IOT_MD5_BLOCK_SIZE;
        input_len -= TC_IOT_MD5_BLOCK_SIZE;
    }

    if ( input_len > 0 ) {
        memcpy(p_md5->buffer+left, input, input_len);
    }
}

void tc_iot_md5_finish( tc_iot_md5_t *p_md5, unsigned char digest[TC_IOT_MD5_DIGEST_SIZE] )
{
    unsigned int left;
    unsigned int padding_len;
    unsigned int pad_high;
    unsigned int pad_low;
    unsigned char pad_msglen[8];
    int i = 0;

    if (p_md5 == NULL) {
        return;
    }

    pad_high = ( p_md5->total_low >> 29 ) | ( p_md5->total_high <<  3 );
    pad_low  = ( p_md5->total_low <<  3 );

    TC_IOT_SAVE_UINT32_LE_TO_ADDR( pad_low, &pad_msglen[0]);
    TC_IOT_SAVE_UINT32_LE_TO_ADDR( pad_high, &pad_msglen[4]);

    left = p_md5->total_low & 0x3F;
    padding_len = ( left < 56 ) ? ( 56 - left ) : ( 120 - left );

    tc_iot_md5_update( p_md5, md5_padding, padding_len );
    tc_iot_md5_update( p_md5, pad_msglen, 8 );

    for (i = 0; i < 4; i++) {
        TC_IOT_SAVE_UINT32_LE_TO_ADDR( p_md5->state[i], &digest[i*4] );
    }
}

void tc_iot_md5_digest( const unsigned char *input, int input_len, unsigned char digest[TC_IOT_MD5_DIGEST_SIZE] )
{
    tc_iot_md5_t md5;

    tc_iot_md5_init( &md5 );
    tc_iot_md5_update( &md5, input, input_len );
    tc_iot_md5_finish( &md5, digest );
    tc_iot_md5_free( &md5 );
}

