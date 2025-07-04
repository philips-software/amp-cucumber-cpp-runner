#ifndef CUCUMBER_CPP_CONTEXT_HPP
#define CUCUMBER_CPP_CONTEXT_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include <algorithm>
#include <forward_list>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>

namespace cucumber_cpp::library
{
    namespace detail
    {
        struct ContextStorage
        {
            struct KeyNotFound : std::out_of_range
            {
                using std::out_of_range::out_of_range;
            };

            struct Entry
            {
                Entry(std::string_view key,
                    const std::shared_ptr<void>& value)
                    : key{ key }
                    , value{ value }
                {}

                std::string key;
                std::shared_ptr<void> value;
            };

            virtual ~ContextStorage() = default;

            [[nodiscard]] virtual bool Contains(std::string_view key) const = 0;

            [[nodiscard]] virtual std::shared_ptr<void> Get(std::string_view key) const = 0;

            [[nodiscard]] virtual std::forward_list<Entry>::const_iterator Find(std::string_view key) const = 0;

            virtual void Erase(std::string_view key) = 0;

            virtual const std::shared_ptr<void>& Assign(std::string_view key, const std::shared_ptr<void>& value) = 0;
        };

        struct ContextStorageImpl : ContextStorage
        {
            ContextStorageImpl() = default;

            explicit ContextStorageImpl(ContextStorage& parent)
                : parent{ &parent }
            {}

            [[nodiscard]] bool Contains(std::string_view key) const override
            {
                if (const auto iter = Find(key); iter != context.end())
                {
                    return true;
                }

                if (parent != nullptr)
                {
                    return parent->Contains(key);
                }

                return false;
            }

            [[nodiscard]] std::shared_ptr<void> Get(std::string_view key) const override
            {
                if (const auto iter = Find(key); iter != context.end())
                {
                    return iter->value;
                }
                else if (parent != nullptr)
                {
                    return parent->Get(key);
                }
                else
                {
                    std::cerr << "key not found: \"" << key << "\"\n";
                    throw KeyNotFound{ std::string{ key } };
                }
            }

            [[nodiscard]] std::forward_list<Entry>::const_iterator Find(std::string_view key) const override
            {
                return std::ranges::find(context, key, &Entry::key);
            }

            void Erase(std::string_view key) override
            {
                std::erase_if(context, [&key](const Entry& pair)
                    {
                        return pair.key == key;
                    });
            }

            const std::shared_ptr<void>& Assign(std::string_view key, const std::shared_ptr<void>& value) override
            {
                Erase(key);

                context.emplace_front(key, value);

                return value;
            }

        private:
            ContextStorage* parent{ nullptr };
            std::forward_list<Entry> context;
        };

        struct ContextStorageDecorator : ContextStorage
        {
            explicit ContextStorageDecorator(std::unique_ptr<ContextStorage> contextStorage)
                : contextStorage{ std::move(contextStorage) }
            {}

            [[nodiscard]] bool Contains(std::string_view key) const override
            {
                return contextStorage->Contains(key);
            }

            [[nodiscard]] std::shared_ptr<void> Get(std::string_view key) const override
            {
                return contextStorage->Get(key);
            }

            [[nodiscard]] std::forward_list<Entry>::const_iterator Find(std::string_view key) const override
            {
                return contextStorage->Find(key);
            }

            void Erase(std::string_view key) override
            {
                contextStorage->Erase(key);
            }

            const std::shared_ptr<void>& Assign(std::string_view key, const std::shared_ptr<void>& value) override
            {
                return contextStorage->Assign(key, value);
            }

        private:
            std::unique_ptr<ContextStorage> contextStorage;
        };

        struct ThreadSafeContextStorage : ContextStorageDecorator
        {
            explicit ThreadSafeContextStorage(std::unique_ptr<ContextStorage> contextStorage)
                : ContextStorageDecorator{ std::move(contextStorage) }
            {}

            [[nodiscard]] bool Contains(std::string_view key) const override
            {
                std::lock_guard lock{ mutex };
                return ContextStorageDecorator::Contains(key);
            }

            [[nodiscard]] std::shared_ptr<void> Get(std::string_view key) const override
            {
                std::lock_guard lock{ mutex };
                return ContextStorageDecorator::Get(key);
            }

            [[nodiscard]] std::forward_list<Entry>::const_iterator Find(std::string_view key) const override
            {
                std::lock_guard lock{ mutex };
                return ContextStorageDecorator::Find(key);
            }

            void Erase(std::string_view key) override
            {
                std::lock_guard lock{ mutex };
                ContextStorageDecorator::Erase(key);
            }

