// #include "benchmark/benchmark.h"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/Steps.hpp"
#include <benchmark/benchmark_api.h>
#include <benchmark/registration.h>
#include <benchmark/state.h>
#include <benchmark/utils.h>
#include <memory>
#include <string>

namespace cucumber_cpp::library::context
{
    namespace
    {
        template<typename T>
        void BmContextInsertion(benchmark::State& state)
        {
            for (auto _ : state)
            {
                cucumber_cpp::Context context{ std::make_shared<T>() };

                for (auto i = 0; i < 1000; ++i)
                    context.InsertAt(std::to_string(i), i);
            }
        }

        BENCHMARK(BmContextInsertion<ContextStorageFactoryImpl>);

        template<class T>
        auto CreateContext(int size)
        {
            cucumber_cpp::Context context{ std::make_shared<T>() };

            for (int i = 0; i < size; ++i)
                context.InsertAt("long_key_goes_here_" + std::to_string(i), i);

            return context;
        };

        template<class T>
        const cucumber_cpp::Context prefilledContext = CreateContext<T>(10000);

        template<typename T>
        void BmContextLookupLow(benchmark::State& state)
        {
            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext<T>.template Get<int>("long_key_goes_here_250"));
            }
        }

        BENCHMARK(BmContextLookupLow<ContextStorageFactoryImpl>);

        template<typename T>
        void BmContextLookupHigh(benchmark::State& state)
        {
            for (auto _ : state)
            {
                benchmark::DoNotOptimize(prefilledContext<T>.template Get<int>("long_key_goes_here_9750"));
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
    }
}
