# Cucumber runtime performance analysis

**Run date:** 2026-11-14

**Environment:** Ubuntu 24.04 host, Linux 7.0.0-1017-azure, GCC 13.3, 4 CPUs

**Build:** RelWithDebInfo (`-O2 -g`), fetched CPM dependencies, examples enabled, tests and sanitizers disabled.

## Scope and method

This report measures the runner and fetched Cucumber libraries, not the test suites, acceptance harness, or benchmark fixtures. The optimized runner was built with:

```sh
cmake --preset host-fetch-deps \
  -D CMAKE_BUILD_TYPE=RelWithDebInfo \
  -D CCR_BUILD_EXAMPLES=On \
  -D CCR_BUILD_TESTS=Off \
  -D CCR_ENABLE_SANITIZERS=Off
cmake --build .build/host-fetch-deps --target cucumber_cpp.cli example.plugin -j4
```

The workload was a generated scenario outline with 100, 500, or 1,000 rows. Each row becomes a scenario with three steps; the example plugin registers three matching definitions. The 1,000-row file therefore exercised feature parsing, pickle compilation, test-case assembly, 3,000 step matches, and execution of 1,000 scenarios. The step bodies perform only small context reads/writes; this is a scaling workload, not a representative application benchmark.

The 1,000-row feature was run both with `--list-scenarios` (parse/pickle/name generation, no test-case assembly or execution) and as a full run. Each measurement was a fresh process. Peak RSS and wall time are from uninstrumented runs; Heaptrack and Massif numbers are instrumented. Callgrind's instruction totals include process startup and instrumentation. `perf stat` hardware counters were denied by `kernel.perf_event_paranoid=4`. gprofng's clock profile warned that the sampling timer period changed and gathered too few samples for reliable hotspot ranking.

Tools used: Valgrind Callgrind and Massif, Heaptrack, gprofng, and strace. The fetched Cucumber library revisions were Gherkin `b3c5a5c3`, cucumber-expressions `9e85b41b`, messages `aee2666e`, query `bb06ca0`, tag-expressions `3183167`, and pretty-formatter `72a1933`.

## End-to-end results

| Generated scenarios | Mode | Wall time | Peak RSS |
|---:|---|---:|---:|
| 100 | Parse/pickle only | <0.01 s | 9.1 MiB |
| 100 | Full run, progress formatter | 0.01 s | 15.4 MiB |
| 500 | Parse/pickle only | <0.01 s | 13.4 MiB |
| 500 | Full run, progress formatter | 0.04 s | 44.9 MiB |
| 1,000 | Parse/pickle only | 0.01 s | 19.0 MiB |
| 1,000 | Full run, progress formatter | 0.08 s | 81.6 MiB |
| 1,000 | Full run, summary formatter | 0.10 s | 83.3 MiB |

The parse-only runs loaded the same example plugin as full runs. Three repeated 1,000-scenario runs varied by less than 2% in peak RSS. The full-run peak grows much faster than parse-only RSS because test cases are assembled and runtime messages are emitted and retained; both modes scale roughly linearly with generated scenario count in this workload.

Callgrind recorded 120.9 million instructions for parse-only and 417.6 million for the full run. Its annotated output attributed nearly all counts to “below main,” so these totals distinguish workload phases but do not support a trustworthy per-function CPU ranking. Heaptrack measured 111,998 allocation calls and 11.46 MB peak heap for parse-only; the full run made 334,282 calls and peaked at 76.21 MB. Massif independently measured a 78,064,160-byte (74.45 MiB) peak for the full run.

## `cucumber_cpp` runtime

