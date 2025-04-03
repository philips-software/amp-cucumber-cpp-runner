FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG "79587f896ed4db2a0b63baa9151113f4da336599" # Unreleased

    OVERRIDE_FIND_PACKAGE
)

set(JSON_Install "On")
set(JSON_BuildTests "Off")

FetchContent_MakeAvailable(nlohmann_json)
