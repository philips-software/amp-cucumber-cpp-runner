FetchContent_Declare(
    cucumber_gherkin
    GIT_REPOSITORY https://github.com/cucumber/gherkin.git
    GIT_TAG 5e4222e3956d52c8758f41727b14a1ab2dce5060 # unrelease PR
)

FetchContent_MakeAvailable(cucumber_gherkin)

add_subdirectory(${cucumber_gherkin_SOURCE_DIR}/cpp ${cucumber_gherkin_BINARY_DIR}/cpp)
