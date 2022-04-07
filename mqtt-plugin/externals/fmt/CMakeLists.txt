FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)

FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
