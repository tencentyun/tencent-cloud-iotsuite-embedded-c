list(APPEND tc_iot_hal_srcs tc_iot_hal_util.c tc_iot_hal_os.c tc_iot_hal_timer.c)

if (ENABLE_MQTT)
    if(ENABLE_TLS)
        list(APPEND tc_iot_hal_srcs tc_iot_hal_net.c tc_iot_hal_tls.c)
    else()
        list(APPEND tc_iot_hal_srcs tc_iot_hal_net.c)
    endif()
endif()

if (ENABLE_COAP)
    if(ENABLE_DTLS)
        list(APPEND tc_iot_hal_srcs tc_iot_hal_udp.c tc_iot_hal_dtls.c)
    else()
        list(APPEND tc_iot_hal_srcs tc_iot_hal_udp.c)
    endif()
endif()



add_library(tc_iot_hal STATIC ${tc_iot_hal_srcs})


