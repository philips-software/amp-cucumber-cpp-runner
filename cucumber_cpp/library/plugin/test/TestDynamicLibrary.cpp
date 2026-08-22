#include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utility>

namespace cucumber_cpp::library::plugin
{
    using testing::Eq;

    namespace
    {
        const std::filesystem::path fixturePath{ PLUGIN_TEST_FIXTURE_PATH };
        using AnswerFn = int (*)();
    }

    TEST(DynamicLibrary, throws_when_library_does_not_exist)
    {
        EXPECT_THROW((DynamicLibrary{ "/no/such/library.so" }), PluginError);
    }

    TEST(DynamicLibrary, resolves_exported_symbol)
    {
        const DynamicLibrary library{ fixturePath };

        const auto answer = library.GetSymbol<AnswerFn>("CcrTestFixtureAnswer");

        EXPECT_THAT(answer(), Eq(42));
    }

    TEST(DynamicLibrary, throws_when_symbol_is_missing)
    {
        const DynamicLibrary library{ fixturePath };

        EXPECT_THROW(library.GetSymbol<AnswerFn>("no_such_symbol"), PluginError);
    }

    TEST(DynamicLibrary, exposes_canonical_path)
    {
        const DynamicLibrary library{ fixturePath };

        EXPECT_THAT(library.Path(), Eq(std::filesystem::weakly_canonical(fixturePath)));
    }

    TEST(DynamicLibrary, move_construction_preserves_symbol_resolution)
    {
        DynamicLibrary library{ fixturePath };
        const DynamicLibrary moved{ std::move(library) };

        const auto answer = moved.GetSymbol<AnswerFn>("CcrTestFixtureAnswer");

        EXPECT_THAT(answer(), Eq(42));
    }

    TEST(DynamicLibrary, move_assignment_preserves_symbol_resolution)
    {
        DynamicLibrary library{ fixturePath };
        DynamicLibrary other{ fixturePath };

        other = std::move(library);

        const auto answer = other.GetSymbol<AnswerFn>("CcrTestFixtureAnswer");

        EXPECT_THAT(answer(), Eq(42));
    }
}
