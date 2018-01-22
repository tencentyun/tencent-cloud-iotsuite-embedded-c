
set(path_pack_dir "paho.mqtt.embedded-c/MQTTPacket/src")
add_library(MQTTPacketClient  
            ${path_pack_dir}/MQTTFormat.c ${path_pack_dir}/MQTTPacket.c
            ${path_pack_dir}/MQTTSerializePublish.c ${path_pack_dir}/MQTTDeserializePublish.c
            ${path_pack_dir}/MQTTConnectClient.c ${path_pack_dir}/MQTTSubscribeClient.c ${path_pack_dir}/MQTTUnsubscribeClient.c)
target_compile_definitions(MQTTPacketClient PRIVATE MQTT_CLIENT)


if(ENABLE_TLS)
    add_definitions(-DENABLE_TLS)

    option(USE_STATIC_MBEDTLS_LIBRARY "Build mbed TLS static library." ON)
    option(USE_SHARED_MBEDTLS_LIBRARY "Build mbed TLS shared library." OFF)
    option(LINK_WITH_PTHREAD "Explicitly link mbed TLS library to pthread." OFF)

    # add mbedtls
    ADD_SUBDIRECTORY(mbedtls/library)
endif()


#jsmn 
add_library(jsmn STATIC jsmn/jsmn.c)

