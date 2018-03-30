@echo off 
set cc_options=-c --cpu Cortex-M3 -g -O0 --apcs=interwork --split_sections -I ../include -I ../include/platform/generic
set cc_options=%cc_options% -DENABLE_TC_IOT_LOG_TRACE -DENABLE_TC_IOT_LOG_DEBUG -DENABLE_TC_IOT_LOG_INFO -DENABLE_TC_IOT_LOG_WARN -DENABLE_TC_IOT_LOG_ERROR -DENABLE_TC_IOT_LOG_FATAL
REM set cc_options=%cc_options% -W
REM set cc_options=%cc_options% --diag_suppress 177

armcc %cc_options% ..\src\common\tc_iot_hmac.c
armcc %cc_options% ..\src\common\tc_iot_json.c
armcc %cc_options% ..\src\common\tc_iot_url.c
armcc %cc_options% ..\src\common\tc_iot_base64.c
armcc %cc_options% ..\src\common\tc_iot_http_utils.c
armcc %cc_options% ..\src\common\tc_iot_log.c
armcc %cc_options% ..\src\common\tc_iot_ya_buffer.c

armcc %cc_options% ..\src\iotsuite\tc_iot_client.c
armcc %cc_options% ..\src\iotsuite\tc_iot_shadow.c
armcc %cc_options% ..\src\iotsuite\tc_iot_certs.c
armcc %cc_options% ..\src\iotsuite\tc_iot_mqtt.c
armcc %cc_options% ..\src\iotsuite\tc_iot_token.c

armcc %cc_options% ..\src\platform\generic\tc_iot_hal_os.c
armcc %cc_options% ..\src\platform\generic\tc_iot_hal_tls.c
armcc %cc_options% ..\src\platform\generic\tc_iot_hal_net.c
armcc %cc_options% ..\src\platform\generic\tc_iot_hal_timer.c
