FetchContent_Declare(cucumber-messages
    GIT_REPOSITORY https://github.com/daantimmer/messages.git
    GIT_TAG "fix-compilation-issues-with-clang"

    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(cucumber-messages)

add_subdirectory(${cucumber-messages_SOURCE_DIR}/cpp ${cucumber-messages_BINARY_DIR}/cpp)
