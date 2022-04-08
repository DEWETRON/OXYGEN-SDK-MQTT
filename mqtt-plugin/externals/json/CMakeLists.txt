#
# JSON-Schema Validator and JSON Library (Validator automatically fetches nlohmann JSON)
# TODO: Switch to official repo once merged
FetchContent_Declare(
    nlohmann_json_schema_validator
    GIT_REPOSITORY https://github.com/vrince/json-schema-validator.git
    GIT_TAG cmake-clean-up
)

FetchContent_MakeAvailable(nlohmann_json_schema_validator)

set(JSON_VALIDATOR_BUILD_TESTS OFF CACHE INTERNAL "")
set(JSON_VALIDATOR_BUILD_EXAMPLES OFF CACHE INTERNAL "")

target_link_libraries(${PROJECT_NAME} PUBLIC
    nlohmann_json
    nlohmann_json_schema_validator
)
