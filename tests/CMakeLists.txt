project(iot_test)

include_directories(../external/cpputest/include)

aux_source_directory(. IOT_TESTS_SRCS)
add_executable(tc_iot_test_runner ${IOT_TESTS_SRCS})
target_link_libraries(tc_iot_test_runner CppUTest CppUTestExt)

install(TARGETS tc_iot_test_runner DESTINATION ${TEST_INSTALL_DIR})

