#ifndef TC_IOT_COAP_H
#define TC_IOT_COAP_H

#include "tc_iot_inc.h"

/*
 * CoAP Message Format
 * https://tools.ietf.org/html/rfc7252
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Ver| T |  TKL  |      Code     |          Message ID           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Token (if any, TKL bytes) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Options (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 1 1 1 1 1 1 1|    Payload (if any) ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

#define TC_IOT_COAP_VER   (0x1)

typedef enum _tc_iot_coap_message_type {
    COAP_CON = 0,
    COAP_NON = 1,
    COAP_ACK = 2,
    COAP_RST = 3,
}tc_iot_coap_message_type;

/*
0.00      Indicates an Empty message (see Section 4.1).

0.01-0.31 Indicates a request.  Values in this range are assigned by
		 the "CoAP Method Codes" sub-registry (see Section 12.1.1).

1.00-1.31 Reserved

2.00-5.31 Indicates a response.  Values in this range are assigned by
		 the "CoAP Response Codes" sub-registry (see
		 Section 12.1.2).<Paste>
*/
typedef enum _tc_iot_coap_sub_registry_main{
    COAP_CODE_REQ = 0,                                                           // 请求包
	COAP_CODE_RESERVED = 1,
    COAP_CODE_RSP_SUCCESS = 2,                                                   // 成功回包
    COAP_CODE_RSP_BAD_REQ = 4,                                                   // 客户端错误回包
    COAP_CODE_RSP_SERVER_FAILURE = 5,                                            // 后台错误回包
} tc_iot_coap_sub_registry_main;

#define TC_IOT_COAP_DEFINE_CODE(m,s) ((m << 5) | s)
/*
   +------+--------+-----------+
   | Code | Name   | Reference |
   +------+--------+-----------+
   | 0.01 | GET    | [RFC7252] |
   | 0.02 | POST   | [RFC7252] |
   | 0.03 | PUT    | [RFC7252] |
   | 0.04 | DELETE | [RFC7252] |
   +------+--------+-----------+
*/
typedef enum _tc_iot_coap_req_code {
    COAP_REQ_GET = 1,
    COAP_REQ_POST = 2,
    COAP_REQ_PUT = 3,
    COAP_REQ_DELETE = 4,
}tc_iot_coap_req_code;

/*
 *
    +------+------------------------------+-----------+
    | Code | Description                  | Reference |
    +------+------------------------------+-----------+
    | 2.01 | Created                      | [RFC7252] |
    | 2.02 | Deleted                      | [RFC7252] |
    | 2.03 | Valid                        | [RFC7252] |
    | 2.04 | Changed                      | [RFC7252] |
    | 2.05 | Content                      | [RFC7252] |
    | 4.00 | Bad Request                  | [RFC7252] |
    | 4.01 | Unauthorized                 | [RFC7252] |
    | 4.02 | Bad Option                   | [RFC7252] |
    | 4.03 | Forbidden                    | [RFC7252] |
    | 4.04 | Not Found                    | [RFC7252] |
    | 4.05 | Method Not Allowed           | [RFC7252] |
    | 4.06 | Not Acceptable               | [RFC7252] |
    | 4.12 | Precondition Failed          | [RFC7252] |
    | 4.13 | Request Entity Too Large     | [RFC7252] |
    | 4.15 | Unsupported Content-Format   | [RFC7252] |
    | 5.00 | Internal Server Error        | [RFC7252] |
    | 5.01 | Not Implemented              | [RFC7252] |
    | 5.02 | Bad Gateway                  | [RFC7252] |
    | 5.03 | Service Unavailable          | [RFC7252] |
    | 5.04 | Gateway Timeout              | [RFC7252] |
    | 5.05 | Proxying Not Supported       | [RFC7252] |
    +------+------------------------------+-----------+

 */
typedef enum _tc_iot_coap_rsp_code {
    COAP_CODE_201_CREATED = TC_IOT_COAP_DEFINE_CODE(2,0x1),
    COAP_CODE_202_DELETED = TC_IOT_COAP_DEFINE_CODE(2,0x2),
    COAP_CODE_203_VALID   = TC_IOT_COAP_DEFINE_CODE(2,0x3),
    COAP_CODE_204_CHANGED = TC_IOT_COAP_DEFINE_CODE(2,0x4),
    COAP_CODE_205_CONTENT = TC_IOT_COAP_DEFINE_CODE(2,0x5),
}tc_iot_coap_rsp_code;

typedef union _tc_iot_coap_header {
	unsigned char all;	/**< coap header */
#if defined(REVERSED)
	struct
	{
        unsigned char ver:2;
        unsigned char type:2;
        unsigned char token_len:4;
	} bits;
#else
	struct
	{
        unsigned char token_len:4;
        unsigned char type:2;
        unsigned char ver:2;
	} bits;
#endif
} tc_iot_coap_header;

typedef struct _tc_iot_coap_option {
    short delta;
    int   length;
    unsigned char * value;
} tc_iot_coap_option;

typedef struct _tc_iot_coap_message {
    tc_iot_coap_header header;
    unsigned char code;
    unsigned short message_id;
    char token[4];
    char * p_options;
    char * p_payload;
}tc_iot_coap_message;

int tc_iot_coap_serialize(char * buffer, int buffer_len, const tc_iot_coap_message * message);

#endif /* end of include guard */
