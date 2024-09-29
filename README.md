# amp-cucumber-cpp-runner

**Description**: amp-cucumber-cpp-runner is a cucumber runner that provides a C++ interface for cucumber tests. amp-cucumber-cpp-runner is to be consumed as a library project as it does not provide an executable of itself.

## Dependencies

amp-cucumber-cpp-runner requires:
- A recent C++ compiler that supports C++20. (Ranges library required)
- CMake 3.24 or higher

amp-cucumber-cpp-runner is known to build using:
- Linux gcc-12
- Linux clang-16
- Linux clang-cl-16 (targeting Windows with WinSDK 2022 downloaded through xwin)

## How to use the software

amp-cucumber-cpp-runner is not supposed to be used standalone. It is possible to add amp-cucumber-cpp-runner as a submodule to your own project or to use cmake's FetchContent to add amp-cucumber-cpp-runner as a cmake build dependency.

En example project is provided which shows most features of amp-cucumber-cpp-runner and how to configure a cmake project to use amp-cucumber-cpp-runner. The simplest solution is to simply add a dependency on `cucumber-cpp-runner` like so:
`cmake
target_link_libraries(cucumber-cpp-example PRIVATE
    cucumber-cpp-runner
)
`

## How to test the software

- `cmake --preset Host`
- `cmake --build --preset Host-Debug`
- `ctest --preset Host-Debug`

## Contributing

[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-%23FE5196?logo=conventionalcommits&logoColor=white)](https://conventionalcommits.org)

amp-cucumber-cpp-runner uses semantic versioning and conventional commits.

Please refer to our [Contributing](CONTRIBUTING.md) guide when you want to contribute to this project.

## License

amp-cucumber-cpp-runner is licensed under the [MIT](https://choosealicense.com/licenses/mit/) license. See [LICENSE file](LICENSE).
