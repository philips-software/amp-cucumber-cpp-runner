FetchContent_Declare(
    cucumber_gherkin
    GIT_REPOSITORY https://github.com/cucumber/gherkin.git
    GIT_TAG 6f0040fee2783fdb455d345e5bdb50d425515b5d # cpp-remove-cmate-use-native-cmake-and-CPM
)

FetchContent_MakeAvailable(cucumber_gherkin)

add_subdirectory(${cucumber_gherkin_SOURCE_DIR}/cpp ${cucumber_gherkin_BINARY_DIR}/cpp)

# The usage of designated initializers requires c++20 in MSVC
set_target_properties(cucumber_gherkin_lib PROPERTIES CXX_STANDARD 20)
set_target_properties(cucumber_gherkin_bin PROPERTIES CXX_STANDARD 20)
set_target_properties(cucumber_gherkin_generate_tokens_bin PROPERTIES CXX_STANDARD 20)

# Mark cucumber_gherkin_lib include directories as SYSTEM so that deprecation
# warnings in third-party headers (e.g. std::wstring_convert in utils.hpp)
# are not raised as errors in consuming targets.
get_target_property(_gherkin_includes cucumber_gherkin_lib INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(cucumber_gherkin_lib PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_gherkin_includes}")
