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
- **Registrations stay in-place** — each plugin keeps its own
  `DefinitionRegistration` instance; the host holds references and aggregates
  lookups across all plugins.
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
        -map registry
        -set customParameters
        -vector~DefinitionRegistration*~ plugins
        +Instance()$ DefinitionRegistration&
        +Register~Body~() size_t
        +RegisterPlugin(plugin)
        +UnregisterPlugins()
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

    Application *-- DynamicLibraryManager
    DynamicLibraryManager *-- "0..*" DynamicLibrary
    DynamicLibrary ..> PluginRegister : resolves ccr_register
    Application ..> DefinitionRegistration : unregister on destroy
    DefinitionRegistration o-- "0..*" DefinitionRegistration : plugins
    ParameterLoader ..> DefinitionRegistration : reads parameters
    StepLoader ..> DefinitionRegistration : reads steps
    HookLoader ..> DefinitionRegistration : reads hooks
```

**`Application`** owns the `DynamicLibraryManager` and orchestrates the full
lifecycle: CLI parsing → plugin loading → execution → cleanup.

**`DynamicLibraryManager`** manages plugin lifetime. It holds a vector of
`DynamicLibrary` RAII wrappers. `Load()` accepts file paths or directories;
directories are scanned for platform-matching shared libraries and loaded in
sorted order.

**`DynamicLibrary`** is a move-only RAII wrapper around `dlopen`/`LoadLibrary`.
Construction opens the library; destruction closes it. `GetSymbol<FnPtr>(name)`
resolves a typed function pointer.

**`DefinitionRegistration`** is a singleton holding step, hook, and parameter
registrations. Entries are keyed by `std::source_location`. Each plugin has its
own instance; the host's instance holds a `plugins` vector of pointers to
plugin instances. Query methods iterate the host's own entries first, then each
plugin's entries.

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
        mgr["DynamicLibraryManager::Load"]
        dir{"Directory\nor File?"}
        scan["Scan & sort by\nplatform extension"]
        open["dlopen / LoadLibrary\nRTLD_NOW | RTLD_GLOBAL"]
        sym["dlsym ccr_register\nValidation gate"]
        invoke["Invoke ccr_register\nRegisterPlugin(local)"]
        pluginInit["Plugin static constructors\nalready ran on dlopen"]
        singleton["Plugin DefinitionRegistration\ninstance populated"]

        cli --> mgr --> dir
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
        unreg["DefinitionRegistration::UnregisterPlugins\nClear plugin pointer list"]
        unload["DynamicLibraryManager::UnloadAll\nlibraries.clear → dlclose"]

        unreg --> unload
    end

    build --> load --> run --> cleanup
```

The data flow has four phases:

1. **Build** — plugin sources are compiled into MODULE shared libraries. The
   `ccr_add_plugin` CMake function automatically links the `ccr_plugin_register`
   object library which provides the `ccr_register` entry point.

2. **Load** — `Application::Run` parses `--load` paths and delegates to
   `DynamicLibraryManager`. Each library is opened (`dlopen` with
   `RTLD_NOW | RTLD_GLOBAL` on Linux/macOS, `LoadLibrary` on Windows),
   which triggers the plugin's static constructors — these register steps/hooks/
   parameters into the plugin's `DefinitionRegistration` instance. Then
   `ccr_register` is called: on Linux/macOS this is a no-op (shared singleton),
   on Windows it registers the plugin's separate instance with the host.

3. **Execution** — `RunCucumber` reads from `DefinitionRegistration` in three
   ordered phases: parameters first (so step expressions can reference custom
   types), then steps, then hooks. Each query iterates the host's entries plus
   all registered plugin entries.

4. **Cleanup** — the `Application` destructor unregisters all plugin pointers,
   then unloads the DLLs (`dlclose`). The host's own registrations remain intact.

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
    participant PDR as Plugin DefinitionRegistration
    participant HDR as Host DefinitionRegistration
    participant RC as RunCucumber
    participant PL as ParameterLoader
    participant SL as StepLoader
    participant HL as HookLoader

    User ->> App: Run(argc, argv)
    App ->> CLI: parse("--load plugin.so -- features/")
    CLI -->> App: options.loadPaths = ["plugin.so"]

    rect rgb(220, 245, 220)
        Note over App, HDR: Plugin Loading
        App ->> DLM: Load(["plugin.so"])
        DLM ->> DL: DynamicLibrary("plugin.so")
        DL ->> OS: dlopen / LoadLibrary
        OS -->> Static: Static constructors execute
        Static ->> PDR: Register<StepBody>("pattern", ...)
        Static ->> PDR: Register<HookBody>(hookType, ...)
        OS -->> DL: handle
        DL ->> DL: GetSymbol("ccr_register")
        DL ->> HDR: ccr_register → RegisterPlugin(PDR)
        Note right of HDR: Linux/macOS: &PDR == &HDR → no-op<br/>Windows: stores pointer to plugin instance
    end

    rect rgb(220, 230, 250)
        Note over App, HL: Three-Phase Registration
        App ->> RC: RunCucumber(options, paramReg, ...)

        RC ->> PL: Load(defReg, paramReg)
        PL ->> HDR: GetRegisteredParameters()
        Note right of HDR: Iterates host + plugins
        HDR -->> PL: custom parameter entries
        PL -->> RC: Parameters registered

        RC ->> SL: Load(stepRegistry)
        SL ->> HDR: ForEachRegisteredStep()
        Note right of HDR: Iterates host + plugins
        HDR -->> SL: step entries with matchers
        SL -->> RC: Steps registered

        RC ->> HL: Load(hookRegistry)
        HL ->> HDR: GetHooks()
        Note right of HDR: Iterates host + plugins
        HDR -->> HL: hook entries
        HL -->> RC: Hooks registered
    end

    rect rgb(250, 230, 230)
        Note over App, HDR: Cleanup
        App ->> HDR: UnregisterPlugins()
        Note right of HDR: Clear plugin pointer list
        App ->> DLM: UnloadAll()
        DLM ->> DL: ~DynamicLibrary()
        DL ->> OS: dlclose(handle)
    end
