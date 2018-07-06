
add_executable(demo_ota app_main.c tc_iot_ota_logic.c ../tc_iot_client_config_opts.c)
target_link_libraries(demo_ota tc_iot_ota tc_iot_mqtt_client tc_iot_token tc_iot_common tc_iot_hal jsmn MQTTPacketClient)


