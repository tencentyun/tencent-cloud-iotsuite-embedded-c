project(iot_test)

aux_source_directory(. IOT_TESTS_SRCS)
add_executable(tc_iot_test_runner ${IOT_TESTS_SRCS})

