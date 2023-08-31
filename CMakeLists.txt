cmake_minimum_required(VERSION 3.20)

project(mqttplugin)

IF (MSVC)
ELSE()
    LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
ENDIF()

#
# Default: Disable Plugin/Firmware-Tests
option(BUILD_PLUGIN_WITH_TESTS "Build plugin with tests" OFF)

if(BUILD_PLUGIN_WITH_TESTS)
    if (WIN32)
        # Ensure CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS is TRUE when Building with tests
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
endif()

add_subdirectory(mqtt-plugin)

if(BUILD_PLUGIN_WITH_TESTS)
    add_subdirectory(tests)
    add_dependencies(mqttplugin_test mqtt)
endif()
