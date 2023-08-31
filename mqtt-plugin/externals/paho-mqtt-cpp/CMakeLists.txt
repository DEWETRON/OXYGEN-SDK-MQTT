
IF (MSVC)
    message(STATUS "Setting up PAHO MQTT for Windows")
    set(PAHO_BUILD_SHARED OFF CACHE INTERNAL "")
    set(PAHO_BUILD_STATIC ON CACHE INTERNAL "")
    set(PAHO_ENABLE_TESTING OFF CACHE INTERNAL "")
    set(PAHO_BUILD_TESTS OFF CACHE INTERNAL "")
    set(PAHO_WITH_MQTT_C ON CACHE INTERNAL "")

    #
    # TODO Change to official paho.mqtt.cpp repository once patches to CMake build system have been accepted
    FetchContent_Declare(
        paho-mqttpp3-static
        GIT_REPOSITORY https://github.com/HpLightcorner/paho.mqtt.cpp.git
        GIT_TAG mqtt-c-submodule
    )

    FetchContent_MakeAvailable(paho-mqttpp3-static)
    target_include_directories(${PROJECT_NAME} INTERFACE
        ${paho-mqttpp3-static_SOURCE_DIR}/src
    )

    target_link_libraries(${PROJECT_NAME} PUBLIC
        paho-mqttpp3-static
    )
ELSE()
    message(STATUS "Setting up PAHO MQTT for Linux")
    find_package(PahoMqttC REQUIRED)
    find_package(PahoMqttCpp REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        PahoMqttC::PahoMqttC
        PahoMqttCpp::PahoMqttCpp
    )
ENDIF()
