# Plugin Architecture

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
        +Instance()$ DefinitionRegistration&
        +Register~Body~() size_t
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
    Application ..> DefinitionRegistration : clears on destroy
    Application ..> ConverterTypeMapClearer : clears on destroy
    ParameterLoader ..> DefinitionRegistration : reads parameters
    StepLoader ..> DefinitionRegistration : reads steps
    HookLoader ..> DefinitionRegistration : reads hooks
```

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
        call["Invoke ccr_register"]
        static["Plugin static constructors\nalready ran on dlopen"]
        singleton["DefinitionRegistration\nsingleton populated"]

        cli --> mgr --> dir
        dir -->|directory| scan --> open
        dir -->|file| open
        open --> sym --> call
        open -.->|side effect| static --> singleton
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
        clear["DefinitionRegistration::Clear\nregistry + customParameters"]
        clearTypes["ConverterTypeMapClearer::ClearAll"]

        unload --> clear --> clearTypes
    end

    build --> load --> run --> cleanup
```

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
        Note over App, DR: Plugin Loading
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
        App ->> App: ConverterTypeMapClearer::ClearAll()
    end
```

## Cleanup Guard

The `Application` destructor only clears registrations when plugins were loaded,
preserving statically-linked step definitions between runs:

```mermaid
flowchart TD
    dest["~Application()"]
    check{"dynamicLibraryManager\n.GetLoadedLibraries()\nempty?"}
    skip["No cleanup needed\nStatic registrations preserved"]
    unload["UnloadAll()\nlibraries.clear()"]
    clearDef["DefinitionRegistration\n::Instance().Clear()"]
    clearConv["ConverterTypeMapClearer\n::ClearAll()"]

    dest --> check
    check -->|yes| skip
    check -->|no| unload --> clearDef --> clearConv
```
