FetchContent_Declare(cucumber_gherkin
    GIT_REPOSITORY https://github.com/cucumber/gherkin.git
    GIT_TAG "0022bb07791485cda296e1785f4d0de47a04e5c9"
)

FetchContent_MakeAvailable(cucumber_gherkin)

add_subdirectory(${cucumber_gherkin_SOURCE_DIR}/cpp ${cucumber_gherkin_BINARY_DIR}/cpp)

# The usage of designated initializers requires c++20 in MSVC
set_target_properties(cucumber_gherkin_lib PROPERTIES CXX_STANDARD 20)
set_target_properties(cucumber_gherkin_bin PROPERTIES CXX_STANDARD 20)
set_target_properties(cucumber_gherkin_generate_tokens_bin PROPERTIES CXX_STANDARD 20)
