# find the Paho MQTT Cpp library
set(_PAHO_MQTT_CPP_LIB_NAME paho-mqttpp3)

# add suffix when using static Paho MQTT C library variant on Windows
if(WIN32)
    if(PAHO_BUILD_STATIC)
        set(_PAHO_MQTT_CPP_LIB_NAME ${_PAHO_MQTT_CPP_LIB_NAME}-static)
    endif()
endif()

find_library(PAHO_MQTT_CPP_LIBRARIES NAMES ${_PAHO_MQTT_CPP_LIB_NAME})
unset(_PAHO_MQTT_CPP_LIB_NAME)
find_path(PAHO_MQTT_CPP_INCLUDE_DIRS NAMES mqtt/async_client.h)

add_library(PahoMqttCpp::PahoMqttCpp UNKNOWN IMPORTED)

set_target_properties(PahoMqttCpp::PahoMqttCpp PROPERTIES
    IMPORTED_LOCATION "${PAHO_MQTT_CPP_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${PAHO_MQTT_CPP_INCLUDE_DIRS}"
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PahoMqttCpp
    REQUIRED_VARS PAHO_MQTT_CPP_LIBRARIES PAHO_MQTT_CPP_INCLUDE_DIRS)
