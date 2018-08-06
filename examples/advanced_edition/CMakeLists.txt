if (ENABLE_MQTT)
    if(ENABLE_DATA_TEMPLATE)
        ADD_SUBDIRECTORY(mqtt)
        ADD_SUBDIRECTORY(http)
    endif()
endif()

if(ENABLE_COAP)
    ADD_SUBDIRECTORY(coap)
endif()

