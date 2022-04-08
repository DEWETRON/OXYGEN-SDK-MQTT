cmake_minimum_required(VERSION 3.20)
Include(FetchContent)

project(mqttplugin_test)
set(CMAKE_CXX_STANDARD 17)

if( MSVC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

#
# Add Dependencies
add_subdirectory(externals)

#
# The Tests
add_executable(${PROJECT_NAME} TestResampler.cpp TestPublishDownsampling.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE mqtt Catch2::Catch2WithMain)

#
# Set C++ Standard to 17
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

#
# Register Tests for CTest
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Catch2_SOURCE_DIR}/extras/)
include(CTest)
include(Catch)

message(STATUS "When using CTest, tests will be executed in the following working directory:")
message(STATUS "${OXYGEN_PLUGIN_OUTPUT_DIRECTORY_GENERATOR_EXPR}")
message(STATUS "Generator-Expressions will be handeld by CTest.")

catch_discover_tests(${PROJECT_NAME} WORKING_DIRECTORY ${OXYGEN_PLUGIN_OUTPUT_DIRECTORY_GENERATOR_EXPR})
