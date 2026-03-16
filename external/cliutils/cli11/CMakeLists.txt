FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG 37bb6edc5317e99af72ef48405e65d9ca5218861 # v2.6.2
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

# set(ARGS_BUILD_EXAMPLE Off CACHE INTERNAL "")
# set(ARGS_BUILD_UNITTESTS Off CACHE INTERNAL "")

FetchContent_MakeAvailable(cli11)

# set_target_properties(cli11 PROPERTIES FOLDER External/cli11)
