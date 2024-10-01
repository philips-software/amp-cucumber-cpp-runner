#!/usr/bin/env bats

setup() {
    load '/usr/local/bats-support/load'
    load '/usr/local/bats-assert/load'
}

teardown() {
    rm -rf out.json out.xml
}

@test "Successful test" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag "@result:OK" --feature cucumber_cpp/acceptance_test/features --report console
    assert_success
}

@test "Parse tag expression" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag @smoke @result:OK --feature cucumber_cpp/acceptance_test/features --report console
    assert_success
}

@test "Failed tests" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag "@result:FAILED" --feature cucumber_cpp/acceptance_test/features --report console
    assert_failure
    assert_output --partial "failed \"cucumber_cpp/acceptance_test/features/test_scenarios.feature\""
    assert_output --partial "skipped Then a then step"
}

@test "Undefined tests" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag "@result:UNDEFINED" --feature cucumber_cpp/acceptance_test/features --report console
    assert_failure
    assert_output --partial "undefined \"cucumber_cpp/acceptance_test/features/test_scenarios.feature\""
    assert_output --partial "skipped Then a then step"
}

@test "No tests" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag "@invalidtag" --feature cucumber_cpp/acceptance_test/features --report console
    assert_success
}

@test "All features in a folder" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features/subfolder --report console
    assert_success
    assert_output --partial "test1 scenario"
    assert_output --partial "test2 scenario"
}

@test "Missing mandatory feature argument" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --report console
    assert_failure
    assert_output --partial "--feature is required"
}

@test "Missing mandatory report argument" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "--report is required"
}

@test "Missing mandatory custom argument" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test.custom run --feature cucumber_cpp/acceptance_test/features --report console
    assert_failure
    assert_output --partial "--required is required"
}

@test "Second feature file does not overwrite success with an undefined status" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --tag "@undefinedsuccess and @result:success" --feature cucumber_cpp/acceptance_test/features/test_undefined_success_1.feature cucumber_cpp/acceptance_test/features/test_undefined_success_2.feature --report console
    assert_success
}

@test "Valid reporters only" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --report doesnotexist
    assert_failure
    assert_output --partial "--report: 'doesnotexist' is not a reporter"
}

@test "Run Program hooks" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag @bats --report console
    assert_success

    assert_output --partial "HOOK_BEFORE_ALL"
    assert_output --partial "HOOK_AFTER_ALL"
}

@test "Run Scenario hooks" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag @bats and @scenariohook and not @stephook --report console
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    refute_output --partial "HOOK_BEFORE_STEP"
    refute_output --partial "HOOK_AFTER_STEP"
}

@test "Run Step hooks" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag @bats and @stephook and not @scenariohook --report console
    assert_success

    refute_output --partial "HOOK_BEFORE_SCENARIO"
    refute_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}

@test "Run Scenario and Step hooks" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@bats and (@scenariohook or @stephook)" --report console
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}

@test "Dry run with known failing steps" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@result:FAILED" --report console
    assert_failure

    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@result:FAILED" --report console --dry
    assert_success
}

@test "Dry run with known missing steps" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@result:UNDEFINED" --report console --dry
    assert_failure
    assert_output --partial "undefined \"cucumber_cpp/acceptance_test/features/test_scenarios.feature\""
    assert_output --partial "skipped Then a then step"
}

@test "Test the and keyword" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@keyword-and" --report console
    assert_success
    assert_output --partial "--when--"
    assert_output --partial "--and--"
}

@test "Test the but keyword" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@keyword-but" --report console
    assert_success
    assert_output --partial "--when--"
    assert_output --partial "--but--"
}

@test "Test the asterisk keyword - will fail" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/features --tag "@keyword-asterisk" --report console
    assert_failure
}
