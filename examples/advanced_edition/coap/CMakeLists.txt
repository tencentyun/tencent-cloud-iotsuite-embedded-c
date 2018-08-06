
add_executable(demo_coap_client coap_app_main.c tc_iot_coap_config_opts.c)
target_link_libraries(demo_coap_client tc_iot_coap tc_iot_common tc_iot_hal)


