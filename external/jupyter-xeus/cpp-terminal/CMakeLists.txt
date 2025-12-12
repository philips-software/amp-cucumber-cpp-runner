FetchContent_Declare(
    cpp-terminal
    GIT_REPOSITORY https://github.com/jupyter-xeus/cpp-terminal
    GIT_TAG 48ae2f284084850901c45b6c10a9d68949c1b272 # unreleased main
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

add_compile_options(
    -Wno-unused-variable
    -Wno-unused-but-set-variable
)

set(CPPTERMINAL_BUILD_EXAMPLES Off)
set(CPPTERMINAL_ENABLE_INSTALL Off)
set(CPPTERMINAL_ENABLE_TESTING Off)
set(CPPTERMINAL_ENABLE_DOCS Off)

FetchContent_MakeAvailable(cpp-terminal)
