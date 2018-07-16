
add_executable(test_http gtest.cpp tc_iot_http_test.cpp)
# Standard linking to gtest stuff.
target_link_libraries(test_http stdc++ pthread gtest gtest_main tc_iot_common )
add_test(NAME run_test_http COMMAND test_http)

add_executable(test_mqtt gtest.cpp tc_iot_mqtt_test.cpp)
target_link_libraries(test_mqtt stdc++ pthread gtest gtest_main tc_iot_mqtt_client tc_iot_common tc_iot_token )
add_test(NAME run_test_mqtt COMMAND test_mqtt)

add_executable(test_iotsuite gtest.cpp data_template/tc_iot_suite_test.cpp
    data_template/tc_iot_device_logic.c)
target_link_libraries(test_iotsuite stdc++ pthread gtest gtest_main tc_iot_suite tc_iot_common tc_iot_token )
add_test(NAME run_test_iotsuite COMMAND test_iotsuite)

