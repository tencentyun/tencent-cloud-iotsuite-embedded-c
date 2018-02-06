
aux_source_directory(. IOT_COMMON_SRCS)
add_library(tc_iot_common STATIC ${IOT_COMMON_SRCS})
target_link_libraries(tc_iot_common jsmn)

