FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG "bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d" # v3.11.2

    OVERRIDE_FIND_PACKAGE
)

set(JSON_Install "On")
set(JSON_BuildTests "Off")

FetchContent_MakeAvailable(nlohmann_json)
