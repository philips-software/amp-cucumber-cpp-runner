FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 56e3bb550c91fd7005566f19c079cb7a503223cf # yaml-cpp-0.9.0
)

set(YAML_ENABLE_PIC OFF CACHE STRING "")

FetchContent_MakeAvailable(yaml-cpp)
