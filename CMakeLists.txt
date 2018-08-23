cmake_minimum_required(VERSION 2.8)
project (tencent-cloud-iotsuite-embedded-c C)

set(CMAKE_BUILD_TYPE Debug)
# sdk use c compiler, use this to disable find cxx compiler.
# set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

if(TC_IOT_PLATFORM_NAME)
else()
    set(TC_IOT_PLATFORM_NAME "linux")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS, 1)

# add_definitions(-Werror=pointer-arith)
add_definitions(-Wall)

message(STATUS "CMake version: " ${CMAKE_VERSION})
message(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})
message(STATUS "TC_IOT_PLATFORM_NAME: " ${TC_IOT_PLATFORM_NAME})

set(CMAKE_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

## build settings
set(TC_IOT_VERSION_MAJOR 1)
set(TC_IOT_VERSION_MINOR 0)
set(TC_IOT_VERSION_PATCH 0)
set(CLIENT_VERSION ${TC_IOT_VERSION_MAJOR}.${TC_IOT_VERSION_MINOR}${TC_IOT_VERSION_PATCH})

string(TIMESTAMP BUILD_TIMESTAMP UTC)
message(STATUS "Timestamp is ${BUILD_TIMESTAMP}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(COPY tools/tc_iot_code_generator.py DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

option(ENABLE_MQTT "use MQTT" ON)
option(ENABLE_DATA_TEMPLATE "use Data Template" ON)
option(ENABLE_COAP "use CoAP" ON)
option(ENABLE_OTA "use OTA" ON)

option(ENABLE_TLS "use mqtt/http over tls" ON)
option(ENABLE_DTLS "use CoAP over dtls" OFF)

option(ENABLE_STACK_TRACE_LOG "Enable stack big mem usage" ON)
option(ENABLE_MQTT_RECONNECT_FOREVER "Enable mqtt reconnect forever" ON)

option(ENABLE_TC_IOT_LOG_TRACE "Enable log" ON)
option(ENABLE_TC_IOT_LOG_DEBUG "Enable log" ON)
option(ENABLE_TC_IOT_LOG_INFO  "Enable log" ON)
option(ENABLE_TC_IOT_LOG_WARN  "Enable log" ON)
option(ENABLE_TC_IOT_LOG_ERROR  "Enable log" ON)
option(ENABLE_TC_IOT_LOG_FATAL  "Enable log" ON)

if(${TC_IOT_PLATFORM_NAME} STREQUAL "generic")
    set(ENABLE_TLS  OFF)
    set(ENABLE_DTLS  OFF)
endif()

option(ENABLE_TESTING "enable testing" OFF)
option(ENABLE_GTEST   "enable gtest" OFF)

if(LIB_INSTALL_DIR)
else()
    set(LIB_INSTALL_DIR lib)
endif()

if(TEST_INSTALL_DIR)
else()
    set(TEST_INSTALL_DIR test)
endif()


if(ENABLE_TLS OR ENABLE_DTLS)
    # add mbedtls
    include_directories(external/mbedtls/include)
endif()

# Linux, Windows, and Darwin for Mac OS X are the values found on the big three
# operating systems.
# And Generic for other systems.
SET(CMAKE_SYSTEM_NAME Linux)

include_directories(include)
include_directories(include/platform/${TC_IOT_PLATFORM_NAME})

configure_file ("${CMAKE_SOURCE_DIR}/include/tc_iot_compile_flags.h.in"
    "${CMAKE_SOURCE_DIR}/include/tc_iot_compile_flags.h")

# enable_testing()
ADD_SUBDIRECTORY(external)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(src)

if(ENABLE_TESTING)
enable_testing()

    if(ENABLE_GTEST)
# Download and unpack googletest at configure time
	configure_file(tests/gtest/CMakeLists.txt.gtest.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
	add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
					"${CMAKE_BINARY_DIR}/googletest-build")

    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate
endif()

	ADD_SUBDIRECTORY(tests)
endif()

