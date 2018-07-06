#ifndef TC_IOT_MD5
#define TC_IOT_MD5
/* convert data to uint32 little endian */
#define TC_IOT_ADDR_TO_UINT32_LE(addr) \
       ((unsigned int) (*(addr  ))      ) \
     | ((unsigned int) (*(addr+1)) << 8 ) \
     | ((unsigned int) (*(addr+2)) << 16) \
     | ((unsigned int) (*(addr+3)) << 24) \

/* uint32 little endian data to local type*/
#define TC_IOT_SAVE_UINT32_LE_TO_ADDR(n,addr)                \
{                                                            \
    *(addr    ) = (unsigned char) (((n)       ) & 0xFF);    \
    *(addr + 1) = (unsigned char) (((n) >>  8 ) & 0xFF);    \
    *(addr + 2) = (unsigned char) (((n) >>  16) & 0xFF);    \
    *(addr + 3) = (unsigned char) (((n) >>  24) & 0xFF);    \
}

#define TC_IOT_MD5_STATE_SIZE       4
#define TC_IOT_MD5_DIGEST_SIZE      16
#define TC_IOT_MD5_BLOCK_SIZE       64
#define TC_IOT_MD5_BLOCK_WORD_COUNT 16

typedef struct _tc_iot_md5_t
{
    unsigned int total_low;
    unsigned int total_high;
    unsigned int state[TC_IOT_MD5_STATE_SIZE];
    unsigned char buffer[TC_IOT_MD5_BLOCK_SIZE];
} tc_iot_md5_t;

void tc_iot_md5_init( tc_iot_md5_t *p_md5 );
void tc_iot_md5_free( tc_iot_md5_t *p_md5 );
void tc_iot_md5_update( tc_iot_md5_t *p_md5, const unsigned char *input, int input_len );
void tc_iot_md5_finish( tc_iot_md5_t *p_md5, unsigned char digest[TC_IOT_MD5_DIGEST_SIZE] );
void tc_iot_md5_process( tc_iot_md5_t *p_md5, const unsigned char data[TC_IOT_MD5_BLOCK_SIZE] );
void tc_iot_md5_digest( const unsigned char *input, int input_len, unsigned char digest[TC_IOT_MD5_DIGEST_SIZE] );

#endif /* end of include guard */
