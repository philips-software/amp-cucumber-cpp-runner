#!/usr/bin/env bats

setup_file() {
    # CTest provides the binaries; otherwise select the last built executables
    : "${custom_test:=$(find . -name "cucumber_cpp.acceptance_test.custom" -printf '%T@ %p\n' | sort -nr | head -n1 | cut -d' ' -f2-)}"
    : "${plugin_test:=$(find . -name "cucumber_cpp.acceptance_test.plugin" -not -name "*.plugin_*" -printf '%T@ %p\n' | sort -nr | head -n1 | cut -d' ' -f2-)}"
    export custom_test plugin_test
}

setup() {
    # amp-devcontainer installs to /usr/local, apt installs to /usr/lib/bats
    local bats_lib_dir
    for bats_lib_dir in /usr/local /usr/lib/bats; do
        if [[ -d "$bats_lib_dir/bats-support" && -d "$bats_lib_dir/bats-assert" ]]; then
            load "$bats_lib_dir/bats-support/load"
            load "$bats_lib_dir/bats-assert/load"
            return
        fi
    done

    echo "bats-support/bats-assert not found in /usr/local or /usr/lib/bats" >&2
    return 1
}

teardown() {
    rm -rf ./out/
}

@test "Missing mandatory custom argument" {
    run $custom_test --format summary pretty message junit -- cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "--required is required"
}

@test "Test error program hook results in error and skipped steps" {
    run $custom_test --format summary pretty message junit --tags "@smoke and @result:OK" --required --failprogramhook cucumber_cpp/acceptance_test/features
    assert_failure
    assert_output --partial "HOOK_BEFORE_ALL"
    assert_output --partial "HOOK_AFTER_ALL"
    assert_output --partial "0 scenarios"
    assert_output --partial "0 steps"
}

@test "Plugin test: load two plugins sequentially with static step" {
    run $plugin_test
    assert_success
}

@test "Plugin test: load all plugins from a directory" {
    run $plugin_test directory
    assert_success
}
