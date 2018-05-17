#include "tc_iot_inc.h"

static tc_iot_code_map _tc_iot_coap_message_type_map[] = {
    {COAP_CON,"CON"},
    {COAP_NON,"NON"},
    {COAP_ACK,"ACK"},
    {COAP_RST,"RST"},
};

const char * tc_iot_coap_get_message_type_str(int type) {
    if (type <= COAP_RST) {
        return _tc_iot_coap_message_type_map[type].str;
    } else {
        return "";
    }
}

static tc_iot_code_map _tc_iot_coap_message_code_map[] = {
    {COAP_CODE_001_GET,"0.01-GET"},
    {COAP_CODE_002_POST,"0.02-POST"},
    {COAP_CODE_003_PUT,"0.03-PUT"},
    {COAP_CODE_004_DELETE,"0.04-DELETE"},
    {COAP_CODE_201_CREATED,"2.01-CREATED"},
    {COAP_CODE_202_DELETED,"2.02-DELETED"},
    {COAP_CODE_203_VALID,"2.03-VALID"},
    {COAP_CODE_204_CHANGED,"2.04-CHANGED"},
    {COAP_CODE_205_CONTENT,"2.05-CONTENT"},
    {COAP_CODE_400_BAD_REQUEST,"4.00-BAD_REQUEST"},
    {COAP_CODE_401_UNAUTHORIZED,"4.01-UNAUTHORIZED"},
    {COAP_CODE_402_BAD_OPTION,"4.02-BAD_OPTION"},
    {COAP_CODE_403_FORBIDDEN,"4.03-FORBIDDEN"},
    {COAP_CODE_404_NOT_FOUND,"4.04-NOT_FOUND"},
    {COAP_CODE_405_METHOD_NOT_ALLOWED,"4.05-METHOD_NOT_ALLOWED"},
    {COAP_CODE_406_NOT_ACCEPTABLE,"4.06-NOT_ACCEPTABLE"},
    {COAP_CODE_412_PRECONDITION_FAILED,"4.12-PRECONDITION_FAILED"},
    {COAP_CODE_413_REQUEST_ENTITY_TOO_LARGE,"4.13-REQUEST_ENTITY_TOO_LARGE"},
    {COAP_CODE_415_UNSUPPORTED_CONTENT_FORMAT,"4.15-UNSUPPORTED_CONTENT_FORMAT"},
    {COAP_CODE_500_INTERNAL_SERVER_ERROR,"5.00-INTERNAL_SERVER_ERROR"},
    {COAP_CODE_501_NOT_IMPLEMENTED,"5.01-NOT_IMPLEMENTED"},
    {COAP_CODE_502_BAD_GATEWAY,"5.02-BAD_GATEWAY"},
    {COAP_CODE_503_SERVICE_UNAVAILABLE,"5.03-SERVICE_UNAVAILABLE"},
    {COAP_CODE_504_GATEWAY_TIMEOUT,"5.04-GATEWAY_TIMEOUT"},
    {COAP_CODE_505_PROXYING_NOT_SUPPORTED,"5.05-PROXYING_NOT_SUPPORTED"},
};

const char * tc_iot_coap_get_message_code_str(int code) {
	int i = 0;
	int map_size = sizeof(_tc_iot_coap_message_code_map)/sizeof(tc_iot_code_map);
    for (i = 0; i < map_size; i++) {
        if (code == _tc_iot_coap_message_code_map[i].code) {
            return _tc_iot_coap_message_code_map[i].str;
        }
    }

    return "";
}

static tc_iot_code_map _tc_iot_coap_option_number_map[] = {
    {COAP_OPTION_IF_MATCH,"1-IF_MATCH"},
    {COAP_OPTION_URI_HOST,"3-URI_HOST"},
    {COAP_OPTION_ETAG,"4-ETAG"},
    {COAP_OPTION_IF_NONE_MATCH,"5-IF_NONE_MATCH"},
    {COAP_OPTION_URI_PORT,"7-URI_PORT"},
    {COAP_OPTION_LOCATION_PATH,"8-LOCATION_PATH"},
    {COAP_OPTION_URI_PATH,"11-URI_PATH"},
    {COAP_OPTION_CONTENT_FORMAT,"12-CONTENT_FORMAT"},
    {COAP_OPTION_MAX_AGE,"14-MAX_AGE"},
    {COAP_OPTION_URI_QUERY,"15-URI_QUERY"},
    {COAP_OPTION_ACCEPT,"17-ACCEPT"},
    {COAP_OPTION_LOCATION_QUERY,"20-LOCATION_QUERY"},
    {COAP_OPTION_PROXY_URI,"35-PROXY_URI"},
    {COAP_OPTION_PROXY_SCHEME,"39-PROXY_SCHEME"},
    {COAP_OPTION_SIZE1,"60-SIZE1"},
};

const char * tc_iot_coap_get_option_number_str(int number) {
	int i = 0;
	int map_size = sizeof(_tc_iot_coap_option_number_map)/sizeof(tc_iot_code_map);
    for (i = 0; i < map_size; i++) {
        if (number == _tc_iot_coap_option_number_map[i].code) {
            return _tc_iot_coap_option_number_map[i].str;
        }
    }

    return "";
}


