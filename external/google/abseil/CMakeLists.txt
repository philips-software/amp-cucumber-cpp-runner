FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 0cf0a5c9d12cc3783363ab20f11613e69fd04c9a # 20250814.2
    OVERRIDE_FIND_PACKAGE
)

set(ABSL_PROPAGATE_CXX_STD On CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL On CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(absl)
