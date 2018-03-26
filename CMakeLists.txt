cmake_minimum_required(VERSION 2.8)
project (tencent-cloud-iotsuite-embedded-c)

set(CMAKE_BUILD_TYPE Debug)

if(TC_IOT_PLATFORM_NAME)
else()
    set(TC_IOT_PLATFORM_NAME "linux")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS, 1)

add_definitions(-DENABLE_LOG_TRACE)
add_definitions(-DENABLE_LOG_DEBUG)
add_definitions(-DENABLE_LOG_INFO)
add_definitions(-DENABLE_LOG_WARN)
add_definitions(-DENABLE_LOG_ERROR)
add_definitions(-DENABLE_LOG_FATAL)

message(STATUS "CMake version: " ${CMAKE_VERSION})
message(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})
message(STATUS "TC_IOT_PLATFORM_NAME: " ${TC_IOT_PLATFORM_NAME})

set(CMAKE_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

## build settings
set(TC_IOT_VERSION_MAJOR 1)
set(TC_IOT_VERSION_MINOR 0)
set(TC_IOT_VERSION_PATCH 0)
set(CLIENT_VERSION ${TC_IOT_VERSION_MAJOR}.${TC_IOT_VERSION_MINOR}.${TC_IOT_VERSION_PATCH})

string(TIMESTAMP BUILD_TIMESTAMP UTC)
message(STATUS "Timestamp is ${BUILD_TIMESTAMP}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(COPY tools/tc_iot_shadow_cli.py DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY tools/tc_iot_code_generator.py DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


if(${TC_IOT_PLATFORM_NAME} STREQUAL "generic")
    option(ENABLE_TLS "use mqtt/http over tls" OFF) 
    add_definitions(-std=c89)
else()
    option(ENABLE_TLS "use mqtt/http over tls" ON) 
endif()

option(ENABLE_TESTING "enable testting" ON)

if(LIB_INSTALL_DIR)
else()
    set(LIB_INSTALL_DIR lib)
endif()

if(TEST_INSTALL_DIR)
else()
    set(TEST_INSTALL_DIR test)
endif()


if(ENABLE_TLS)
    add_definitions(-DENABLE_TLS)
    # add mbedtls
    include_directories(external/mbedtls/include)

endif()

# Linux, Windows, and Darwin for Mac OS X are the values found on the big three
# operating systems.
# And Generic for other systems.
SET(CMAKE_SYSTEM_NAME Linux)

include_directories(include)
include_directories(include/platform/${TC_IOT_PLATFORM_NAME})

# enable_testing()
ADD_SUBDIRECTORY(external)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(src)

if(ENABLE_TESTING)
    ADD_SUBDIRECTORY(tests)
endif()

