
add_executable(basic_coap coap_app_main.c tc_iot_coap_config_opts.c)
target_link_libraries(basic_coap tc_iot_coap tc_iot_common tc_iot_hal)


