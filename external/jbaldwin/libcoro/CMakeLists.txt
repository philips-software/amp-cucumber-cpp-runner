
FetchContent_Declare(
    libcoro
    # GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
    GIT_REPOSITORY https://github.com/daantimmer/libcoro.git
    GIT_TAG 9b2182cedbb95a563025b1a06d38b3989097c5d6 # public contribution on forked repository fixing warnings
    SYSTEM
)

set(LIBCORO_FEATURE_TLS OFF CACHE STRING "")
set(LIBCORO_BUILD_TESTS OFF CACHE STRING "")
set(LIBCORO_BUILD_EXAMPLES OFF CACHE STRING "")

FetchContent_MakeAvailable(libcoro)
