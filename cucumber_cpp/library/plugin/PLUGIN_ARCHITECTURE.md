# Plugin Architecture

This document describes the dynamic plugin loading system for cucumber-cpp-runner.
Plugins are shared libraries (`.so` / `.dylib` / `.dll`) that register steps, hooks,
and parameter types at runtime via the same macros used in statically-linked code.

---

## Overview

The plugin system enables loading cucumber step definitions, hooks, and custom
parameter types from shared libraries at runtime via the `--load` CLI option.
This allows test code to be compiled independently of the main runner executable
and loaded on demand.

Key design principles:

- **Transparent authoring** — plugin source files use the same `STEP`, `HOOK_*`,
  and `PARAMETER` macros as statically-linked code.
- **Static registration preserved** — destroying an `Application` only removes
  dynamically-loaded definitions; statically-linked registrations survive.
- **Cross-platform** — works on Linux (`.so`), macOS (`.dylib`), and Windows (`.dll`).
- **Validation** — each plugin must export a `ccr_register` symbol (provided
  automatically by `ccr_add_plugin`) as an entry-point validation gate.

---

## Object Diagram

```mermaid
classDiagram
    class Application {
        -Options options
        -CLI::App cli
        -Broadcaster broadcaster
        -DynamicLibraryManager dynamicLibraryManager
        -ParameterRegistry parameterRegistry
        -Formatters formatters
        +Run(argc, argv) int
        +~Application()
    }

    class DynamicLibraryManager {
        -vector~DynamicLibrary~ libraries
        +Load(paths)
        +UnloadAll()
        +GetLoadedLibraries() vector~path~
        -LoadFile(path)
        -LoadDirectory(path)
    }

    class DynamicLibrary {
        -void* handle
        -path libraryPath
        +DynamicLibrary(path)
        +~DynamicLibrary()
        +GetSymbol~FnPtr~(name) FnPtr
        +PlatformExtension()$ string_view
    }

    class DefinitionRegistration {
        -map staticRegistry
        -set staticParameters
        -map registry
        -set customParameters
        +Instance()$ DefinitionRegistration&
        +Register~Body~() size_t
        +TakeSnapshot()
        +Clear()
        +ForEachRegisteredStep()
        +GetHooks()
        +GetRegisteredParameters()
    }

    class ParameterLoader {
        +Load(defReg, paramReg)$
    }

    class StepLoader {
        +Load(stepRegistry)$
    }

    class HookLoader {
        +Load(hookRegistry)$
    }

    class PluginRegister {
        +ccr_register()
    }

    class ConverterTypeMapClearer {
        +ClearAll()$
    }

    Application *-- DynamicLibraryManager
    DynamicLibraryManager *-- "0..*" DynamicLibrary
    DynamicLibrary ..> PluginRegister : resolves ccr_register
    Application ..> DefinitionRegistration : snapshot + restore
    Application ..> ConverterTypeMapClearer : clears on destroy
    ParameterLoader ..> DefinitionRegistration : reads parameters
    StepLoader ..> DefinitionRegistration : reads steps
    HookLoader ..> DefinitionRegistration : reads hooks
```

**`Application`** owns the `DynamicLibraryManager` and orchestrates the full
lifecycle: CLI parsing → snapshot → plugin loading → execution → cleanup.

**`DynamicLibraryManager`** manages plugin lifetime. It holds a vector of
`DynamicLibrary` RAII wrappers. `Load()` accepts file paths or directories;
directories are scanned for platform-matching shared libraries and loaded in
sorted order.

**`DynamicLibrary`** is a move-only RAII wrapper around `dlopen`/`LoadLibrary`.
Construction opens the library; destruction closes it. `GetSymbol<FnPtr>(name)`
resolves a typed function pointer.

**`DefinitionRegistration`** is the global singleton holding all step, hook, and
parameter registrations. Entries are keyed by `std::source_location`. The
dual-container design (`TakeSnapshot` / `Clear`) separates static and
dynamic entries: `TakeSnapshot` moves current entries to dedicated static
containers, and `Clear` only removes the dynamic ones.

---

## Data Flow

```mermaid
flowchart TB
    subgraph build["Build Phase"]
        src["Plugin Source\nSTEP / HOOK / PARAMETER macros"]
        reg["ccr_plugin_register\nObject Library"]
        mod["MODULE Library\n.so / .dylib / .dll"]
        src --> mod
        reg --> mod
    end

    subgraph load["Load Phase — Application::Run"]
        cli["Parse --load paths"]
        snap["TakeSnapshot()\nMove entries to static containers"]
        mgr["DynamicLibraryManager::Load"]
        dir{"Directory\nor File?"}
        scan["Scan & sort by\nplatform extension"]
        open["dlopen / LoadLibrary\nRTLD_NOW | RTLD_GLOBAL"]
        sym["dlsym ccr_register\nValidation gate"]
        invoke["Invoke ccr_register"]
        pluginInit["Plugin static constructors\nalready ran on dlopen"]
        singleton["DefinitionRegistration\nsingleton populated"]

        cli --> snap --> mgr --> dir
        dir -->|directory| scan --> open
        dir -->|file| open
        open --> sym --> invoke
        open -.-> pluginInit --> singleton
    end

    subgraph run["Execution Phase — RunCucumber"]
        p1["Phase 1: ParameterLoader\nCustom parameters → ParameterRegistry"]
        p2["Phase 2: StepLoader\nStep definitions → StepRegistry"]
        p3["Phase 3: HookLoader\nHooks → HookRegistry"]
        exec["Runtime executes\nscenarios"]

        p1 --> p2 --> p3 --> exec
    end

    subgraph cleanup["Cleanup Phase — ~Application"]
        unload["DynamicLibraryManager::UnloadAll\nlibraries.clear → dlclose"]
        restore["DefinitionRegistration::Clear()\nRemove only dynamic entries"]
        clearTypes["ConverterTypeMapClearer::ClearAll\nMaps repopulated next run"]

        unload --> restore --> clearTypes
    end

    build --> load --> run --> cleanup
```

