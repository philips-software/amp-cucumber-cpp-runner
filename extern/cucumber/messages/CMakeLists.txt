FetchContent_Declare(cucumber_messages
    GIT_REPOSITORY https://github.com/cucumber/messages.git
    GIT_TAG "main"

    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(cucumber_messages)

add_subdirectory(${cucumber_messages_SOURCE_DIR}/cpp ${cucumber_messages_BINARY_DIR}/cpp)
