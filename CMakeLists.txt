cmake_minimum_required(VERSION 2.8)
project (tencent-cloud-iotsuite-embedded-c)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-DENABLE_LOG_TRACE)
add_definitions(-DENABLE_LOG_DEBUG)
add_definitions(-DENABLE_LOG_WARN)
add_definitions(-DENABLE_LOG_ERROR)
add_definitions(-DENABLE_LOG_CRIT)

message(STATUS "CMake version: " ${CMAKE_VERSION})
message(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})

set(CMAKE_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

## build settings
set(IOT_VERSION_MAJOR 1)
set(IOT_VERSION_MINOR 0)
set(IOT_VERSION_PATCH 0)
set(CLIENT_VERSION ${IOT_VERSION_MAJOR}.${IOT_VERSION_MINOR}.${IOT_VERSION_PATCH})

string(TIMESTAMP BUILD_TIMESTAMP UTC)
message(STATUS "Timestamp is ${BUILD_TIMESTAMP}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(ENABLE_TLS "use mqtt/http over tls" ON) 
option(ENABLE_TESTING "enable testting" OFF)

# 
set(platform_name "linux")

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
include_directories(include/common)
include_directories(include/platform)
include_directories(include/platform/${platform_name})
include_directories(external/paho.mqtt.embedded-c/MQTTPacket/src)

add_definitions(-DJSMN_PARENT_LINKS)
#add_definitions(-std=gnu99)

include_directories(external/jsmn/)

# enable_testing()
ADD_SUBDIRECTORY(external)
ADD_SUBDIRECTORY(samples)
ADD_SUBDIRECTORY(src)

if(ENABLE_TESTING)
    ADD_SUBDIRECTORY(tests)
endif()

