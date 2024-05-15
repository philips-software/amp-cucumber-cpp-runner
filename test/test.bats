#!/usr/bin/env bats

setup() {
    load '/usr/local/bats-support/load'
    load '/usr/local/bats-assert/load'
}

teardown() {
    rm -rf out.json out.xml
}

@test "Successful test" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag "@smoke and @result:OK" --feature test/features/test_scenarios.feature --report console --com COMx
    assert_success
}

@test "Parse tag expression" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag @smoke @result:OK --feature test/features/test_scenarios.feature --report console --com COMx
    assert_success
}

@test "Failed tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag "@smoke and @result:FAILED" --feature test/features/test_scenarios.feature --report console --com COMx
    assert_failure
}

@test "Undefined tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag "@smoke and @result:UNDEFINED" --feature test/features/test_scenarios.feature --report console --com COMx
    assert_failure
}

@test "No tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag "@invalidtag" --feature test/features/test_scenarios.feature --report console --com COMx
    assert_success
}

@test "All features in a folder" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features/subfolder --report console --com COMx
    assert_success
    assert_output --partial "Given there are 10 cucumbers"
    assert_output --partial "Given there are 20 cucumbers"
}

@test "Missing mandatory feature argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --report console
    assert_failure
    assert_output --partial "--feature is required"
}

@test "Missing mandatory report argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features
    assert_failure
    assert_output --partial "--report is required"
}

@test "Missing mandatory custom argument" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features --report console
    assert_failure
    assert_output --partial "--com is required"
}

@test "Second feature file does not overwrite success with an undefined status" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --tag "@undefinedsuccess and @result:success" --feature test/features/test_undefined_success_1.feature test/features/test_undefined_success_2.feature --report console --com COMx
    assert_success
}

@test "Valid reporters only" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features --report doesnotexist
    assert_failure
    assert_output --partial "--report: 'doesnotexist' is not a reporter"
}

@test "Run Scenario hooks" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features --tag @bats and @scenariohook and not @stephook --report console --com COMx
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    refute_output --partial "HOOK_BEFORE_STEP"
    refute_output --partial "HOOK_AFTER_STEP"
}

@test "Run Step hooks" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features --tag @bats and @stephook and not @scenariohook --report console --com COMx
    assert_success

    refute_output --partial "HOOK_BEFORE_SCENARIO"
    refute_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}

@test "Run Scenario and Step hooks" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example run --feature test/features --tag "@bats and (@scenariohook or @stephook)" --report console --com COMx
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}
