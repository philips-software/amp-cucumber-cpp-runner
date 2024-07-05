FetchContent_Declare(cucumber_gherkin
    GIT_REPOSITORY https://github.com/cucumber/gherkin.git
    GIT_TAG "main"
)

FetchContent_MakeAvailable(cucumber_gherkin)

add_subdirectory(${cucumber_gherkin_SOURCE_DIR}/cpp ${cucumber_gherkin_BINARY_DIR}/cpp)

target_compile_options(cucumber_messages_lib PUBLIC
    # cucumber_gherkin-src/cpp/include/gherkin/cucumber/gherkin/utils.hpp:34:31: error: 'codecvt_utf8<char32_t>' is deprecated
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
)
