if (ENABLE_MQTT)
    ADD_SUBDIRECTORY(mqtt)
endif()

if(ENABLE_COAP)
    ADD_SUBDIRECTORY(coap)
endif()

