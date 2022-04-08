cmake_minimum_required(VERSION 3.20)

project(mqttplugin)

#
# Default: Disable Plugin/Firmware-Tests
option(BUILD_PLUGIN_WITH_TESTS "Build plugin with tests." OFF)

if(BUILD_PLUGIN_WITH_TESTS)
    # Ensure CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS is TRUE when Building with tests
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

add_subdirectory(mqtt-plugin)

if(BUILD_PLUGIN_WITH_TESTS)
    add_subdirectory(tests)
    add_dependencies(mqttplugin_test mqtt)
endif()
