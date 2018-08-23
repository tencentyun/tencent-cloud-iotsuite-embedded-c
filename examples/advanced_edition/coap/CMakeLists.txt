
add_executable(advanced_coap coap_app_main.c tc_iot_coap_config_opts.c)
target_link_libraries(advanced_coap tc_iot_coap tc_iot_common tc_iot_hal)


