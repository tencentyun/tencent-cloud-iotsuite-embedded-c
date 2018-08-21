
# AUX_SOURCE_DIRECTORY(. BASIC_TEST_SRCS)
# AUX_SOURCE_DIRECTORY(data_template DATATEMPLATE_TEST_SRCS)
#
# add_executable(all_test ${BASIC_TEST_SRCS} ${DATATEMPLATE_TEST_SRCS})
# target_link_libraries(all_test stdc++ pthread gtest gtest_main tc_iot_suite tc_iot_common tc_iot_token )
# add_test(NAME all_test COMMAND all_test)

add_executable(test_http gtest.cpp tc_iot_http_test.cpp)
# Standard linking to gtest stuff.
target_link_libraries(test_http stdc++ pthread gtest gtest_main tc_iot_common )
add_test(NAME test_http COMMAND test_http)

add_executable(test_shadow gtest.cpp data_template/tc_iot_shadow_test.cpp)
target_link_libraries(test_shadow stdc++ pthread gtest gtest_main tc_iot_suite tc_iot_common tc_iot_token )
add_test(NAME test_shadow COMMAND test_shadow)

add_executable(test_mqtt gtest.cpp tc_iot_mqtt_test.cpp)
target_link_libraries(test_mqtt stdc++ pthread gtest gtest_main tc_iot_mqtt_client tc_iot_common tc_iot_token )
add_test(NAME test_mqtt COMMAND test_mqtt)

add_executable(test_iotsuite gtest.cpp data_template/tc_iot_suite_test.cpp
    data_template/tc_iot_device_logic.cpp)
target_link_libraries(test_iotsuite stdc++ pthread gtest gtest_main tc_iot_suite tc_iot_common tc_iot_token )
add_test(NAME test_iotsuite COMMAND test_iotsuite)

add_executable(test_mqapi gtest.cpp data_template/tc_iot_mqapi_test.cpp)
target_link_libraries(test_mqapi stdc++ pthread gtest gtest_main tc_iot_suite
    tc_iot_common tc_iot_token tc_iot_http_mqapi)
add_test(NAME test_mqapi COMMAND test_mqapi)

