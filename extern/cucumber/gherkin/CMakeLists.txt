FetchContent_Declare(cucumber_gherkin
    GIT_REPOSITORY https://github.com/daantimmer/gherkin.git
    GIT_TAG "fix-compilation-issues-with-clang"
)

FetchContent_MakeAvailable(cucumber_gherkin)

add_subdirectory(${cucumber_gherkin_SOURCE_DIR}/cpp ${cucumber_gherkin_BINARY_DIR}/cpp)
