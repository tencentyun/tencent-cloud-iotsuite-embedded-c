
list(APPEND DEMO_LIGHT_SRCS app_main.c tc_iot_device_logic.c tc_iot_client_config_opts.c)
list(APPEND DEMO_LIGHT_LIBS tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)

add_executable(scn_light ${DEMO_LIGHT_SRCS})
target_link_libraries(scn_light ${DEMO_LIGHT_LIBS})

