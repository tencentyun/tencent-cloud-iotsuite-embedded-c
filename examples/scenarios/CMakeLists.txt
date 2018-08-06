if (ENABLE_MQTT)
    if(ENABLE_DATA_TEMPLATE)
        ADD_SUBDIRECTORY(light)
        ADD_SUBDIRECTORY(smartbox)
    endif()

    if (ENABLE_OTA)
        ADD_SUBDIRECTORY(ota)
    endif()
endif()

