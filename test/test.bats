#!/usr/bin/env bats

setup() {
    load '/usr/local/bats-support/load'
    load '/usr/local/bats-assert/load'
}

teardown() {
    rm -rf out.json out.xml
}

@test "Successful test" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:OK" --feature cucumber-cpp-example/features/1very_long.feature cucumber-cpp-example/features/2simple.feature --report json console junit-xml --Xapp,--st
    assert_success
}

@test "Parse tag expression" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag @smoke @result:OK --feature cucumber-cpp-example/features/1very_long.feature cucumber-cpp-example/features/2simple.feature --report json console junit-xml --Xapp,--st
    assert_success
}

@test "Failed tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:FAILED" --feature cucumber-cpp-example/features/1very_long.feature cucumber-cpp-example/features/2simple.feature --report json console junit-xml --Xapp,--st
    assert_failure
}

@test "Undefined tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@smoke and @result:UNDEFINED" --feature cucumber-cpp-example/features/1very_long.feature cucumber-cpp-example/features/2simple.feature --report json console junit-xml --Xapp,--st
    assert_failure
}

@test "No tests" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --tag "@invalidtag" --feature cucumber-cpp-example/features/1very_long.feature cucumber-cpp-example/features/2simple.feature --report json console junit-xml --Xapp,--st
    assert_failure
    assert_output --partial "Error: no features have been executed"
}

@test "All features in a folder" {
    run .build/Host/cucumber-cpp-example/Debug/cucumber-cpp-example --feature cucumber-cpp-example/features/subfolder --report console --Xapp,--st
    assert_success
    assert_output --partial "Given there are 10 cucumbers"
    assert_output --partial "Given there are 20 cucumbers"
}
