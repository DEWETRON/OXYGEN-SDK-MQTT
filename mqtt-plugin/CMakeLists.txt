cmake_minimum_required(VERSION 3.20)

#
# Include some helpers and the fetchcontent module for external dependencies
include(FetchContent)

#
# Define file-generate for relative paths
if(POLICY CMP0070)
   cmake_policy(SET CMP0070 NEW)
endif()

#
# Activate Policy for Static-Runtime 
if(POLICY CMP0091)
   cmake_policy(SET CMP0091 NEW)
endif()

#
# Activate Policy for option() honors normal variables
if(POLICY CMP0077)
   cmake_policy(SET CMP0077 NEW)
   set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()

#
# Project
project(mqtt)

#
# C++ Standard, Compiler Flags
set(CMAKE_CXX_STANDARD 17)

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

#
# Oxygen SDK
add_subdirectory(externals/oxygen-sdk)
set(ODK_ROOT ${OXYGEN_SDK_ROOT})
message("ODKROOT = ${ODK_ROOT}")

# expand cmake search path to check for project settings
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ODK_ROOT}/cmake )

include(CMakeSettings)
include(OxygenPluginFunctions)

# Set Static Runtime 
set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

SetCommonOutputDirectory()
#SetBoostOptions()
#find_package(Boost REQUIRED)
find_package(SDKQt REQUIRED)

message(STATUS "Qt5Core_RCC_EXECUTABLE = ${Qt5Core_RCC_EXECUTABLE}")

AddUniqueTargetFromSubdirectory(pugixml "${SW_APP_ROOT}/3rdparty/pugixml-1.9/scripts" "3rdparty/pugixml-1.9")
AddUniqueTargetFromSubdirectory(odk "${ODK_ROOT}/odk" "odk")

set(MQTT_PLUGIN_HEADER_FILES
    include/Service.h 
    include/subscription/Subscription.h
    include/subscription/Channel.h
    include/subscription/decoding/Decoder.h
    include/subscription/decoding/TextPlainDecoder.h
    include/subscription/decoding/TextJsonDecoder.h
    include/subscription/decoding/CborSyncDecoder.h
    include/publish/Publish.h 
    include/configuration/Configuration.h
    include/configuration/Server.h
    include/configuration/Topic.h
    include/configuration/details/Schema.h
    include/resampling/StreamClock.h
    include/resampling/Stream.h
)
source_group("Header Files" FILES ${MQTT_PLUGIN_HEADER_FILES})

set(MQTT_PLUGIN_SOURCE_FILES
    src/MqttPlugin.cpp
    src/Service.cpp
    src/subscription/Subscription.cpp
    src/subscription/Channel.cpp
    src/subscription/decoding/CborSyncDecoder.cpp
    src/publish/Publish.cpp 
    src/configuration/Configuration.cpp
    src/configuration/Topic.cpp
    src/configuration/Server.cpp
    src/resampling/StreamClock.cpp
    src/resampling/Stream.cpp
    src/Utility.cpp
)
source_group("Source Files" FILES ${MQTT_PLUGIN_SOURCE_FILES})

set(MQTT_PLUGIN_QML_FILES
    qml/AddMqttPlugin.qml
)
source_group("QML Files" FILES ${MQTT_PLUGIN_QML_FILES})


add_library(${PROJECT_NAME} SHARED
    ${MQTT_PLUGIN_HEADER_FILES}
    ${MQTT_PLUGIN_SOURCE_FILES}
    ${MQTT_PLUGIN_QML_FILES}
)

#
# Set C++ Standard to 17
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC odk_framework
)

target_include_directories(${PROJECT_NAME}
    PUBLIC include
    SYSTEM
    #${Boost_INCLUDE_DIRS}
)

#
# Add QML Resources
qt5_add_binary_resources(${PROJECT_NAME}_qml qml.qrc DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/qml.rcc")

set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${RESOURCE_DIR}
)
AddResourceFile(${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/qml.rcc" ${RESOURCE_DIR})

#
# Add Dependencies 
add_subdirectory(externals)

#
# General-Plugin Options
SetPluginOutputOptions(${PROJECT_NAME})

#
# Cache the Plugin-Output-Directory to allow a test-executeable to set its Working-Directory
# The Output Directory is set by SetPluginOutputOptions
# Generator-Expressions are allowed in CTest WORKING_DIRECTORY
IF(MSVC_IDE)
    set(OXYGEN_PLUGIN_OUTPUT_DIRECTORY_GENERATOR_EXPR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/plugins CACHE INTERNAL "")
ELSE(MSVC_IDE)
    set(OXYGEN_PLUGIN_OUTPUT_DIRECTORY_GENERATOR_EXPR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins CACHE INTERNAL "")
ENDIF(MSVC_IDE)
