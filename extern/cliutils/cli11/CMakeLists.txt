FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG f4d0731cebb123ff0ace712c099dffbcd2c58e5a # v2.4.1
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

# set(ARGS_BUILD_EXAMPLE Off CACHE INTERNAL "")
# set(ARGS_BUILD_UNITTESTS Off CACHE INTERNAL "")

FetchContent_MakeAvailable(cli11)

# set_target_properties(cli11 PROPERTIES FOLDER External/cli11)
