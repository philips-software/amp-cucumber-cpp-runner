FetchContent_Declare(cucumber_messages
    GIT_REPOSITORY https://github.com/cucumber/messages.git
    GIT_TAG "v26.0.1"

    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(cucumber_messages)

add_subdirectory(${cucumber_messages_SOURCE_DIR}/cpp ${cucumber_messages_BINARY_DIR}/cpp)

target_compile_options(cucumber_messages_lib PUBLIC
    # cucumber_gherkin-src/cpp/include/gherkin/cucumber/gherkin/utils.hpp:34:31: error: 'codecvt_utf8<char32_t>' is deprecated
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
)
