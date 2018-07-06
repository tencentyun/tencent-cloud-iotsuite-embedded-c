
add_executable(runUnitTests gtest.cpp tc_iot_http_test.cpp)

# Standard linking to gtest stuff.
target_link_libraries(runUnitTests stdc++ pthread gtest gtest_main tc_iot_common )
add_test(NAME runUnitTests COMMAND runUnitTests)

