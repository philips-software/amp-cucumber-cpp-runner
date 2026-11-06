#include "cucumber_cpp/library/context/Context.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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

    TEST_F(TestContext, OverwriteValue)
    {
        Mock mock;

        auto first = programContext.Emplace<Obj>(&mock).get();
        EXPECT_CALL(mock, Deconstructing(first));

        auto second = programContext.Emplace<Obj>().get();

        EXPECT_THAT(first, testing::Not(testing::Pointer(second)));
    }

    TEST_F(TestContext, OverrideValue)
    {
        auto first = programContext.Emplace<Obj>().get();
        auto second = scenarioContext.Emplace<Obj>().get();

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

        const auto& key1 = scenarioContext.Get<Derived>("key1");
        EXPECT_THAT(key1.a, testing::StrEq("a"));
        EXPECT_THAT(key1.b, testing::Eq(2));

        const auto& key2 = scenarioContext.Get<Derived>("key2");
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

        EXPECT_THAT(&scenarioContext.Get<NonCopyable>("key1"), testing::Pointer(&nonCopyable));
        EXPECT_THAT(&scenarioContext.Get<NonCopyable>("key2"), testing::Pointer(&nonCopyable));
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

        EXPECT_THAT(&scenarioContext.Get<Obj>("key1"), testing::Not(testing::Pointer(&obj)));
        EXPECT_THAT(&scenarioContext.Get<Obj>("key2"), testing::Not(testing::Pointer(&obj)));
        EXPECT_THAT(&scenarioContext.Get<Obj>("key1"), testing::Not(testing::Pointer(&scenarioContext.Get<Obj>("key2"))));
    }

    TEST_F(TestContext, KeyNotFound)
    {
        scenarioContext.Emplace<Obj>();

        EXPECT_THROW((void)programContext.Get<Obj>(), std::out_of_range);
        EXPECT_THROW((void)scenarioContext.Get<Base>(), std::out_of_range);
        EXPECT_THROW((void)scenarioContext.Get<int>("mykey"), std::out_of_range);
    }

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
        EXPECT_THAT(context->GetShared<Obj>("first").get(), testing::Eq(replacement));

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
        EXPECT_THAT(child->GetShared<Obj>("first").get(), testing::Eq(parentFirst));
        EXPECT_THAT(child->GetShared<Obj>("second").get(), testing::Eq(parentSecond));

        EXPECT_CALL(mock, Deconstructing(childSecond));
        child.reset();
        EXPECT_THAT(parent->GetShared<Obj>("first").get(), testing::Eq(parentFirst));
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
                EXPECT_THAT(activeContext->GetShared<Obj>("first").get(), testing::Eq(first));
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
            EXPECT_THAT(context->Get<Ordered>(std::to_string(id)).id, testing::Eq(id));
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

    INSTANTIATE_TEST_SUITE_P(StorageFactories, TestContextOrdering, testing::Bool(), [](const auto& info)
        {
            return info.param ? "ThreadSafe" : "Plain";
        });
}