The data flow has four phases:

1. **Build** — plugin sources are compiled into MODULE shared libraries. The
   `ccr_add_plugin` CMake function automatically links the `ccr_plugin_register`
   object library which provides the `ccr_register` entry point.

2. **Load** — `Application::Run` parses `--load` paths, takes a registry
   snapshot (capturing the static baseline), then delegates to
   `DynamicLibraryManager`. Each library is opened with `RTLD_NOW | RTLD_GLOBAL`,
   which triggers the plugin's static constructors — these register steps/hooks/
   parameters into the global `DefinitionRegistration` singleton.

3. **Execution** — `RunCucumber` reads from `DefinitionRegistration` in three
   ordered phases: parameters first (so step expressions can reference custom
   types), then steps, then hooks.

4. **Cleanup** — the `Application` destructor unloads plugins (`dlclose`), then
   calls `Clear()` which removes only the dynamic entries (those registered
   after `TakeSnapshot`), preserving statically-linked definitions.

---

## Sequence Diagram — Plugin Lifecycle

```mermaid
sequenceDiagram
    actor User
    participant App as Application
    participant CLI as CLI11 Parser
    participant DLM as DynamicLibraryManager
    participant DL as DynamicLibrary
    participant OS as OS (dlopen)
    participant Static as Plugin Static Constructors
    participant DR as DefinitionRegistration
    participant RC as RunCucumber
    participant PL as ParameterLoader
    participant SL as StepLoader
    participant HL as HookLoader

    User ->> App: Run(argc, argv)
    App ->> CLI: parse("--load plugin.so -- features/")
    CLI -->> App: options.loadPaths = ["plugin.so"]

    rect rgb(220, 245, 220)
        Note over App, DR: Snapshot & Plugin Loading
        App ->> DR: TakeSnapshot()
        Note right of DR: Move current entries<br/>to static containers
        App ->> DLM: Load(["plugin.so"])
        DLM ->> DL: DynamicLibrary("plugin.so")
        DL ->> OS: dlopen("plugin.so", RTLD_NOW | RTLD_GLOBAL)
        OS -->> Static: Static constructors execute
        Static ->> DR: Register<StepBody>("pattern", ...)
        Static ->> DR: Register<HookBody>(hookType, ...)
        OS -->> DL: handle
        DL ->> DL: GetSymbol("ccr_register")
        DL -->> DLM: ccr_register validated & called
    end

    rect rgb(220, 230, 250)
        Note over App, HL: Three-Phase Registration
        App ->> RC: RunCucumber(options, paramReg, ...)

        RC ->> PL: Load(defReg, paramReg)
        PL ->> DR: GetRegisteredParameters()
        DR -->> PL: custom parameter entries
        PL -->> RC: Parameters registered

        RC ->> SL: Load(stepRegistry)
        SL ->> DR: ForEachRegisteredStep()
        DR -->> SL: step entries with matchers
        SL -->> RC: Steps registered

        RC ->> HL: Load(hookRegistry)
        HL ->> DR: GetHooks()
        DR -->> HL: hook entries
        HL -->> RC: Hooks registered
    end

    rect rgb(250, 230, 230)
        Note over App, DR: Cleanup
        App ->> DLM: UnloadAll()
        DLM ->> DL: ~DynamicLibrary()
        DL ->> OS: dlclose(handle)
        App ->> DR: Clear()
        Note right of DR: Remove only dynamic entries<br/>Static containers preserved
        App ->> App: ConverterTypeMapClearer::ClearAll()
    end
```

---

## Snapshot & Cleanup

The `Application` destructor uses a snapshot-based approach so that statically-
linked step/hook/parameter definitions are never lost — only plugin-injected
entries are removed.

```mermaid
flowchart TD
    dest["~Application()"]
    check{"dynamicLibraryManager\n.GetLoadedLibraries()\nempty?"}
    skip["No cleanup needed"]
    unload["UnloadAll()\nlibraries.clear() → dlclose"]
    restore["Clear()\nRemove dynamic registry\nand customParameters"]
    clearConv["ConverterTypeMapClearer::ClearAll()\nType maps repopulated next run"]
    done["Static registrations intact\nReady for next Application"]

    dest --> check
    check -->|yes| skip
    check -->|no| unload --> restore --> clearConv --> done
```

**How it works:**

1. Before plugins are loaded, `TakeSnapshot()` moves the current `registry`
   and `customParameters` into separate `staticRegistry` and `staticParameters`
   containers. This represents the "static baseline" — everything registered
   by code compiled directly into the executable.

2. Plugins load and their static constructors add new entries to the (now
   empty) `registry` and `customParameters` containers.

3. On destruction, `Clear()` clears only the dynamic `registry` and
   `customParameters`. The static containers are untouched.

4. `ConverterTypeMapClearer::ClearAll()` clears all converter type maps. This
   is safe because these maps are repopulated from `DefinitionRegistration`
   contents at the start of each `RunCucumber` call (Phase 1: ParameterLoader).

**Consequence**: Multiple sequential `Application` instances in the same process
will correctly share statically-linked definitions while each getting a fresh
set of plugin-loaded definitions.
