
aux_source_directory(. IOT_COAP_SRCS)
add_library(tc_iot_coap STATIC ${IOT_COAP_SRCS})
target_link_libraries(tc_iot_coap tc_iot_common tc_iot_hal)


