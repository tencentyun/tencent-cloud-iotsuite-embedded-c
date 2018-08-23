#ifndef TC_IOT_INC_H
#define TC_IOT_INC_H

#include "tc_iot_config.h"

#include "tc_iot_external.h"
#include "tc_iot_platform.h"

#include "common/tc_iot_log.h"
#include "common/tc_iot_const.h"
#include "common/tc_iot_bit.h"
#include "common/tc_iot_safety_check.h"
#include "common/tc_iot_string_utils.h"
#include "common/tc_iot_md5.h"
#include "common/tc_iot_hmac.h"
#include "platform/tc_iot_hal_os.h"
#include "platform/tc_iot_hal_timer.h"
#include "platform/tc_iot_hal_network.h"

#include "common/tc_iot_ya_buffer.h"
#include "common/tc_iot_base64.h"
#include "common/tc_iot_json.h"
#include "common/tc_iot_url.h"
#include "common/tc_iot_sign_utils.h"
#include "common/tc_iot_http_utils.h"

#include "common/tc_iot_certs.h"

#if defined(ENABLE_MQTT)
#include "mqtt/tc_iot_mqtt.h"
#include "mqtt/tc_iot_client.h"
#include "iotsuite/tc_iot_shadow.h"
#include "http/tc_iot_token.h"
#endif

#if defined(ENABLE_COAP)
#include "coap/tc_iot_coap.h"
#endif

#if defined(ENABLE_OTA)
#include "ota/tc_iot_ota_mqtt.h"
#include "ota/tc_iot_ota_download.h"
#endif

#endif /* end of include guard */