            const std::shared_ptr<void>& Assign(std::string_view key, const std::shared_ptr<void>& value) override
            {
                std::lock_guard lock{ mutex };
                return ContextStorageDecorator::Assign(key, value);
            }

        private:
            mutable std::recursive_mutex mutex;
        };

    }

    struct ContextStorageFactory
    {
    protected:
        ~ContextStorageFactory() = default;

    public:
        [[nodiscard]] virtual std::unique_ptr<detail::ContextStorage> Create() const = 0;
        [[nodiscard]] virtual std::unique_ptr<detail::ContextStorage> Create(detail::ContextStorage& parent) const = 0;
    };

    struct ContextStorageFactoryImpl : ContextStorageFactory
    {
        virtual ~ContextStorageFactoryImpl() = default;

        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create() const override
        {
            return std::make_unique<detail::ContextStorageImpl>();
        }

        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create(detail::ContextStorage& parent) const override
        {
            return std::make_unique<detail::ContextStorageImpl>(parent);
        }
    };

    struct ContextStorageFactoryThreadSafe : ContextStorageFactoryImpl
    {
        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create() const override
        {
            return std::make_unique<detail::ThreadSafeContextStorage>(ContextStorageFactoryImpl::Create());
        }

        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create(detail::ContextStorage& parent) const override
        {
            return std::make_unique<detail::ThreadSafeContextStorage>(ContextStorageFactoryImpl::Create(parent));
        }
    };

    struct Context
    {
        explicit Context(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
            : contextStorageFactory{ std::move(contextStorageFactory) }
        {}

        explicit Context(const Context* parent)
            : contextStorageFactory{ parent->contextStorageFactory }
            , storage{ contextStorageFactory->Create(*parent->storage) }
        {}

        template<class T>
        void SetShared(const std::shared_ptr<T>& value)
        {
            SetShared(typeid(T).name(), value);
        }

        template<class T>
        void SetShared(std::string_view key, const std::shared_ptr<T>& value)
        {
            storage->Assign(key, value);
        }

        template<class T, class... Args>
        std::shared_ptr<T> Emplace(Args&&... args)
        {
            return EmplaceAt<T, Args...>(typeid(T).name(), std::forward<Args>(args)...);
        }

        template<class T, class U, class... Args>
        std::shared_ptr<T> EmplaceAs(Args&&... args)
        {
            return EmplaceAt<U, Args...>(typeid(T).name(), std::forward<Args>(args)...);
        }

        template<class T, class... Args>
        std::shared_ptr<T> EmplaceAt(std::string_view key, Args&&... args)
        {
            return std::static_pointer_cast<T>(storage->Assign(key, std::make_shared<T>(std::forward<Args>(args)...)));
        }

        template<class T>
        void InsertRef(T& ref)
        {
            InsertRefAs<T, T>(ref);
        }

        template<class T, class U>
        void InsertRefAs(U& ref)
        {
            InsertRefAt<U>(typeid(T).name(), ref);
        }

        template<class T>
        void InsertRefAt(std::string_view key, T& ref)
        {
            storage->Assign(key, std::shared_ptr<T>(&ref, [](auto) { /* lifetime managed by caller */ }));
        }

        template<class T>
        void Insert(const T& value)
        {
            InsertAs<T, T>(value);
        }

        template<class T, class U>
        void InsertAs(const U& value)
        {
            InsertAt<U>(typeid(T).name(), value);
        }

        template<class T>
        void InsertAt(std::string_view key, const T& value)
        {
            storage->Assign(key, std::make_shared<T>(value));
        }

        template<class T>
        [[nodiscard]] bool Contains() const
        {
            return Contains(typeid(T).name());
        }

        [[nodiscard]] bool Contains(std::string_view key) const
        {
            return storage->Contains(key);
        }

        template<class T>
        void Clear()
        {
            Clear(typeid(T).name());
        }

        void Clear(std::string_view key)
        {
            storage->Erase(key);
        }

        template<class T>
        [[nodiscard]] std::shared_ptr<T> GetShared(std::string_view key = typeid(T).name())
        {
            return std::static_pointer_cast<T>(storage->Get(key));
        }

        template<class T>
        [[nodiscard]] T& Get(std::string_view key = typeid(T).name())
        {
            return *GetShared<T>(key);
        }

    private:
        std::shared_ptr<ContextStorageFactory> contextStorageFactory;
        std::unique_ptr<detail::ContextStorage> storage{ contextStorageFactory->Create() };
    };
}

#endif
