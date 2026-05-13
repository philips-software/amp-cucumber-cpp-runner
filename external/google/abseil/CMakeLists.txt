FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20240722.0
    OVERRIDE_FIND_PACKAGE
)

set(ABSL_PROPAGATE_CXX_STD On CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL On CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(absl)