| Path | Evidence | Finding |
|---|---|---|
| Event broadcast and query archive | Full-run RSS was 81.6–83.3 MiB versus 19.0 MiB parse-only. At the Massif peak, 67.23 MB (86.1% of total heap) was allocated through `EnvelopeArchive::Store`; about 24 MB each came from archiving `TestStepStarted` and `TestStepFinished` events. `Query::Update` keeps pointers into archived envelopes, explaining why events remain live through the run. | **Highest-confidence memory scaling concern.** A full run retains runtime events proportional to scenarios and steps, in addition to the assembled test cases. The archive lifetime may be required by current pointer ownership, so reducing it safely would require reviewing query ownership/lifetimes rather than simply clearing the deque. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/util/Broadcaster.hpp:63-75`, `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/util/Broadcaster.cpp:37-42`, and the fetched Query implementation at `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/query-src/cpp/src/query/Query.cpp:65-110`. |
| Test-case assembly | `AssembleTestSuites` assembles every pickle before the serial runtime starts running suites, emits a `TestCase` envelope, and also stores the test case in its assembled suite. Massif/Heaptrack observed per-case message copies, but they are smaller than the full event archive in this workload. | **Secondary memory cost.** Peak memory includes both the ready-to-run test-case collection and archived message envelopes. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/assemble/AssembleTestSuites.cpp:72-110` and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/runtime/SerialRuntimeAdapter.cpp:52-63`. |
| Parameter registration and parsing | The runner loads custom parameter definitions before step definitions; each step expression is compiled when its definition is registered. This workload uses only the built-in `{string}` parameter and has no custom parameter parsers. | Registration is once per run and did not stand out as a hotspot. Per-step capture conversion is part of step matching; custom parameter-transform cost was not measured here. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/api/RunCucumber.cpp:144-154`, `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/plugin/ParameterLoader.cpp:8-25`, and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/support/StepRegistry.cpp:82-110`. |
| Step-definition matching | Each assembled pickle step scans the registered definition list and applies its matcher. Heaptrack showed repeated temporary-allocation paths through `TreeRegexp::MatchToGroup` and `Expression::MatchToArguments`. | Cost is proportional to steps × registered definitions. With only three definitions, matching was not a dominant measured cost; a large user registry could change that. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/assemble/AssembleTestSuites.cpp:29-42` and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/support/MatchStepDefinitions.cpp:34-50`. |
| Per-step pickle lookup during execution | `TestCaseRunner::RunAttempt` finds each pickle step by scanning `pickle.steps` for its ID. | A long scenario can incur quadratic lookup work in its step count. The measured scenarios have only three steps, so this is a scaling candidate, not an observed bottleneck in these runs. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/runtime/TestCaseRunner.cpp:130-154`. |

## Fetched Cucumber libraries and output

| Path | Evidence | Finding |
|---|---|---|
| Gherkin parsing and outline compilation | The parse-only profile covers `Parser::Parse` and `PickleCompiler::Compile`; 1,000 example rows produce 1,000 pickles. `CollectPickles` retains an AST shared pointer for each pickle source, emits a copied Gherkin document, copies each pickle into a shared pointer, and emits each pickle as an envelope. | Parsing and pickle creation scale with feature size; large outlines also multiply pickle and archived-envelope storage. The full-run peak is dominated by runtime event envelopes, not the Gherkin AST copy. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/api/Gherkin.cpp:21-59` and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/gherkin-src/cpp/src/lib/gherkin/cucumber/gherkin/PickleCompiler.cpp:134-315`. |
| cucumber-expressions matching | The example plugin's three expressions are registered once; matching runs once per pickle step and invokes `TreeRegexp::MatchToGroup`. | No high-cost regex behavior was isolated with this small definition set. The repeated scan and match work, rather than expression compilation, is the main concern when the number of registered definitions grows. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/cucumber_cpp/library/support/StepRegistry.cpp:35-41,82-110` and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/cucumber-expressions-src/cpp/src/cucumber/cucumber-expressions/TreeRegexp.cpp:243`. |
| Query envelope archive | The fetched query library stores every envelope in a `std::deque`; its query maps retain pointers to messages from those envelopes. | This lifetime requirement is the main reason the runner's event history scales with emitted messages. Any archive reduction needs an ownership redesign across both the runner and query library. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/query-src/cpp/include/cucumber/query/EnvelopeArchive.hpp:9-15` and `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/query-src/cpp/src/query/EnvelopeArchive.cpp:7-10`. |
| Progress formatter output | On the 1,000-scenario progress run, strace counted 5,001 `write` calls and 24.3 ms in the traced `write` syscalls. The fetched `ProgressPrinter::PrintStatus` flushes its stream after each status update. | A syscall-heavy output path, but not a proven wall-time bottleneck here: native progress runs took 0.08 s versus 0.10 s for summary, and tracing overhead affects the syscall timing. Buffering or choosing a less chatty formatter may help when output is directed to a slow terminal or pipe. See `/home/runner/work/amp-cucumber-cpp-runner/amp-cucumber-cpp-runner/.build/host-fetch-deps/_deps/pretty-formatter-src/cpp/src/cucumber/pretty-formatter/ProgressPrinter.cpp:21-51`. |

## Conclusions

1. The clearest measured issue is **memory growth during full runs**, primarily the retained Cucumber message archive. At 1,000 short scenarios the runner peaked at about 82–83 MiB RSS, compared with about 19 MiB for parse-only. This is a stress-scale result, not a claim that typical small suites use that much memory.
2. Feature parsing and scenario-outline pickle creation are measurable startup work, but the tested parsing-only run was fast. For larger outlines, avoidable copies and the event archive add memory on top of the source AST and generated pickles.
3. Step matching is structurally O(step count × definition count); this workload does not show it dominating because only three definitions are registered. Increase the definition registry in a representative profile before prioritizing indexing or dispatch changes.
4. Progress output flushes after every status and creates many writes. This is most relevant for slow output destinations; observed native timings do not establish it as the primary runtime bottleneck.
5. A long individual scenario may pay repeated linear pickle-step searches. The three-step test scenarios are too short to quantify that candidate.
