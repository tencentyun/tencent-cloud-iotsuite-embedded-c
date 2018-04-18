
aux_source_directory(. IOT_TESTS_SRCS)
add_executable(unit_test ${IOT_TESTS_SRCS})
target_link_libraries(unit_test tc_iot_common)

add_executable(st_udp_client st/st_udp_client.c)
target_link_libraries(st_udp_client tc_iot_common tc_iot_suite tc_iot_hal mbedtls)

add_executable(st_coap st/st_coap.c)
target_link_libraries(st_coap tc_iot_common tc_iot_coap tc_iot_hal mbedtls)

add_executable(st_download st/st_download.c)
target_link_libraries(st_download tc_iot_common tc_iot_suite tc_iot_hal mbedtls)

add_executable(st_md5 st/st_md5.c)
target_link_libraries(st_md5 tc_iot_common tc_iot_suite tc_iot_hal mbedtls)
