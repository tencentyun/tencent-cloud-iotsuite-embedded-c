
aux_source_directory(. IOT_TESTS_SRCS)
add_executable(unit_test ${IOT_TESTS_SRCS})
target_link_libraries(unit_test tc_iot_common)

