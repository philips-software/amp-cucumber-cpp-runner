#!/usr/bin/env bats

setup() {
    load '/usr/local/bats-support/load'
    load '/usr/local/bats-assert/load'
}

teardown() {
    rm -rf out.json out.xml
}

@test "Successful test" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:OK" --feature test/features/test_scenarios.feature --report json console junit-xml --com COMx
    assert_success
}

@test "Parse tag expression" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag @smoke @result:OK --feature test/features/test_scenarios.feature --report json console junit-xml --com COMx
    assert_success
}

@test "Failed tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:FAILED" --feature test/features/test_scenarios.feature --report json console junit-xml --com COMx
    assert_failure
}

@test "Undefined tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:UNDEFINED" --feature test/features/test_scenarios.feature --report json console junit-xml --com COMx
    assert_failure
}

@test "No tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@invalidtag" --feature test/features/test_scenarios.feature --report json console junit-xml --com COMx
    assert_failure
    assert_output --partial "Error: no features have been executed"
}

@test "All features in a folder" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --feature test/features/subfolder --report console --com COMx
    assert_success
    assert_output --partial "Given there are 10 cucumbers"
    assert_output --partial "Given there are 20 cucumbers"
}


@test "Missing mandatory feature argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --report console
    assert_failure
    assert_output --partial "--feature is required"
}


@test "Missing mandatory report argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --feature test/features
    assert_failure
    assert_output --partial "--report is required"
}

@test "Missing mandatory custom argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --feature test/features --report console
    assert_failure
    assert_output --partial "--com is required"
}


@test "Second feature file does not overwrite success with an undefined status" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@undefinedsuccess and @result:success" --feature test/features/test_undefined_success_1.feature test/features/test_undefined_success_2.feature --report console --com COMx
    assert_success
}
