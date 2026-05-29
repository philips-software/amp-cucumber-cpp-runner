FetchContent_Declare(
    cucumber_messages
    GIT_REPOSITORY https://github.com/cucumber/messages.git
    GIT_TAG 6582661d5b1c44a628f8eab65d3aca539f99c4ab # v32.2.0
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(cucumber_messages)

add_subdirectory(${cucumber_messages_SOURCE_DIR}/cpp ${cucumber_messages_BINARY_DIR}/cpp)
