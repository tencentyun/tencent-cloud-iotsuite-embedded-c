if(ENABLE_TLS)
    add_library(tc_iot_hal STATIC tc_iot_hal_os.c tc_iot_hal_timer.c tc_iot_hal_net.c tc_iot_hal_tls.c)
else()
    add_library(tc_iot_hal STATIC tc_iot_hal_os.c tc_iot_hal_timer.c tc_iot_hal_net.c)
endif()


