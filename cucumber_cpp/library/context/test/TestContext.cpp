#include "cucumber_cpp/library/context/Context.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::context
{
    struct TestContext : testing::Test
    {
        struct Obj;

        struct Mock
        {
            MOCK_METHOD(void, Deconstructing, (const Obj*));
        };

        struct Obj
        {
            explicit Obj(Mock* mock = nullptr)
                : mock{ mock }
            {}

            ~Obj()
            {
                if (mock)
                {
                    mock->Deconstructing(this);
                }
            }

            Mock* mock;
        };

        struct Other
        {};

        struct Base
        {
            virtual std::string Name() const
            {
                return "Base";
            }
        };

        struct Derived : Base
        {
            Derived() = default;

            Derived(const char* a, int b)
                : a{ a }
                , b{ b }
            {}

            std::string
            Name() const override
            {
                return "Derived";
            }

            const char* a{ "default" };
            int b{ 10 };
        };

        struct LeadingBase
        {
            virtual ~LeadingBase() = default;
        };

        struct MultipleDerived : LeadingBase, Base
        {
            std::string Name() const override
            {
                return "MultipleDerived";
            }
        };

        struct NonDefaultCtor
        {
            NonDefaultCtor(int a, int b)
                : a{ a }
                , b{ b }
            {}

            int a;
            int b;
        };

        struct NonCopyable
        {
            NonCopyable() = default;

            NonCopyable(NonCopyable&) = delete;
            NonCopyable& operator=(NonCopyable&) = delete;

            NonCopyable(NonCopyable&&) = delete;
            NonCopyable& operator=(NonCopyable&&) = delete;
        };

        Context programContext{ std::make_shared<ContextStorageFactoryThreadSafe>() };
        Context scenarioContext{ &programContext };
    };

    TEST_F(TestContext, GetByReference)
    {
        programContext.Emplace<Obj>();
        const auto& expected = *scenarioContext.GetShared<Obj>();
        const auto& actual = scenarioContext.Get<Obj>();

        EXPECT_THAT(expected, testing::Ref(actual));
    }

    TEST_F(TestContext, GetAtReturnsReferenceToKeyedValue)
    {
        const auto value = scenarioContext.EmplaceAt<int>("key", 42);

        const auto& actual = scenarioContext.GetAt<int>("key");

        EXPECT_THAT(actual, testing::Eq(42));
        EXPECT_THAT(&actual, testing::Pointer(value.get()));
    }

    TEST_F(TestContext, GetSharedAtReturnsSharedOwnerForKeyedValue)
    {
        const auto value = scenarioContext.EmplaceAt<int>("key", 42);

        const auto actual = scenarioContext.GetSharedAt<int>("key");

        EXPECT_THAT(actual, testing::Eq(value));
        EXPECT_THAT(actual.get(), testing::Pointer(value.get()));
    }

    TEST_F(TestContext, GetOrReturnsFallbackWithoutInserting)
    {
        const auto& context = scenarioContext;

        EXPECT_THAT(context.GetOr<int>(42), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
        EXPECT_THAT(programContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrReturnsIndependentValueFromLvalueFallback)
    {
        std::string fallback{ "fallback" };

        static_assert(std::is_same_v<decltype(scenarioContext.GetOr<std::string>(fallback)), std::string>);
        auto result = scenarioContext.GetOr<std::string>(fallback);
        result = "changed";

        EXPECT_THAT(fallback, testing::Eq("fallback"));
        EXPECT_THAT(scenarioContext.Contains<std::string>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrSupportsNonDefaultConstructibleFallback)
    {
        const auto result = scenarioContext.GetOr<NonDefaultCtor>(NonDefaultCtor{ 1, 2 });

        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(scenarioContext.Contains<NonDefaultCtor>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrReturnsStoredValueInsteadOfFallback)
    {
        scenarioContext.Emplace<int>(7);
        const auto& context = scenarioContext;

        static_assert(std::is_same_v<decltype(context.GetOr<int>(42)), int>);
        EXPECT_THAT(context.GetOr<int>(42), testing::Eq(7));
        EXPECT_THAT(context.Get<int>(), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrReturnsIndependentCopyOfStoredValue)
    {
        scenarioContext.Emplace<std::string>("stored");
        const auto& result = scenarioContext.GetOr<std::string>(std::string{ "fallback" });

        EXPECT_THAT(result, testing::Eq("stored"));
        EXPECT_THAT(&result, testing::Not(testing::Pointer(&scenarioContext.Get<std::string>())));

        scenarioContext.Get<std::string>() = "changed";

        EXPECT_THAT(result, testing::Eq("stored"));
    }

    TEST_F(TestContext, GetOrFindsParentValueWithoutInsertingLocally)
    {
        programContext.Emplace<int>(7);

        EXPECT_THAT(scenarioContext.GetOr<int>(42), testing::Eq(7));

        programContext.Clear<int>();

        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrPrefersLocalValueOverParentValue)
    {
        programContext.Emplace<int>(7);
        scenarioContext.Emplace<int>(9);

        EXPECT_THAT(scenarioContext.GetOr<int>(42), testing::Eq(9));
        EXPECT_THAT(programContext.Get<int>(), testing::Eq(7));
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(9));
    }

    TEST_F(TestContext, GetOrDoesNotUseExplicitlyKeyedValue)
    {
        scenarioContext.EmplaceAt<int>("key", 7);

        EXPECT_THAT(scenarioContext.GetOr<int>(42), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrElseInvokesFactoryAndReturnsValueWithoutInserting)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto& context = scenarioContext;

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        EXPECT_THAT(context.GetOrElse<int>(factory.AsStdFunction()), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
        EXPECT_THAT(programContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseReturnsFactoryResultByValue)
    {
        testing::StrictMock<testing::MockFunction<std::string()>> factory;

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrElse<std::string>(factory.AsStdFunction())), std::string>);

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(std::string{ "fallback" }));

        EXPECT_THAT(scenarioContext.GetOrElse<std::string>(factory.AsStdFunction()), testing::Eq("fallback"));
        EXPECT_THAT(scenarioContext.Contains<std::string>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseSupportsNonDefaultConstructibleFallback)
    {
        testing::StrictMock<testing::MockFunction<NonDefaultCtor()>> factory;

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(NonDefaultCtor{ 1, 2 }));

        const auto result = scenarioContext.GetOrElse<NonDefaultCtor>(factory.AsStdFunction());

        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(scenarioContext.Contains<NonDefaultCtor>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseReturnsStoredValueWithoutInvokingFactory)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.Emplace<int>(7);
        const auto& context = scenarioContext;

        static_assert(std::is_same_v<decltype(context.GetOrElse<int>(factory.AsStdFunction())), int>);
        EXPECT_THAT(context.GetOrElse<int>(factory.AsStdFunction()), testing::Eq(7));
        EXPECT_THAT(context.Get<int>(), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrElseReturnsIndependentCopyOfStoredValue)
    {
        testing::StrictMock<testing::MockFunction<std::string()>> factory;
        scenarioContext.Emplace<std::string>("stored");

        const auto& result = scenarioContext.GetOrElse<std::string>(factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq("stored"));
        EXPECT_THAT(&result, testing::Not(testing::Pointer(&scenarioContext.Get<std::string>())));

        scenarioContext.Get<std::string>() = "changed";

        EXPECT_THAT(result, testing::Eq("stored"));
    }

    TEST_F(TestContext, GetOrElseFindsParentValueWithoutInvokingFactoryOrInsertingLocally)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        programContext.Emplace<int>(7);

        EXPECT_THAT(scenarioContext.GetOrElse<int>(factory.AsStdFunction()), testing::Eq(7));

        programContext.Clear<int>();

        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElsePrefersLocalValueOverParentValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        programContext.Emplace<int>(7);
        scenarioContext.Emplace<int>(9);

        EXPECT_THAT(scenarioContext.GetOrElse<int>(factory.AsStdFunction()), testing::Eq(9));
        EXPECT_THAT(programContext.Get<int>(), testing::Eq(7));
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(9));
    }

    TEST_F(TestContext, GetOrElseDoesNotUseExplicitlyKeyedValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("key", 7);

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        EXPECT_THAT(scenarioContext.GetOrElse<int>(factory.AsStdFunction()), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrAtReturnsFallbackWithoutInserting)
    {
        const auto& context = scenarioContext;

        EXPECT_THAT(context.GetOrAt<int>("key", 42), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(programContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrAtReturnsIndependentValueFromLvalueFallback)
    {
        std::string fallback{ "fallback" };

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrAt<std::string>("key", fallback)), std::string>);
        auto result = scenarioContext.GetOrAt<std::string>("key", fallback);
        result = "changed";

        EXPECT_THAT(fallback, testing::Eq("fallback"));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrAtReturnsStoredValueInsteadOfFallback)
    {
        scenarioContext.EmplaceAt<int>("key", 7);
        const auto& context = scenarioContext;

        static_assert(std::is_same_v<decltype(context.GetOrAt<int>("key", 42)), int>);
        EXPECT_THAT(context.GetOrAt<int>("key", 42), testing::Eq(7));
        EXPECT_THAT(context.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrAtReturnsIndependentCopyOfStoredValue)
    {
        scenarioContext.EmplaceAt<std::string>("key", "stored");
        const auto& result = scenarioContext.GetOrAt<std::string>("key", std::string{ "fallback" });

        EXPECT_THAT(result, testing::Eq("stored"));
        EXPECT_THAT(&result, testing::Not(testing::Pointer(&scenarioContext.GetAt<std::string>("key"))));

        scenarioContext.GetAt<std::string>("key") = "changed";

        EXPECT_THAT(result, testing::Eq("stored"));
    }

    TEST_F(TestContext, GetOrAtFindsParentValueWithoutInsertingLocally)
    {
        programContext.EmplaceAt<int>("key", 7);

        EXPECT_THAT(scenarioContext.GetOrAt<int>("key", 42), testing::Eq(7));

        programContext.Clear("key");

        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrAtPrefersLocalValueOverParentValue)
    {
        programContext.EmplaceAt<int>("key", 7);
        scenarioContext.EmplaceAt<int>("key", 9);

        EXPECT_THAT(scenarioContext.GetOrAt<int>("key", 42), testing::Eq(9));
        EXPECT_THAT(programContext.GetAt<int>("key"), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(9));
    }

    TEST_F(TestContext, GetOrAtDoesNotUseTypeKeyedValue)
    {
        scenarioContext.Emplace<int>(7);

        EXPECT_THAT(scenarioContext.GetOrAt<int>("key", 42), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrElseAtInvokesFactoryAndReturnsValueWithoutInserting)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto& context = scenarioContext;

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        EXPECT_THAT(context.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(programContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseAtReturnsFactoryResultByValue)
    {
        testing::StrictMock<testing::MockFunction<std::string()>> factory;

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrElseAt<std::string>("key", factory.AsStdFunction())), std::string>);

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(std::string{ "fallback" }));

        EXPECT_THAT(scenarioContext.GetOrElseAt<std::string>("key", factory.AsStdFunction()), testing::Eq("fallback"));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseAtSupportsNonDefaultConstructibleFallback)
    {
        testing::StrictMock<testing::MockFunction<NonDefaultCtor()>> factory;

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(NonDefaultCtor{ 1, 2 }));

        const auto result = scenarioContext.GetOrElseAt<NonDefaultCtor>("key", factory.AsStdFunction());

        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseAtReturnsStoredValueWithoutInvokingFactory)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("key", 7);
        const auto& context = scenarioContext;

        static_assert(std::is_same_v<decltype(context.GetOrElseAt<int>("key", factory.AsStdFunction())), int>);
        EXPECT_THAT(context.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(7));
        EXPECT_THAT(context.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrElseAtReturnsIndependentCopyOfStoredValue)
    {
        testing::StrictMock<testing::MockFunction<std::string()>> factory;
        scenarioContext.EmplaceAt<std::string>("key", "stored");

        const auto& result = scenarioContext.GetOrElseAt<std::string>("key", factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq("stored"));
        EXPECT_THAT(&result, testing::Not(testing::Pointer(&scenarioContext.GetAt<std::string>("key"))));

        scenarioContext.GetAt<std::string>("key") = "changed";

        EXPECT_THAT(result, testing::Eq("stored"));
    }

    TEST_F(TestContext, GetOrElseAtFindsParentValueWithoutInvokingFactoryOrInsertingLocally)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        programContext.EmplaceAt<int>("key", 7);

        EXPECT_THAT(scenarioContext.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(7));

        programContext.Clear("key");

        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrElseAtPrefersLocalValueOverParentValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        programContext.EmplaceAt<int>("key", 7);
        scenarioContext.EmplaceAt<int>("key", 9);

        EXPECT_THAT(scenarioContext.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(9));
        EXPECT_THAT(programContext.GetAt<int>("key"), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(9));
    }

    TEST_F(TestContext, GetOrElseAtDoesNotUseTypeKeyedValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.Emplace<int>(7);

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        EXPECT_THAT(scenarioContext.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrElseAtDoesNotUseAnotherExplicitKey)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("other-key", 7);

        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        EXPECT_THAT(scenarioContext.GetOrElseAt<int>("key", factory.AsStdFunction()), testing::Eq(42));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(scenarioContext.GetAt<int>("other-key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrEmplaceConstructsAndStoresByType)
    {
        const auto& result = scenarioContext.GetOrEmplace<NonDefaultCtor>(1, 2);

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrEmplace<NonDefaultCtor>(1, 2)), NonDefaultCtor&>);
        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<NonDefaultCtor>()));
    }

    TEST_F(TestContext, GetOrEmplaceReturnsExistingValueWithoutReconstructing)
    {
        struct CtorProbe
        {
            explicit CtorProbe(testing::MockFunction<int()>& factory)
                : value{ factory.Call() }
            {}

            const int value;
        };

        testing::StrictMock<testing::MockFunction<int()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(7));

        const auto& first = scenarioContext.GetOrEmplace<CtorProbe>(factory);
        const auto& second = scenarioContext.GetOrEmplace<CtorProbe>(factory);

        EXPECT_THAT(&second, testing::Pointer(&first));
        EXPECT_THAT(second.value, testing::Eq(7));
    }

    TEST_F(TestContext, GetOrEmplaceFindsParentValueWithoutInsertingLocally)
    {
        const auto parentValue = programContext.Emplace<int>(7);

        const auto& result = scenarioContext.GetOrEmplace<int>(42);

        EXPECT_THAT(&result, testing::Pointer(parentValue.get()));
        EXPECT_THAT(result, testing::Eq(7));

        programContext.Clear<int>();

        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplacePrefersLocalValueOverParentValue)
    {
        const auto parentValue = programContext.Emplace<int>(7);
        const auto localValue = scenarioContext.Emplace<int>(9);

        const auto& result = scenarioContext.GetOrEmplace<int>(42);

        EXPECT_THAT(&result, testing::Not(testing::Pointer(parentValue.get())));
        EXPECT_THAT(&result, testing::Pointer(localValue.get()));
        EXPECT_THAT(programContext.Get<int>(), testing::Eq(7));
        EXPECT_THAT(result, testing::Eq(9));
    }

    TEST_F(TestContext, GetOrEmplaceUsesTypeKeyInsteadOfNamedKey)
    {
        scenarioContext.EmplaceAt<int>("key", 7);

        const auto& result = scenarioContext.GetOrEmplace<int>(9);

        EXPECT_THAT(result, testing::Eq(9));
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(9));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrEmplaceAtConstructsAndStoresByKey)
    {
        const auto& result = scenarioContext.GetOrEmplaceAt<NonDefaultCtor>("key", 1, 2);

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrEmplaceAt<NonDefaultCtor>("key", 1, 2)), NonDefaultCtor&>);
        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.GetAt<NonDefaultCtor>("key")));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsTrue());
        EXPECT_THAT(scenarioContext.Contains<NonDefaultCtor>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAtReturnsExistingValueWithoutReconstructing)
    {
        struct CtorProbe
        {
            explicit CtorProbe(testing::MockFunction<int()>& factory)
                : value{ factory.Call() }
            {}

            const int value;
        };

        testing::StrictMock<testing::MockFunction<int()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(7));

        const auto& first = scenarioContext.GetOrEmplaceAt<CtorProbe>("key", factory);
        const auto& second = scenarioContext.GetOrEmplaceAt<CtorProbe>("key", factory);

        EXPECT_THAT(&second, testing::Pointer(&first));
        EXPECT_THAT(second.value, testing::Eq(7));
    }

    TEST_F(TestContext, GetOrEmplaceAtFindsParentValueWithoutInsertingLocally)
    {
        const auto parentValue = programContext.EmplaceAt<int>("key", 7);

        const auto& result = scenarioContext.GetOrEmplaceAt<int>("key", 42);

        EXPECT_THAT(&result, testing::Pointer(parentValue.get()));
        EXPECT_THAT(result, testing::Eq(7));

        programContext.Clear("key");

        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAtPrefersLocalValueOverParentValue)
    {
        const auto parentValue = programContext.EmplaceAt<int>("key", 7);
        const auto localValue = scenarioContext.EmplaceAt<int>("key", 9);

        const auto& result = scenarioContext.GetOrEmplaceAt<int>("key", 42);

        EXPECT_THAT(&result, testing::Not(testing::Pointer(parentValue.get())));
        EXPECT_THAT(&result, testing::Pointer(localValue.get()));
        EXPECT_THAT(programContext.GetAt<int>("key"), testing::Eq(7));
        EXPECT_THAT(result, testing::Eq(9));
    }

    TEST_F(TestContext, GetOrEmplaceAtDoesNotUseTypeKeyedValue)
    {
        scenarioContext.Emplace<int>(7);

        const auto& result = scenarioContext.GetOrEmplaceAt<int>("key", 42);

        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, GetOrEmplaceAtDoesNotUseAnotherExplicitKey)
    {
        scenarioContext.EmplaceAt<int>("other-key", 7);

        const auto& result = scenarioContext.GetOrEmplaceAt<int>("key", 42);

        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(scenarioContext.GetAt<int>("other-key"), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, GetOrEmplaceWithInvokesFactoryAndStoresValueByType)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        const auto& result = scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction());

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction())), int&>);
        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<int>()));
    }

    TEST_F(TestContext, GetOrEmplaceWithReturnsExistingValueWithoutInvokingFactory)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.Emplace<int>(7);

        const auto& result = scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq(7));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<int>()));
    }

    TEST_F(TestContext, GetOrEmplaceWithFindsParentValueWithoutInvokingFactoryOrInsertingLocally)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto parentValue = programContext.Emplace<int>(7);

        const auto& result = scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Pointer(parentValue.get()));
        EXPECT_THAT(result, testing::Eq(7));

        programContext.Clear<int>();

        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceWithPrefersLocalValueOverParentValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto parentValue = programContext.Emplace<int>(7);
        const auto localValue = scenarioContext.Emplace<int>(9);

        const auto& result = scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Not(testing::Pointer(parentValue.get())));
        EXPECT_THAT(&result, testing::Pointer(localValue.get()));
        EXPECT_THAT(result, testing::Eq(9));
    }

    TEST_F(TestContext, GetOrEmplaceWithUsesTypeKeyInsteadOfNamedKey)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("key", 7);
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        const auto& result = scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(7));
    }

    TEST_F(TestContext, GetOrEmplaceWithDoesNotInsertWhenFactoryThrows)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Throw(std::runtime_error{ "factory failed" }));

        EXPECT_THROW(
            static_cast<void>(scenarioContext.GetOrEmplaceWith<int>(factory.AsStdFunction())),
            std::runtime_error);
        EXPECT_THAT(scenarioContext.Contains<int>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceWithUsesEntryInsertedByFactory)
    {
        auto& mutableContext = scenarioContext;
        const auto& result = mutableContext.GetOrEmplaceWith<int>([&mutableContext] {
            mutableContext.Emplace<int>(7);
            return 42;
        });

        EXPECT_THAT(result, testing::Eq(7));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<int>()));
    }

    TEST_F(TestContext, GetOrEmplaceAtWithInvokesFactoryAndStoresValueByKey)
    {
        testing::StrictMock<testing::MockFunction<NonDefaultCtor()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(NonDefaultCtor{ 1, 2 }));

        const auto& result = scenarioContext.GetOrEmplaceAtWith<NonDefaultCtor>("key", factory.AsStdFunction());

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrEmplaceAtWith<NonDefaultCtor>("key", factory.AsStdFunction())), NonDefaultCtor&>);
        EXPECT_THAT(result.a, testing::Eq(1));
        EXPECT_THAT(result.b, testing::Eq(2));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.GetAt<NonDefaultCtor>("key")));
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsTrue());
        EXPECT_THAT(scenarioContext.Contains<NonDefaultCtor>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAtWithReturnsExistingValueWithoutInvokingFactory)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("key", 7);

        const auto& result = scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq(7));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.GetAt<int>("key")));
    }

    TEST_F(TestContext, GetOrEmplaceAtWithFindsParentValueWithoutInvokingFactoryOrInsertingLocally)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto parentValue = programContext.EmplaceAt<int>("key", 7);

        const auto& result = scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Pointer(parentValue.get()));
        EXPECT_THAT(result, testing::Eq(7));

        programContext.Clear("key");

        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAtWithPrefersLocalValueOverParentValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        const auto parentValue = programContext.EmplaceAt<int>("key", 7);
        const auto localValue = scenarioContext.EmplaceAt<int>("key", 9);

        const auto& result = scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Not(testing::Pointer(parentValue.get())));
        EXPECT_THAT(&result, testing::Pointer(localValue.get()));
        EXPECT_THAT(result, testing::Eq(9));
    }

    TEST_F(TestContext, GetOrEmplaceAtWithDoesNotUseTypeKeyedValue)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.Emplace<int>(7);
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        const auto& result = scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(scenarioContext.Get<int>(), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, GetOrEmplaceAtWithDoesNotUseAnotherExplicitKey)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        scenarioContext.EmplaceAt<int>("other-key", 7);
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(42));

        const auto& result = scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction());

        EXPECT_THAT(result, testing::Eq(42));
        EXPECT_THAT(scenarioContext.GetAt<int>("other-key"), testing::Eq(7));
        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, GetOrEmplaceAtWithDoesNotInsertWhenFactoryThrows)
    {
        testing::StrictMock<testing::MockFunction<int()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Throw(std::runtime_error{ "factory failed" }));

        EXPECT_THROW(
            static_cast<void>(scenarioContext.GetOrEmplaceAtWith<int>("key", factory.AsStdFunction())),
            std::runtime_error);
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAsWithInvokesFactoryAndStoresDerivedByBaseType)
    {
        testing::StrictMock<testing::MockFunction<MultipleDerived()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(MultipleDerived{}));

        const auto& result = scenarioContext.GetOrEmplaceAsWith<Base, MultipleDerived>(factory.AsStdFunction());

        static_assert(std::is_same_v<decltype(scenarioContext.GetOrEmplaceAsWith<Base, MultipleDerived>(factory.AsStdFunction())), Base&>);
        EXPECT_THAT(result.Name(), testing::StrEq("MultipleDerived"));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<Base>()));
        EXPECT_THAT(scenarioContext.Contains<Base>(), testing::IsTrue());
        EXPECT_THAT(scenarioContext.Contains<MultipleDerived>(), testing::IsFalse());
    }

    TEST_F(TestContext, EmplaceAsStoresAdjustedBasePointerForMultipleInheritance)
    {
        const auto value = scenarioContext.EmplaceAs<Base, MultipleDerived>();

        EXPECT_THAT(value->Name(), testing::StrEq("MultipleDerived"));
        EXPECT_THAT(value.get(), testing::Pointer(&scenarioContext.Get<Base>()));
    }

    TEST_F(TestContext, InsertAsStoresAdjustedBasePointerForMultipleInheritance)
    {
        const MultipleDerived value;

        scenarioContext.InsertAs<Base>(value);

        EXPECT_THAT(scenarioContext.Get<Base>().Name(), testing::StrEq("MultipleDerived"));
        EXPECT_THAT(&scenarioContext.Get<Base>(), testing::Not(testing::Pointer(&value)));
    }

    TEST_F(TestContext, InsertRefAsStoresAdjustedBasePointerForMultipleInheritance)
    {
        MultipleDerived value;

        scenarioContext.InsertRefAs<Base, MultipleDerived>(value);

        EXPECT_THAT(&scenarioContext.Get<Base>(), testing::Pointer(static_cast<Base*>(&value)));
    }

    TEST_F(TestContext, GetOrEmplaceAsWithReturnsExistingValueWithoutInvokingFactory)
    {
        testing::StrictMock<testing::MockFunction<Derived()>> factory;
        scenarioContext.EmplaceAs<Base, Derived>("stored", 2);

        const auto& result = scenarioContext.GetOrEmplaceAsWith<Base, Derived>(factory.AsStdFunction());

        EXPECT_THAT(result.Name(), testing::StrEq("Derived"));
        EXPECT_THAT(&result, testing::Pointer(&scenarioContext.Get<Base>()));
    }

    TEST_F(TestContext, GetOrEmplaceAsWithFindsParentValueWithoutInvokingFactoryOrInsertingLocally)
    {
        testing::StrictMock<testing::MockFunction<Derived()>> factory;
        const auto parentValue = programContext.EmplaceAs<Base, Derived>("parent", 2);

        const auto& result = scenarioContext.GetOrEmplaceAsWith<Base, Derived>(factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Pointer(parentValue.get()));
        EXPECT_THAT(result.Name(), testing::StrEq("Derived"));

        programContext.Clear<Base>();

        EXPECT_THAT(scenarioContext.Contains<Base>(), testing::IsFalse());
    }

    TEST_F(TestContext, GetOrEmplaceAsWithPrefersLocalValueOverParentValue)
    {
        testing::StrictMock<testing::MockFunction<Derived()>> factory;
        const auto parentValue = programContext.EmplaceAs<Base, Derived>("parent", 2);
        const auto localValue = scenarioContext.EmplaceAs<Base, Derived>("local", 4);

        const auto& result = scenarioContext.GetOrEmplaceAsWith<Base, Derived>(factory.AsStdFunction());

        EXPECT_THAT(&result, testing::Not(testing::Pointer(parentValue.get())));
        EXPECT_THAT(&result, testing::Pointer(localValue.get()));
        EXPECT_THAT(result.Name(), testing::StrEq("Derived"));
        EXPECT_THAT(programContext.Get<Base>().Name(), testing::StrEq("Derived"));
    }

    TEST_F(TestContext, GetOrEmplaceAsWithUsesBaseTypeKeyInsteadOfDerivedTypeOrNamedKeys)
    {
        testing::StrictMock<testing::MockFunction<Derived()>> factory;
        scenarioContext.Emplace<Derived>();
        scenarioContext.EmplaceAt<Base>("named", Base{});
        EXPECT_CALL(factory, Call()).WillOnce(testing::Return(Derived{ "factory", 6 }));

        const auto& result = scenarioContext.GetOrEmplaceAsWith<Base, Derived>(factory.AsStdFunction());

        EXPECT_THAT(result.Name(), testing::StrEq("Derived"));
        EXPECT_THAT(scenarioContext.Get<Derived>().Name(), testing::StrEq("Derived"));
        EXPECT_THAT(scenarioContext.GetAt<Base>("named").Name(), testing::StrEq("Base"));
    }

    TEST_F(TestContext, GetOrEmplaceAsWithDoesNotInsertWhenFactoryThrows)
    {
        testing::StrictMock<testing::MockFunction<Derived()>> factory;
        EXPECT_CALL(factory, Call()).WillOnce(testing::Throw(std::runtime_error{ "factory failed" }));

        EXPECT_THROW(
            static_cast<void>(scenarioContext.GetOrEmplaceAsWith<Base, Derived>(factory.AsStdFunction())),
            std::runtime_error);
        EXPECT_THAT(scenarioContext.Contains<Base>(), testing::IsFalse());
    }

    TEST_F(TestContext, OverwriteValue)
    {
        Mock mock;

        const auto* first = programContext.Emplace<Obj>(&mock).get();
        EXPECT_CALL(mock, Deconstructing(first));

        const auto* second = programContext.Emplace<Obj>().get();

        EXPECT_THAT(first, testing::Not(testing::Pointer(second)));
    }

    TEST_F(TestContext, OverrideValue)
    {
        const auto* first = programContext.Emplace<Obj>().get();
        const auto* second = scenarioContext.Emplace<Obj>().get();

        EXPECT_THAT(first, testing::Not(testing::Pointer(second)));
    }

    TEST_F(TestContext, ContainsInParentByType)
    {
        programContext.Emplace<Obj>();

        EXPECT_THAT(scenarioContext.Contains<Obj>(), testing::IsTrue());
    }

    TEST_F(TestContext, ContainsInParentByName)
    {
        programContext.EmplaceAt<Obj>("name");

        EXPECT_THAT(scenarioContext.Contains("name"), testing::IsTrue());
    }

    TEST_F(TestContext, ContainsInDerivedByType)
    {
        scenarioContext.Emplace<Obj>();

        EXPECT_THAT(programContext.Contains<Obj>(), testing::IsFalse());
        EXPECT_THAT(scenarioContext.Contains<Obj>(), testing::IsTrue());
    }

    TEST_F(TestContext, ContainsInDerivedByName)
    {
        scenarioContext.EmplaceAt<Obj>("key");

        EXPECT_THAT(programContext.Contains("key"), testing::IsFalse());
        EXPECT_THAT(scenarioContext.Contains("key"), testing::IsTrue());
    }

    TEST_F(TestContext, EmplaceByType)
    {
        scenarioContext.Emplace<NonDefaultCtor>(1, 2);

        const auto& obj = scenarioContext.Get<NonDefaultCtor>();
        EXPECT_THAT(obj.a, testing::Eq(1));
        EXPECT_THAT(obj.b, testing::Eq(2));
    }

    TEST_F(TestContext, EmplaceByName)
    {
        scenarioContext.EmplaceAt<Derived>("key1", "a", 2);
        scenarioContext.EmplaceAt<Derived>("key2", "b", 4);

        const auto& key1 = scenarioContext.GetAt<Derived>("key1");
        EXPECT_THAT(key1.a, testing::StrEq("a"));
        EXPECT_THAT(key1.b, testing::Eq(2));

        const auto& key2 = scenarioContext.GetAt<Derived>("key2");
        EXPECT_THAT(key2.a, testing::StrEq("b"));
        EXPECT_THAT(key2.b, testing::Eq(4));
    }

    TEST_F(TestContext, EmplaceByBaseAndDerived)
    {
        scenarioContext.EmplaceAs<Base, Derived>("a", 2);

        EXPECT_THAT(scenarioContext.Get<Base>().Name(), testing::StrEq("Derived"));
    }

    TEST_F(TestContext, InsertRefByType)
    {
        NonCopyable nonCopyable;

        scenarioContext.InsertRef<NonCopyable>(nonCopyable);
        EXPECT_THAT(&scenarioContext.Get<NonCopyable>(), testing::Pointer(&nonCopyable));
    }

    TEST_F(TestContext, InsertRefAsType)
    {
        Derived derived;

        scenarioContext.InsertRefAs<Base, Derived>(derived);
        EXPECT_THAT(&scenarioContext.Get<Base>(), testing::Pointer(&derived));
    }

    TEST_F(TestContext, InsertRefByName)
    {
        NonCopyable nonCopyable;

        scenarioContext.InsertRefAt("key1", nonCopyable);
        scenarioContext.InsertRefAt("key2", nonCopyable);

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key1"), testing::Pointer(&nonCopyable));
        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key2"), testing::Pointer(&nonCopyable));
    }

    TEST_F(TestContext, InsertByType)
    {
        Obj obj;
        scenarioContext.Insert<Obj>(obj);

        EXPECT_THAT(&scenarioContext.Get<Obj>(), testing::Not(testing::Pointer(&obj)));
    }

    TEST_F(TestContext, InsertByTypeAs)
    {
        Derived derived;
        scenarioContext.InsertAs<Base, Derived>(derived);

        EXPECT_THAT(&scenarioContext.Get<Base>(), testing::Not(testing::Pointer(&derived)));
    }

    TEST_F(TestContext, InsertByName)
    {
        Obj obj;
        scenarioContext.InsertAt("key1", obj);
        scenarioContext.InsertAt("key2", obj);

        EXPECT_THAT(&scenarioContext.GetAt<Obj>("key1"), testing::Not(testing::Pointer(&obj)));
        EXPECT_THAT(&scenarioContext.GetAt<Obj>("key2"), testing::Not(testing::Pointer(&obj)));
        EXPECT_THAT(&scenarioContext.GetAt<Obj>("key1"), testing::Not(testing::Pointer(&scenarioContext.GetAt<Obj>("key2"))));
    }

    TEST_F(TestContext, KeyNotFound)
    {
        scenarioContext.Emplace<Obj>();

        EXPECT_THROW((void)programContext.Get<Obj>(), std::out_of_range);
        EXPECT_THROW((void)scenarioContext.Get<Base>(), std::out_of_range);
        EXPECT_THROW((void)scenarioContext.GetAt<int>("mykey"), std::out_of_range);
    }

    TEST_F(TestContext, SetSharedWithStringKey)
    {
        std::string key{ "key" };
        auto value = std::make_shared<int>(42);
        scenarioContext.SetShared(key, value);
        key.assign("changed");

        EXPECT_THAT(scenarioContext.GetSharedAt<int>("key"), testing::Eq(value));
    }

    TEST_F(TestContext, SetSharedWithMovedStringKey)
    {
        std::string key{ "key" };
        auto value = std::make_shared<int>(42);
        scenarioContext.SetShared(std::move(key), value);

        EXPECT_THAT(scenarioContext.GetSharedAt<int>("key"), testing::Eq(value));
    }

    TEST_F(TestContext, SetSharedWithStringLiteralKey)
    {
        auto value = std::make_shared<int>(42);
        scenarioContext.SetShared("key", value);

        EXPECT_THAT(scenarioContext.GetSharedAt<int>("key"), testing::Eq(value));
    }

    TEST_F(TestContext, SetSharedWithCStringKey)
    {
        char buffer[]{ "key" };
        const char* key = buffer;
        auto value = std::make_shared<int>(42);
        scenarioContext.SetShared(key, value);
        buffer[0] = 'x';

        EXPECT_THAT(scenarioContext.GetSharedAt<int>("key"), testing::Eq(value));
    }

    TEST_F(TestContext, EmplaceAtWithStringKey)
    {
        std::string key{ "key" };
        auto value = scenarioContext.EmplaceAt<NonDefaultCtor>(key, 1, 2);
        key.assign("changed");

        EXPECT_THAT(scenarioContext.GetSharedAt<NonDefaultCtor>("key"), testing::Eq(value));
        EXPECT_THAT(value->a, testing::Eq(1));
        EXPECT_THAT(value->b, testing::Eq(2));
    }

    TEST_F(TestContext, EmplaceAtWithMovedStringKey)
    {
        std::string key{ "key" };
        auto value = scenarioContext.EmplaceAt<std::unique_ptr<int>>(std::move(key), std::make_unique<int>(42));

        EXPECT_THAT(scenarioContext.GetSharedAt<std::unique_ptr<int>>("key"), testing::Eq(value));
        EXPECT_THAT(**value, testing::Eq(42));
    }

    TEST_F(TestContext, EmplaceAtWithStringLiteralKey)
    {
        auto value = scenarioContext.EmplaceAt<std::unique_ptr<int>>("key", std::make_unique<int>(42));

        EXPECT_THAT(scenarioContext.GetSharedAt<std::unique_ptr<int>>("key"), testing::Eq(value));
        EXPECT_THAT(**value, testing::Eq(42));
    }

    TEST_F(TestContext, EmplaceAtWithCStringKey)
    {
        char buffer[]{ "key" };
        const char* key = buffer;
        auto value = scenarioContext.EmplaceAt<std::unique_ptr<int>>(key, std::make_unique<int>(42));
        buffer[0] = 'x';

        EXPECT_THAT(scenarioContext.GetSharedAt<std::unique_ptr<int>>("key"), testing::Eq(value));
        EXPECT_THAT(**value, testing::Eq(42));
    }

    TEST_F(TestContext, InsertRefAtWithStringKey)
    {
        std::string key{ "key" };
        NonCopyable value;
        scenarioContext.InsertRefAt(key, value);
        key.assign("changed");

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key"), testing::Pointer(&value));
    }

    TEST_F(TestContext, InsertRefAtWithMovedStringKey)
    {
        std::string key{ "key" };
        NonCopyable value;
        scenarioContext.InsertRefAt(std::move(key), value);

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key"), testing::Pointer(&value));
    }

    TEST_F(TestContext, InsertRefAtWithStringLiteralKey)
    {
        NonCopyable value;
        scenarioContext.InsertRefAt("key", value);

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key"), testing::Pointer(&value));
    }

    TEST_F(TestContext, InsertRefAtWithCStringKey)
    {
        char buffer[]{ "key" };
        const char* key = buffer;
        NonCopyable value;
        scenarioContext.InsertRefAt(key, value);
        buffer[0] = 'x';

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key"), testing::Pointer(&value));
    }

    TEST_F(TestContext, InsertAtWithStringKey)
    {
        std::string key{ "key" };
        int value = 42;
        scenarioContext.InsertAt(key, value);
        key.assign("changed");
        value = 0;

        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, InsertAtWithMovedStringKey)
    {
        std::string key{ "key" };
        int value = 42;
        scenarioContext.InsertAt(std::move(key), value);
        value = 0;

        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, InsertAtWithStringLiteralKey)
    {
        int value = 42;
        scenarioContext.InsertAt("key", value);
        value = 0;

        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    TEST_F(TestContext, InsertAtWithCStringKey)
    {
        char buffer[]{ "key" };
        const char* key = buffer;
        int value = 42;
        scenarioContext.InsertAt(key, value);
        buffer[0] = 'x';
        value = 0;

        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

    // These compatibility tests intentionally call deprecated overloads.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    TEST_F(TestContext, SetSharedWithStringViewKey)
    {
        std::string buffer{ "prefixkeysuffix" };
        const std::string_view key{ buffer.data() + 6, 3 };
        auto value = std::make_shared<int>(42);
        scenarioContext.SetShared(key, value);
        buffer.assign("changed");

        EXPECT_THAT(scenarioContext.GetSharedAt<int>("key"), testing::Eq(value));
    }

    TEST_F(TestContext, EmplaceAtWithStringViewKey)
    {
        std::string buffer{ "prefixkeysuffix" };
        const std::string_view key{ buffer.data() + 6, 3 };
        auto value = scenarioContext.EmplaceAt<std::unique_ptr<int>>(key, std::make_unique<int>(42));
        buffer.assign("changed");

        EXPECT_THAT(scenarioContext.GetSharedAt<std::unique_ptr<int>>("key"), testing::Eq(value));
        EXPECT_THAT(**value, testing::Eq(42));
    }

    TEST_F(TestContext, InsertRefAtWithStringViewKey)
    {
        std::string buffer{ "prefixkeysuffix" };
        const std::string_view key{ buffer.data() + 6, 3 };
        NonCopyable value;
        scenarioContext.InsertRefAt(key, value);
        buffer.assign("changed");

        EXPECT_THAT(&scenarioContext.GetAt<NonCopyable>("key"), testing::Pointer(&value));
    }

    TEST_F(TestContext, InsertAtWithStringViewKey)
    {
        std::string buffer{ "prefixkeysuffix" };
        const std::string_view key{ buffer.data() + 6, 3 };
        int value = 42;
        scenarioContext.InsertAt(key, value);
        buffer.assign("changed");
        value = 0;

        EXPECT_THAT(scenarioContext.GetAt<int>("key"), testing::Eq(42));
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

    TEST_F(TestContext, OrderOfDestructionReverseOfOrderOfInsertion)
    {
        struct Ordered : Obj
        {
            Ordered(Mock& mock, int id)
                : Obj{ &mock }
                , id{ id }
            {}

            ~Ordered()
            {
                std::cout << "\ndestruct " << id;
            }

            int id;
        };

        auto privateContext = std::make_unique<Context>(std::make_shared<ContextStorageFactoryImpl>());
        Mock mock;

        const auto* dkey = privateContext->EmplaceAt<Ordered>("dkey", mock, 1).get();
        const auto* ckey = privateContext->EmplaceAt<Ordered>("ckey", mock, 2).get();
        const auto* bkey = privateContext->EmplaceAt<Ordered>("bkey", mock, 3).get();
        const auto* akey = privateContext->EmplaceAt<Ordered>("akey", mock, 4).get();
        const auto* xkey = privateContext->EmplaceAt<Ordered>("xkey", mock, 5).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(ckey));

        EXPECT_CALL(mock, Deconstructing(xkey));
        EXPECT_CALL(mock, Deconstructing(akey));
        EXPECT_CALL(mock, Deconstructing(bkey));
        EXPECT_CALL(mock, Deconstructing(dkey));

        privateContext->Clear("ckey");
        privateContext = nullptr;
    }

    TEST_F(TestContext, OrderOfDestructionReverseOfOrderOfInsertionWithoutDiagnosticOutput)
    {
        auto privateContext = std::make_unique<Context>(std::make_shared<ContextStorageFactoryImpl>());
        Mock mock;

        const auto* dkey = privateContext->EmplaceAt<Obj>("dkey", &mock).get();
        const auto* ckey = privateContext->EmplaceAt<Obj>("ckey", &mock).get();
        const auto* bkey = privateContext->EmplaceAt<Obj>("bkey", &mock).get();
        const auto* akey = privateContext->EmplaceAt<Obj>("akey", &mock).get();
        const auto* xkey = privateContext->EmplaceAt<Obj>("xkey", &mock).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(ckey));

        EXPECT_CALL(mock, Deconstructing(xkey));
        EXPECT_CALL(mock, Deconstructing(akey));
        EXPECT_CALL(mock, Deconstructing(bkey));
        EXPECT_CALL(mock, Deconstructing(dkey));

        privateContext->Clear("ckey");
        privateContext = nullptr;
    }

    namespace
    {
        struct TestContextOrdering : TestContext
            , testing::WithParamInterface<bool>
        {
            static std::shared_ptr<ContextStorageFactory> CreateFactory()
            {
                if (GetParam())
                {
                    return std::make_shared<ContextStorageFactoryThreadSafe>();
                }

                return std::make_shared<ContextStorageFactoryImpl>();
            }
        };
    }

    TEST_P(TestContextOrdering, DestructionReleasesNewestFirst)
    {
        testing::StrictMock<Mock> mock;
        auto context = std::make_unique<Context>(CreateFactory());

        const auto* first = context->EmplaceAt<Obj>("first", &mock).get();
        const auto* second = context->EmplaceAt<Obj>("second", &mock).get();
        const auto* third = context->EmplaceAt<Obj>("third", &mock).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(third));
        EXPECT_CALL(mock, Deconstructing(second));
        EXPECT_CALL(mock, Deconstructing(first));

        context.reset();
    }

    TEST_P(TestContextOrdering, OverwriteBecomesNewestInsertion)
    {
        testing::StrictMock<Mock> mock;
        auto context = std::make_unique<Context>(CreateFactory());

        const auto* first = context->EmplaceAt<Obj>("first", &mock).get();
        const auto* second = context->EmplaceAt<Obj>("second", &mock).get();
        const auto* third = context->EmplaceAt<Obj>("third", &mock).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(first));
        const auto* replacement = context->EmplaceAt<Obj>("first", &mock).get();
        EXPECT_THAT(context->GetSharedAt<Obj>("first").get(), testing::Eq(replacement));

        EXPECT_CALL(mock, Deconstructing(replacement));
        EXPECT_CALL(mock, Deconstructing(third));
        EXPECT_CALL(mock, Deconstructing(second));

        context.reset();
    }

    TEST_P(TestContextOrdering, ClearAndReinsertBecomesNewestInsertion)
    {
        testing::StrictMock<Mock> mock;
        auto context = std::make_unique<Context>(CreateFactory());

        const auto* first = context->EmplaceAt<Obj>("first", &mock).get();
        const auto* second = context->EmplaceAt<Obj>("second", &mock).get();
        const auto* third = context->EmplaceAt<Obj>("third", &mock).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(second));
        context->Clear("second");
        EXPECT_FALSE(context->Contains("second"));
        context->Clear("missing");
        context->Clear("second");

        const auto* replacement = context->EmplaceAt<Obj>("second", &mock).get();
        EXPECT_CALL(mock, Deconstructing(replacement));
        EXPECT_CALL(mock, Deconstructing(third));
        EXPECT_CALL(mock, Deconstructing(first));

        context.reset();
    }

    TEST_P(TestContextOrdering, ChildRemovalAndDestructionLeaveParentUntouched)
    {
        testing::StrictMock<Mock> mock;
        auto parent = std::make_unique<Context>(CreateFactory());
        auto child = std::make_unique<Context>(parent.get());

        const auto* parentFirst = parent->EmplaceAt<Obj>("first", &mock).get();
        const auto* parentSecond = parent->EmplaceAt<Obj>("second", &mock).get();
        const auto* childFirst = child->EmplaceAt<Obj>("first", &mock).get();
        const auto* childSecond = child->EmplaceAt<Obj>("local", &mock).get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(childFirst));
        child->Clear("first");
        child->Clear("second");
        EXPECT_THAT(child->GetSharedAt<Obj>("first").get(), testing::Eq(parentFirst));
        EXPECT_THAT(child->GetSharedAt<Obj>("second").get(), testing::Eq(parentSecond));

        EXPECT_CALL(mock, Deconstructing(childSecond));
        child.reset();
        EXPECT_THAT(parent->GetSharedAt<Obj>("first").get(), testing::Eq(parentFirst));
        EXPECT_CALL(mock, Deconstructing(parentSecond));
        EXPECT_CALL(mock, Deconstructing(parentFirst));
        parent.reset();
    }

    TEST_P(TestContextOrdering, ExternalOwnerDelaysObjectDestruction)
    {
        testing::StrictMock<Mock> mock;
        auto context = std::make_unique<Context>(CreateFactory());

        const auto* first = context->EmplaceAt<Obj>("first", &mock).get();
        auto retained = context->EmplaceAt<Obj>("second", &mock);

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(first));
        context.reset();
        EXPECT_THAT(retained.use_count(), testing::Eq(1));
        EXPECT_CALL(mock, Deconstructing(retained.get()));
        retained.reset();
    }

    TEST_P(TestContextOrdering, OlderEntriesRemainAvailableDuringDestruction)
    {
        testing::StrictMock<Mock> mock;
        auto context = std::make_unique<Context>(CreateFactory());
        const auto* first = context->EmplaceAt<Obj>("first", &mock).get();
        const auto* second = context->EmplaceAt<Obj>("second", &mock).get();
        auto* activeContext = context.get();

        testing::InSequence seq;
        EXPECT_CALL(mock, Deconstructing(second)).WillOnce([&](const Obj*)
            {
                EXPECT_FALSE(activeContext->Contains("second"));
                EXPECT_THAT(activeContext->GetSharedAt<Obj>("first").get(), testing::Eq(first));
            });
        EXPECT_CALL(mock, Deconstructing(first));

        context.reset();
    }

    TEST_P(TestContextOrdering, OrderingSurvivesRehashing)
    {
        struct Ordered
        {
            Ordered(std::vector<int>& destroyed, int id)
                : destroyed{ destroyed }
                , id{ id }
            {}

            ~Ordered()
            {
                destroyed.push_back(id);
            }

            std::vector<int>& destroyed;
            int id;
        };

        constexpr int count = 1000;
        std::vector<int> destroyed;
        auto context = std::make_unique<Context>(CreateFactory());

        for (int id = 0; id != count; ++id)
        {
            context->EmplaceAt<Ordered>(std::to_string(id), destroyed, id);
        }

        for (int id = 0; id != count; ++id)
        {
            EXPECT_THAT(context->GetAt<Ordered>(std::to_string(id)).id, testing::Eq(id));
        }

        context->Clear("250");
        context->EmplaceAt<Ordered>("100", destroyed, count);
        context->EmplaceAt<Ordered>("250", destroyed, count + 1);
        context.reset();

        std::vector<int> expected{ 250, 100, count + 1, count };
        for (int id = count - 1; id >= 0; --id)
        {
            if (id != 250 && id != 100)
            {
                expected.push_back(id);
            }
        }

        EXPECT_THAT(destroyed, testing::ElementsAreArray(expected));
    }

    namespace
    {
        struct Released
        {
            explicit Released(int& count)
                : count{ count }
            {}

            ~Released()
            {
                ++count;
            }

            int& count;
        };

        // Inserts 64 entries into the context from its destructor.
        struct Grower
        {
            Grower(Context& context, int& released)
                : context{ context }
                , released{ released }
            {}

            ~Grower()
            {
                for (int id = 0; id != 64; ++id)
                {
                    context.EmplaceAt<Released>("late-" + std::to_string(id), released);
                }
            }

            Context& context;
            int& released;
        };
    }

    TEST_P(TestContextOrdering, ErasedValueMayGrowContext)
    {
        int released = 0;
        auto context = std::make_unique<Context>(CreateFactory());
        context->InsertAt("first", 1);
        context->EmplaceAt<Grower>("grower", *context, released);

        context->Clear("grower");

        EXPECT_FALSE(context->Contains("grower"));
        EXPECT_THAT(context->GetAt<int>("first"), testing::Eq(1));
        EXPECT_TRUE(context->Contains("late-0"));
        EXPECT_TRUE(context->Contains("late-63"));
        EXPECT_THAT(released, testing::Eq(0));

        context.reset();
        EXPECT_THAT(released, testing::Eq(64));
    }

    TEST_P(TestContextOrdering, ReplacedValueMayGrowContext)
    {
        int released = 0;
        auto context = std::make_unique<Context>(CreateFactory());
        context->EmplaceAt<Grower>("grower", *context, released);

        context->InsertAt("grower", 2);

        EXPECT_THAT(context->GetAt<int>("grower"), testing::Eq(2));
        EXPECT_TRUE(context->Contains("late-63"));

        context.reset();
        EXPECT_THAT(released, testing::Eq(64));
    }

    TEST_P(TestContextOrdering, ValueReleasedDuringDestructionMayGrowContext)
    {
        int released = 0;
        auto context = std::make_unique<Context>(CreateFactory());
        context->InsertAt("first", 1);
        context->EmplaceAt<Grower>("grower", *context, released);

        context.reset();

        EXPECT_THAT(released, testing::Eq(64));
    }

    INSTANTIATE_TEST_SUITE_P(StorageFactories, TestContextOrdering, testing::Bool(), [](const auto& info)
        {
            return info.param ? "ThreadSafe" : "Plain";
        });

    namespace
    {
        // Same storage-factory parameterization as TestContextOrdering.
        struct TestContextKeyLifetime : TestContextOrdering
        {};
    }

    // The context must own its keys: the caller's buffer is freed before the next lookup.
    TEST_P(TestContextKeyLifetime, StoredKeyDoesNotDependOnCallersBuffer)
    {
        Context context{ CreateFactory() };
        auto key = std::make_unique<std::string>("a key longer than the small string buffer");

        context.InsertAt(*key, 42);
        EXPECT_TRUE(context.Contains(*key));
        key.reset();

        EXPECT_THAT(context.GetAt<int>("a key longer than the small string buffer"), testing::Eq(42));
    }

    TEST_P(TestContextKeyLifetime, LookupWithTemporaryKey)
    {
        Context parent{ CreateFactory() };
        Context child{ &parent };
        parent.InsertAt("a key longer than the small string buffer", 7);

        EXPECT_THAT(child.GetAt<int>(std::string{ "a key longer than the small string buffer" }), testing::Eq(7));
        EXPECT_TRUE(child.Contains(std::string{ "a key longer than the small string buffer" }));

        parent.Clear(std::string{ "a key longer than the small string buffer" });
        EXPECT_FALSE(child.Contains("a key longer than the small string buffer"));
    }

    TEST_P(TestContextKeyLifetime, LookupWithKeyThatIsNotNullTerminated)
    {
        constexpr std::string_view buffer{ "firstmissing!" };
        const auto first = buffer.substr(0, 5);
        const auto missing = buffer.substr(5, 7);

        Context context{ CreateFactory() };
        context.InsertAt("first", 1);

        EXPECT_THAT(context.GetAt<int>(first), testing::Eq(1));
        EXPECT_FALSE(context.Contains(missing));
        EXPECT_THAT([&]
            {
                (void)context.GetAt<int>(missing);
            },
            testing::ThrowsMessage<std::out_of_range>(testing::StrEq("missing")));

        context.Clear(first);
        EXPECT_FALSE(context.Contains("first"));
    }

    // The key refers to the value that Clear destroys, so Erase must not read it afterwards.
    TEST_P(TestContextKeyLifetime, KeyMayAliasTheErasedValue)
    {
        Context context{ CreateFactory() };
        const std::string key{ "a key longer than the small string buffer" };
        context.InsertAt(key, key);

        context.Clear(context.GetAt<std::string>(key));

        EXPECT_FALSE(context.Contains(key));
    }

    INSTANTIATE_TEST_SUITE_P(StorageFactories, TestContextKeyLifetime, testing::Bool(), [](const auto& info)
        {
            return info.param ? "ThreadSafe" : "Plain";
        });
}
