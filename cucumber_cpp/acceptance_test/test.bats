#!/usr/bin/env bats

setup_file() {
    acceptance_test=$(find / -name "cucumber_cpp.acceptance_test" -print -quit)
    export acceptance_test
}

setup() {
    load '/usr/local/bats-support/load'
    load '/usr/local/bats-assert/load'
}

teardown() {
    rm -rf ./out/
}

@test "Successful test" {
    run $acceptance_test run --tags "@result:OK" -- cucumber_cpp/acceptance_test/features
    assert_success
}

@test "Parse tag expression" {
    run $acceptance_test run --tags "@smoke and @result:OK" -- cucumber_cpp/acceptance_test/features
    assert_success
}

@test "Failed tests" {
    run $acceptance_test run --tags "@result:FAILED" -- cucumber_cpp/acceptance_test/features
    assert_failure
}

@test "Undefined tests" {
    run $acceptance_test run --tags "@result:UNDEFINED" -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "UNDEFINED Given a missing step"
    assert_output --partial "SKIPPED Then this should be skipped"
}

@test "No tests" {
    run $acceptance_test run --tags "@invalidtag" -- cucumber_cpp/acceptance_test/features
    assert_success
}

@test "All features in a folder" {
    run $acceptance_test run cucumber_cpp/acceptance_test/features/subfolder
    assert_success
    assert_output --partial "test1 feature"
    assert_output --partial "test1 scenario"
    assert_output --partial "test2 feature"
    assert_output --partial "test2 scenario"
}

@test "Missing mandatory feature argument" {
    run $acceptance_test run
    assert_failure
    assert_output --partial "paths is required"
}

# @test "Missing mandatory report argument" {
#     run $acceptance_test run cucumber_cpp/acceptance_test/features
#     assert_failure
#     assert_output --partial "--report is required"
# }

@test "Missing mandatory custom argument" {
    run $acceptance_test.custom run cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "--required is required"
}

@test "Second feature file does not overwrite success with an undefined status" {
    run $acceptance_test run --tags "@undefinedsuccess and @result:success" -- cucumber_cpp/acceptance_test/features/test_undefined_success_1.feature cucumber_cpp/acceptance_test/features/test_undefined_success_2.feature
    assert_success
}

# @test "Valid reporters only" {
#     run $acceptance_test run cucumber_cpp/acceptance_test/features
#     assert_failure
#     assert_output --partial "--report: 'doesnotexist' is not a reporter"
# }

@test "Run Program hooks" {
    run $acceptance_test run --tags @bats and @program_hooks -- cucumber_cpp/acceptance_test/features
    assert_success

    assert_output --partial "HOOK_BEFORE_ALL"
    assert_output --partial "HOOK_AFTER_ALL"
}

@test "Run Scenario hooks" {
    run $acceptance_test run --tags @bats and @scenariohook and not @stephook -- cucumber_cpp/acceptance_test/features
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    refute_output --partial "HOOK_BEFORE_STEP"
    refute_output --partial "HOOK_AFTER_STEP"
}

@test "Run Step hooks" {
    run $acceptance_test run --tags @bats and @stephook and not @scenariohook -- cucumber_cpp/acceptance_test/features
    assert_success

    refute_output --partial "HOOK_BEFORE_SCENARIO"
    refute_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}

@test "Run Scenario and Step hooks" {
    run $acceptance_test run --tags "@bats and (@scenariohook or @stephook)" -- cucumber_cpp/acceptance_test/features
    assert_success

    assert_output --partial "HOOK_BEFORE_SCENARIO"
    assert_output --partial "HOOK_AFTER_SCENARIO"

    assert_output --partial "HOOK_BEFORE_STEP"
    assert_output --partial "HOOK_AFTER_STEP"
}

@test "Dry run with known failing steps" {
    run $acceptance_test run --tags "@result:FAILED" -- cucumber_cpp/acceptance_test/features
    assert_failure

    run $acceptance_test run --tags "@result:FAILED" --dry-run cucumber_cpp/acceptance_test/features
    assert_success
}

@test "Dry run with known missing steps" {
    run $acceptance_test run --tags "@result:UNDEFINED" -- cucumber_cpp/acceptance_test/features
    assert_failure

    run $acceptance_test run --tags "@result:UNDEFINED" --dry-run  cucumber_cpp/acceptance_test/features
    assert_success
    assert_output --partial "UNDEFINED Given a missing step"
}

@test "Test the and keyword" {
    run $acceptance_test run --tags "@keyword-and" -- cucumber_cpp/acceptance_test/features
    assert_success
    assert_output --partial "--when--"
    assert_output --partial "--and--"
}

@test "Test the but keyword" {
    run $acceptance_test run --tags "@keyword-but" -- cucumber_cpp/acceptance_test/features
    assert_success
    assert_output --partial "--when--"
    assert_output --partial "--but--"
}

@test "Test the asterisk keyword" {
    run $acceptance_test run --tags "@keyword-asterisk" -- cucumber_cpp/acceptance_test/features
    assert_output --partial "print: --when--"
    assert_output --partial "print: --asterisk--"
    assert_success
}

@test "Test passing scenario after failed scenario reports feature as failed" {
    run $acceptance_test run  --tags "@fail_feature" -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "2 scenarios"
    assert_output --partial "1 passed"
}

@test "Test failing hook before results in error" {
    run $acceptance_test run  --tags "@fail_scenariohook_before" -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "FAILED Before"
}

@test "Test failing hook after results in error" {
    run $acceptance_test run  --tags "@fail_scenariohook_after" -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "FAILED After"
}

@test "Test throwing hook results in error" {
    run $acceptance_test run  --tags "@throw_scenariohook" -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "FAILED Before"
}

@test "Test error program hook results in error and skipped steps" {
    run $acceptance_test.custom run  --tags "@smoke and @result:OK" --required --failprogramhook cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "HOOK_BEFORE_ALL"
    assert_output --partial "HOOK_AFTER_ALL"
    assert_output --partial "0 scenarios"
    assert_output --partial "0 steps"
}

@test "Test unicode" {
    run $acceptance_test run  --tags "@unicode" -- cucumber_cpp/acceptance_test/features
    assert_success
    assert_output --partial "1 scenario"
    assert_output --partial "1 passed"
}

# @test "Test unused step reporting" {
#     run $acceptance_test run --tags "@unused_steps" --report cucumber_cpp/acceptance_test/features
#     assert_success
#     assert_output --regexp ".*The following steps have not been used:.*this step is not being used.*"
#     refute_output --regexp ".*The following steps have not been used:.*this step is being used.*"
# }

@test "Test unused steps by default not reported" {
    run $acceptance_test run --tags "@unused_steps" -- cucumber_cpp/acceptance_test/features
    assert_success
    refute_output --partial "The following steps have not been used:"
}
