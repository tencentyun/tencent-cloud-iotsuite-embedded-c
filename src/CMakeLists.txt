
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(platform)

if (ENABLE_MQTT)
    ADD_SUBDIRECTORY(iotsuite)
endif()

if (ENABLE_COAP)
    ADD_SUBDIRECTORY(coap)
endif()

if (ENABLE_OTA)
    ADD_SUBDIRECTORY(ota)
endif()