```

---

## Platform Differences

The plugin-list model adapts to each platform's shared library semantics:

```mermaid
flowchart LR
    subgraph linux["Linux / macOS"]
        direction TB
        lhost["Host Process"]
        ldr["DefinitionRegistration\n(single shared instance)"]
        lplugin["Plugin .so / .dylib"]
        lstatic["Static constructors\nregister directly into\nshared instance"]
        lccr["ccr_register:\nRegisterPlugin(self)\n→ self-guard skips"]

        lhost --> ldr
        lplugin -.->|"RTLD_GLOBAL\nsymbols from host"| ldr
        lplugin --> lstatic --> ldr
        lplugin --> lccr
    end

    subgraph windows["Windows"]
        direction TB
        whost["Host Process"]
        whdr["Host DefinitionRegistration\nplugins: [&A, &B]"]
        wpluginA["Plugin A .dll"]
        wpdrA["Plugin A\nDefinitionRegistration"]
        wpluginB["Plugin B .dll"]
        wpdrB["Plugin B\nDefinitionRegistration"]
        wccr["ccr_register:\nRegisterPlugin(local)\n→ host stores pointer"]

        whost --> whdr
        wpluginA --> wpdrA
        wpluginB --> wpdrB
        whdr -.->|"iterates"| wpdrA
        whdr -.->|"iterates"| wpdrB
        wpluginA --> wccr
        wpluginB --> wccr
    end
```

| Aspect | Linux / macOS | Windows |
|--------|---------------|---------|
| Symbol resolution | `RTLD_GLOBAL` — plugin uses host symbols | Static link — plugin has own copy of `cucumber_cpp` |
| `DefinitionRegistration::Instance()` | Same address in host and plugin | Different instance per DLL |
| Static constructors | Register into the shared (host) instance | Register into plugin-local instance |
| `ccr_register` effect | No-op (self-registration guard) | Stores plugin instance pointer in host |
| `ConverterTypeMap<T>` | Single shared instance | Separate per DLL (step executes in its own DLL context) |
| Plugin list (`plugins`) | Empty — all entries already in host | One entry per loaded plugin |
| CMake linking | No link (symbols from host via `-rdynamic`) | `target_link_libraries(plugin PRIVATE cucumber_cpp)` |
| macOS specifics | `-undefined dynamic_lookup` suppresses linker errors | — |

---

## Plugin Registration Model

Each plugin has its own `DefinitionRegistration` singleton. The host's instance
maintains a list of plugin instance pointers.

```mermaid
flowchart TD
    dest["~Application()"]
    check{"dynamicLibraryManager\n.GetLoadedLibraries()\nempty?"}
    skip["No cleanup needed"]
    unreg["UnregisterPlugins()\nClear plugin pointer list"]
    unload["UnloadAll()\nlibraries.clear() → dlclose"]
    done["Host registrations intact\nPlugin DLLs unloaded"]

    dest --> check
    check -->|yes| skip
    check -->|no| unreg --> unload --> done
```

**How it works:**

1. Plugins are loaded and their static constructors register steps/hooks/parameters
   into the plugin's own `DefinitionRegistration::Instance()`.

2. `ccr_register` is called with a pointer to the host's `DefinitionRegistration`.
   It calls `host.RegisterPlugin(local)`. A self-registration guard (`&plugin == this`)
   skips registration on Linux/macOS where RTLD_GLOBAL makes both point to the
   same singleton.

3. Query methods (`ForEachRegisteredStep`, `GetHooks`, `GetRegisteredParameters`,
   `LoadIds`) iterate the host's own entries, then each plugin's entries.

4. On destruction, `UnregisterPlugins()` clears the pointer list before `UnloadAll()`
   closes the shared libraries (preventing dangling pointers).

**Consequence**: Multiple sequential `Application` instances in the same process
will correctly share statically-linked definitions while each getting a fresh
set of plugin-loaded definitions.
