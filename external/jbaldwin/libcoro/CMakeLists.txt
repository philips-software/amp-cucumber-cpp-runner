
FetchContent_Declare(
    libcoro
    GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
    GIT_TAG 0161911f260a7507ac1dd4aa26da2e2c092fb550 # v0.16.0
    SYSTEM
)

set(LIBCORO_FEATURE_TLS OFF CACHE STRING "")
set(LIBCORO_BUILD_TESTS OFF CACHE STRING "")
set(LIBCORO_BUILD_EXAMPLES OFF CACHE STRING "")

FetchContent_MakeAvailable(libcoro)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    # libcoro sets public Wall and Wextra warnings, but we want to disable them for our project
    target_compile_options(libcoro INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-extra>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-field-initializers>
    )
endif()
