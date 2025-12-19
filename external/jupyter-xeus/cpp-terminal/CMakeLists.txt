FetchContent_Declare(
    cpp-terminal
    GIT_REPOSITORY https://github.com/jupyter-xeus/cpp-terminal
    GIT_TAG 48ae2f284084850901c45b6c10a9d68949c1b272 # unreleased main
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

if (NOT CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
    add_compile_options(
        -Wno-unused-variable
        -Wno-unused-but-set-variable
    )
endif()

set(CPPTERMINAL_BUILD_EXAMPLES Off CACHE STRING "")
set(CPPTERMINAL_ENABLE_INSTALL Off CACHE STRING "")
set(CPPTERMINAL_ENABLE_TESTING Off CACHE STRING "")
set(CPPTERMINAL_ENABLE_DOCS Off CACHE STRING "")

FetchContent_MakeAvailable(cpp-terminal)
