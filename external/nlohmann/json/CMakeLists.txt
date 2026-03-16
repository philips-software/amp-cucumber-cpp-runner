FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 55f93686c01528224f448c19128836e7df245f72 # v3.12.0
    OVERRIDE_FIND_PACKAGE
)

# required for cucumber/messages and cucumber/gherkin
set(JSON_Install "On")

FetchContent_MakeAvailable(nlohmann_json)
