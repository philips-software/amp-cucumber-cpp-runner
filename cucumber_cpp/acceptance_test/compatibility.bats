#!/usr/bin/env bats

@test "Minimal" {
    run .build/Host/cucumber_cpp/acceptance_test/Debug/cucumber_cpp.acceptance_test run --feature cucumber_cpp/acceptance_test/compatibility/minimal/ --report ndjson
    # TODO link run NdjsonComparer with paths to expected file in the repository and actual file from the result
}
