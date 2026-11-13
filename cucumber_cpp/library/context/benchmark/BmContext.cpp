// #include "benchmark/benchmark.h"
#include "cucumber_cpp/library/context/Context.hpp"
#include <array>
#include <benchmark/benchmark_api.h>
#include <benchmark/registration.h>
#include <benchmark/state.h>
#include <benchmark/utils.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>

namespace cucumber_cpp::library::context
{
    namespace
    {
        // Scaling tests: far more entries than a scenario's context normally holds.
        template<typename T>
        void BmContextInsertion(benchmark::State& state)
        {
            for (auto _ : state)
            {
                Context context{ std::make_shared<T>() };

                for (auto i = 0; i < 1000; ++i)
                    context.InsertAt(std::to_string(i), i);
            }
        }

        BENCHMARK(BmContextInsertion<ContextStorageFactoryImpl>);

        template<class T>
        auto CreateContext(int size)
        {
            Context context{ std::make_shared<T>() };

            for (int i = 0; i < size; ++i)
                context.InsertAt("long_key_goes_here_" + std::to_string(i), i);

            return context;
        };

        template<class T>
        const Context prefilledContext = CreateContext<T>(10000);

        template<typename T>
        void BmContextLookupLow(benchmark::State& state)
        {
            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext<T>.template GetAt<int>("long_key_goes_here_250"));
            }
        }

        BENCHMARK(BmContextLookupLow<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextLookupHigh(benchmark::State& state)
        {
            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext<T>.template GetAt<int>("long_key_goes_here_9750"));
            }
        }

        BENCHMARK(BmContextLookupHigh<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextOverwriteLow(benchmark::State& state)
        {
            auto prefilledContext = CreateContext<T>(1000);

            for (auto _ : state)
            {
                prefilledContext.InsertAt("long_key_goes_here_100", 100);
            }
        }

        BENCHMARK(BmContextOverwriteLow<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextOverwriteHigh(benchmark::State& state)
        {
            auto prefilledContext = CreateContext<T>(1000);

            for (auto _ : state)
            {
                prefilledContext.InsertAt("long_key_goes_here_900", 900);
            }
        }

        BENCHMARK(BmContextOverwriteHigh<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextContainsLow(benchmark::State& state)
        {
            auto prefilledContext = CreateContext<T>(1000);

            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext.Contains("long_key_goes_here_100"));
            }
        }

        BENCHMARK(BmContextContainsLow<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextContainsHigh(benchmark::State& state)
        {
            auto prefilledContext = CreateContext<T>(1000);

            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext.Contains("long_key_goes_here_900"));
            }
        }

        BENCHMARK(BmContextContainsHigh<ContextStorageFactoryImpl>);

        // Scenario-sized tests, shaped like the runner's use of contexts: a long-lived program
        // context, an empty per-feature context and a short-lived per-scenario context. Keys are
        // typeid(T).name(), as used by Emplace<T>(), Insert<T>() and Get<T>().
        template<std::size_t>
        struct Value
        {};

        template<std::size_t... I>
        std::array<const char*, sizeof...(I)> TypeKeys(std::index_sequence<I...> /*indices*/)
        {
            return { typeid(Value<I>).name()... };
        }

        const auto typeKeys = TypeKeys(std::make_index_sequence<64>{});

        struct Options
        {};

        struct Fixture
        {};

        struct Hierarchy
        {
            Hierarchy()
            {
                program.Emplace<Options>();
                program.InsertAt("--com", std::string{ "COM3" });
                program.InsertAt("--st", true);
                program.InsertAt("--nordic", false);
                program.Emplace<Fixture>();
            }

            Context program{ std::make_shared<ContextStorageFactoryImpl>() };
            Context suite{ &program };
        };

        void Fill(Context& context, std::size_t entries)
        {
            for (std::size_t i = 0; i != entries; ++i)
                context.InsertAt(typeKeys[i], static_cast<int>(i));
        }

        // Creates a scenario context, inserts N values, reads each value three times plus two
        // program-level values, then destroys the context.
        void BmScenarioLifecycle(benchmark::State& state)
        {
            const auto entries = static_cast<std::size_t>(state.range(0));
            Hierarchy hierarchy;

            for (auto _ : state)
            {
                Context scenario{ &hierarchy.suite };
                Fill(scenario, entries);

                for (int read = 0; read != 3; ++read)
                    for (std::size_t i = 0; i != entries; ++i)
                        benchmark::DoNotOptimize(scenario.GetAt<int>(typeKeys[i]));

                benchmark::DoNotOptimize(scenario.GetAt<bool>("--st"));
                benchmark::DoNotOptimize(scenario.Get<Fixture>());
            }
        }

        BENCHMARK(BmScenarioLifecycle)->Arg(0)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

        void BmScenarioLookupOldest(benchmark::State& state)
        {
            Hierarchy hierarchy;
            Context scenario{ &hierarchy.suite };
            Fill(scenario, static_cast<std::size_t>(state.range(0)));

            for (auto _ : state)
                benchmark::DoNotOptimize(scenario.GetAt<int>(typeKeys[0]));
        }

        BENCHMARK(BmScenarioLookupOldest)->Arg(1)->Arg(4)->Arg(16)->Arg(64);

        void BmScenarioLookupNewest(benchmark::State& state)
        {
            const auto entries = static_cast<std::size_t>(state.range(0));
            Hierarchy hierarchy;
            Context scenario{ &hierarchy.suite };
            Fill(scenario, entries);

            for (auto _ : state)
                benchmark::DoNotOptimize(scenario.GetAt<int>(typeKeys[entries - 1]));
        }

        BENCHMARK(BmScenarioLookupNewest)->Arg(1)->Arg(4)->Arg(16)->Arg(64);

        // A program-level value read from a scenario misses the scenario and feature contexts first.
        void BmScenarioLookupFallThrough(benchmark::State& state)
        {
            Hierarchy hierarchy;
            Context scenario{ &hierarchy.suite };
            Fill(scenario, static_cast<std::size_t>(state.range(0)));

            for (auto _ : state)
                benchmark::DoNotOptimize(scenario.Get<Fixture>());
        }

        BENCHMARK(BmScenarioLookupFallThrough)->Arg(0)->Arg(4)->Arg(16)->Arg(64);
    }
}
