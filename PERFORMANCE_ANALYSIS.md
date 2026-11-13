# Runtime performance analysis

**Run date:** 2026-11-13  
**Environment:** Ubuntu 24.04 host, GCC 13.3, 4 vCPUs  
**Build:** `test-fetch-deps` preset, `RelWithDebInfo`, fetched dependencies, tests and examples enabled, sanitizers disabled; Google Benchmark enabled for profiling.

## Tools and coverage

Configured and built with:

```sh
cmake --preset test-fetch-deps \
  -D CMAKE_BUILD_TYPE=RelWithDebInfo \
  -D CCR_ENABLE_SANITIZERS=Off \
  -D CCR_BUILD_BENCHMARKS=On
cmake --build .build/test-fetch-deps -j4
ctest --preset test-fetch-deps -j4 --output-on-failure
```

CTest reported **0 failures out of 184 cases**, with three compatibility cases marked skipped; wall time was **295.63 s**. The Cucumber acceptance label accounted for **1,154.45 process-seconds** across 57 cases.

Used Valgrind 3.22 (Callgrind and Massif), Heaptrack 1.5, strace 6.8, and the repository's Google Benchmark target. `perf` 7.0.14 was present, but hardware-counter collection was blocked by `kernel.perf_event_paranoid=4` (`perf stat -e instructions true` was denied). Callgrind instruction counts and benchmark timings below are software-instrumented or synthetic measurements, not hardware-counter measurements.

## Cucumber library subproject

| Area | Measurement | Finding |
|---|---:|---|
| Context lookup, 10,000-entry scaling benchmark | First/last lookup: **0.797 µs / 41.725 µs** (52×). `Contains` first/last: **0.127 µs / 4.408 µs**. | Lookup cost grows with position in the `std::list`; the implementation's linear scan and string comparisons are visible at this intentionally oversized scale. |
| Scenario-sized context benchmark | Lifecycle at 64 entries: **36.1 µs**; newest-entry lookup: **261 ns**; oldest-entry lookup: **12.3 ns**; parent fall-through at 64 entries: **95.1 ns**. | The scan asymmetry is measurable but the tested scenario sizes are small. The 10,000-entry stress result should not be read as typical runner performance. |
| Context ordering stress test | Callgrind measured **67.29 million instructions** for the two 1,000-entry plain/thread-safe cases. `ContextStorageImpl::Find` accounted for **60.47 million (89.9%) inclusive**; string comparison (`memcmp`) accounted for **37.27 million (55.4%)**. | The test inserts and retrieves 1,000 keys, so repeated list scans dominate as expected for this stress case. It does not indicate that ordinary scenario contexts are this large. |
| Context test-suite memory | Heaptrack: **7,969 allocation calls**, **214 KiB** peak heap, 4.26 KiB still live at process exit. Massif: **230.8 KiB** peak. | No large retained-memory use was observed. The main context allocation site is list-node insertion in `ContextStorageImpl::Assign`. |
| Benchmark process setup | Heaptrack reported **30,123 allocations** and **1.27 MiB** peak heap even when filtering to one benchmark. Its 10,000 string, list-node, and shared-object allocations resolve to `prefilledContext`'s static initialization in `BmContext.cpp:45`. | The global 10,000-entry fixture is constructed even when the selected benchmark does not use it, adding avoidable startup allocation and memory overhead to filtered benchmark runs. This is benchmark-fixture overhead, not runner memory use. |

The measurements above come from `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/context/benchmark/BmContext.cpp`, `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/context/Context.cpp`, and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/context/test/TestContext.cpp`. The benchmark itself describes 10,000 entries as a scaling workload and models scenario sizes through 64 entries.

## Test harness and acceptance subproject

| Area | Measurement | Finding |
|---|---:|---|
| Parallel acceptance suite | Three plugin/configuration scenarios took **286.84–294.90 s each** in the full `ctest -j4` run. Running the exact configuration scenario in isolation took **9.60 s**; strace attributed **8.98 s** to compiling its sandbox plugin and about 0.2 s to linking. | Parallel acceptance cases rebuild the shared sandbox target. `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/acceptance/sandbox/BuildSandbox.cmake:4-7,31,47-49` explicitly serializes those builds with a cross-process lock. The roughly 30× isolated-vs-parallel duration strongly points to contention around this shared build, rather than a slow scenario execution path. |
| Subprocess large-output test | `acceptance.util.test` took **5.17 s**; its large-output case took **5.14 s**. Strace counted **131,023 reads** and **131,131 writes** across the process tree. | The stress child emits 131,072 bytes on each stream one character at a time (`SubprocessChild.cpp:75-81`), producing approximately one write syscall per byte and substantial test-only overhead. Chunked writes would preserve the output volume while reducing syscall count. |
| Runtime test suite | Callgrind: **4.84 million instructions** across 15 tests. `TestCaseRunner::Run` was **17.8% inclusive**; `RunAttempt` was **17.5% inclusive**. Heaptrack: **2,828 allocation calls**, **176.86 KiB** peak heap. | No separate high-cost runtime-library path or unusually large memory footprint stood out in this small unit-test workload; much of the instruction profile is test framework and dynamic-loader startup. |

The acceptance scenario comparison is based on the full CTest run and an isolated strace run of the same configuration scenario. It does not directly measure time spent waiting on the lock, so the precise split between lock wait and other parallel-build contention remains unmeasured.

## Suggested follow-up

1. Defer creation of the 10,000-entry benchmark fixture until a benchmark that uses it is selected.
2. Reduce repeated serialized sandbox rebuilds in acceptance tests, or isolate their build trees, then compare parallel CTest duration.
3. Emit the subprocess stress-test payload in chunks rather than one character at a time.
4. Keep the context scan finding scoped to larger contexts; revisit its cost only if representative scenario contexts grow substantially beyond the tested small sizes.
