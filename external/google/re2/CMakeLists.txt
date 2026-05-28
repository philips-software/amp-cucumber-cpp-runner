FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG 0f6c07eae69151e606acb3d9232750c3442dff23 # 2025-08-12
    OVERRIDE_FIND_PACKAGE
)

set(RE2_BUILD_TESTING Off CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(re2)
