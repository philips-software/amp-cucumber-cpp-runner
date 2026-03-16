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

```cmake
FetchContent_Declare(cucumber_cpp
    GIT_REPOSITORY https://github.com/philips-software/amp-cucumber-cpp-runner.git
    GIT_TAG main
)

FetchContent_MakeAvailable(cucumber_cpp)

add_executable(my_test_runner)
target_sources(my_test_runner PRIVATE
    steps/mysteps.cpp
    hooks/myhooks.cpp
)

target_link_libraries(my_test_runner PRIVATE
    cucumber_cpp.runner
)
```

## Writing step definitions

Steps are written using the `GIVEN`, `WHEN`, `THEN` or `STEP` macro's. The syntax for step definitions is:

```cpp
// using cucumber expressions

STEP("cucumber expression without captures"){
    // body
}

STEP("cucumber expression with an {int}", (std::int32_t arg)){
    // body
}

// or as a regular expression:

STEP(R"(^a regular\?? expression without captures$)"){

}

STEP(R"(^a (regular|unregular) expression with ([1-2]) capture groups$)", (const std::string& type, const std::string& nrOfGroups)){

}
```

### Builtin Parameter Types

The argument list has to match with the cucumber expression type:

| Cucumber Expression | Parsed as      | Notes                            |
|---------------------|----------------|----------------------------------|
| `{int}`             | `std::int32_t` |                                  |
| `{float}`           | `float`        |                                  |
| `{word}`            | `std::string`  | words without whitespace         |
| `{string}`          | `std::string`  | "banana split" or 'banana split' |
| `{}`                | `std::string`  |                                  |
| `{bigdecimal}`      | `double`       | non-standard                     |
| `{biginteger}`      | `std::int64_t` | non-standard                     |
| `{byte}`            | `std::int8_t`  |                                  |
| `{short}`           | `std::int16_t` |                                  |
| `{long}`            | `std::int64_t` |                                  |
| `{double}`          | `double`       |                                  |


