
aux_source_directory(. TC_IOT_OTA_SRCS)
add_library(tc_iot_ota STATIC ${TC_IOT_OTA_SRCS})
target_link_libraries(tc_iot_ota tc_iot_common tc_iot_hal)


