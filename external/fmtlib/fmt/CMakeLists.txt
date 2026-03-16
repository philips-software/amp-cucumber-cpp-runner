FetchContent_Declare(
    libfmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f # 12.1.0
)

FetchContent_MakeAvailable(libfmt)