See the [Cucumber Expressions documentation](https://github.com/cucumber/cucumber-expressions#readme) for more details on supported types. amp-cucumber-cpp-runner has full support for cucumber expressions.

> ℹ️ except for biginteger, but it's optional

### Custom Parameter Types

There is also support for custom parameter types. They are written and used like:

```cpp
struct Flight
{
    std::string from;
    std::string to;
};

PARAMETER(Flight, ("flight", "([A-Z]{3})-([A-Z]{3})", true), (const std::string& from, const std::string& to))
{
    return Flight{ .from = from, .to = to };
}

STEP(R"({flight} has been delayed)", (const Flight& flight))
{
    EXPECT_THAT(flight.from, testing::StrEq("LHR"));
    EXPECT_THAT(flight.to, testing::StrEq("CDG"));
}
```

The `PARAMETER` macro API is as follows:
```
PARAMETER(<return type>, (<name>, <regular expression>, <use for snippets>), (<capture1, capture2, ...>))
```
> ℹ️ \<use for snippets\> is not supported yet

The `PARAMETER` arguments' arity has to match the number of top-level capture groups. Capture groups can be optional. Then a `std::optional<std::string>` has to be used instead.

```cpp
PARAMETER(int, ("optint", "([0-9]+)?", false), (const std::optional<std::string>& opt))
{
    if (!opt)
        return 0;
    // ...
}
```

### Custom step fixtures

amp-cucumber-cpp-runner has support for custom base classes for step definitions using the `GIVEN_F`, `WHEN_F`, `THEN_F` or `STEP_F` macros. These give additional flexibility for re-using common patterns for steps. For example:

```cpp
struct LoadMeOnConstruction
{
    void Tadaa() const
    {}
};

struct CustomFixture : cucumber_cpp::library::engine::Step
{
    using Step::Step;

    const LoadMeOnConstruction& alwaysAvailable{ context.Get<LoadMeOnConstruction>() };
};

GIVEN_F(CustomFixture, R"(a custom fixture background step)")
{
    alwaysAvailable.Tadaa();
}
```

### Assertion Framework

amp-cucumber-cpp-runner uses GoogleTest and GoogleMock as the basis for the assertion framework. For example:
```cpp
STEP(R"(the value should be {string})", (const std::string& expected_value))
{
    const auto& actual = context.Get<std::string>("cell");

    ASSERT_THAT(actual, testing::StrEq(expected_value));
}
```

> ℹ️ amp-cucumber-cpp-runner has been configured to throw on a failed `ASSERT_`'s. However, failed `EXPECT_`'s continue execution.

If asserting (or expecting) is not an option then it is also safe to throw exceptions.

## Writing hook definitions

Hooks are executed in order of appearance (reverse for AFTER hooks), sorted by filename and line number. If any before hook is executed then all after hooks are also executed. Even if a before hook has a failure.

API:
```cpp
// HOOK_BEFORE_ALL and HOOK_AFTER_ALL have optional name and order arguments
// BEFORE_ALL hooks are executed before any test is executed
HOOK_BEFORE_ALL(){}
HOOK_BEFORE_ALL(.name = "Custom Name", .order = -1){}

// AFTER_ALL hooks are executed after all tests have executed
HOOK_AFTER_ALL(.name = "Custom After All hook"){}
HOOK_AFTER_ALL(.order = 10){}

// All other hooks have an additional optional (tag) expression argument
// HOOK_BEFORE_FEATURE (non-standard) executes before the first scenario per feature file
HOOK_BEFORE_FEATURE(){}
HOOK_BEFORE_FEATURE("@smoke", "custom name", 10){}

// HOOK_AFTER_FEATURE (non-standard) executes after the last scenario per feature file
HOOK_AFTER_FEATURE(.expression = "@dummy"){}
HOOK_AFTER_FEATURE(.name = "after feature name"){}

// HOOK_BEFORE_SCENARIO and HOOK_AFTER_SCENARIO are executed before and after every scenario.
HOOK_BEFORE_SCENARIO(){}
HOOK_AFTER_SCENARIO(){}

// HOOK_BEFORE_SCENARIO and HOOK_AFTER_SCENARIO are executed before and after every step.
HOOK_BEFORE_STEP(){}
HOOK_AFTER_STEP(){}
```

## Executing tests

```sh
# executes all tests in the ./feature folder using the default summary formatter
$ example

# executes all tests in the ./feature folder using the prety formatter
$ example --format pretty

# executes all tests in the ./feature folder redirecting the pretty formatter to output.txt
$ example --format pretty:output.txt

# executes all tests in the ./feature folder redirecting the pretty formatter to output.txt. Also prints the pretty formatter to the console
$ example --format pretty:output.txt pretty

# executes all tests in the ./feature folder redirecting the pretty formatter to output.txt. Also prints the pretty formatter to the console. Sets the theme for all pretty formatters to plain
$ example --format pretty:output.txt pretty --format-options "{ \"pretty\": {\"theme\" : \"plain\"} }"

# executes all tests in the ./feature/acceptance and ./feature/integration folders using the prety formatter
$ example --format pretty -- ./feature/acceptance ./feature/integration

# executes all tests matching the exact tag expression in the ./feature/acceptance and ./feature/integration folders
$ example --format pretty --tags @smoke and not @ignore -- ./feature/acceptance ./feature/integration

# On windows you might need to enclose the tags in quotes:
$ example --format pretty --tags "@smoke and not @ignore" -- ./feature/acceptance ./feature/integration
```

> ℹ️ use --help to see all available arguments and options

## Contributing

[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-%23FE5196?logo=conventionalcommits&logoColor=white)](https://conventionalcommits.org)

amp-cucumber-cpp-runner uses semantic versioning and conventional commits.

Please refer to our [Contributing](CONTRIBUTING.md) guide when you want to contribute to this project.

## How to test the software

- `cmake --preset Host`
- `cmake --build --preset Host-Debug`
- `ctest --preset Host-Debug`

## License

amp-cucumber-cpp-runner is licensed under the [MIT](https://choosealicense.com/licenses/mit/) license. See [LICENSE file](LICENSE).
