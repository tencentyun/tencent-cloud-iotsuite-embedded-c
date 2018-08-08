
list(APPEND DEMO_LIGHT_SRCS app_main.c tc_iot_device_logic.c ../tc_iot_client_config_opts.c)
list(APPEND DEMO_LIGHT_LIBS tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)

if (ENABLE_OTA) 
    list(APPEND DEMO_LIGHT_SRCS ../ota/tc_iot_ota_logic.c)
    list(APPEND DEMO_LIGHT_LIBS tc_iot_ota)
endif()

add_executable(light ${DEMO_LIGHT_SRCS})
target_link_libraries(light ${DEMO_LIGHT_LIBS})

