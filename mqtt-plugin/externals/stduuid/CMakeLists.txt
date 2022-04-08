FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
)

set(UUID_BUILD_TESTS OFF CACHE INTERNAL "")
set(UUID_SYSTEM_GENERATOR ON CACHE INTERNAL "")

FetchContent_MakeAvailable(stduuid)
target_link_libraries(${PROJECT_NAME} PRIVATE stduuid)
