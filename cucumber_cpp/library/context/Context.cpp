#include "cucumber_cpp/library/context/Context.hpp"
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace cucumber_cpp::library::context
{
    namespace detail
    {
        ContextStorageImpl::ContextStorageImpl(ContextStorage& parent)
            : parent{ &parent }
        {}

        ContextStorageImpl::~ContextStorageImpl()
        {
            Clear();
        }

        bool ContextStorageImpl::Contains(std::string_view key) const
        {
            if (Find(key) != entries.end())
            {
                return true;
            }

            if (parent != nullptr)
            {
                return parent->Contains(key);
            }

            return false;
        }

        std::shared_ptr<void> ContextStorageImpl::Get(std::string_view key) const
        {
            if (auto iter = Find(key); iter != entries.end())
            {
                return iter->value;
            }

            if (parent != nullptr)
            {
                return parent->Get(key);
            }

            std::cerr << "key not found: \"" << key << "\"\n";

            throw KeyNotFound{ std::string{ key } };
        }

        void ContextStorageImpl::Erase(std::string_view key)
        {
            if (auto iter = Find(key); iter != entries.end())
            {
                entries.splice(entries.end(), entries, iter); // makes it the entry ReleaseNewest releases
                ReleaseNewest();
            }
        }

        std::shared_ptr<void> ContextStorageImpl::Assign(std::string key, std::shared_ptr<void> value)
        {
            if (auto iter = Find(key); iter != entries.end())
            {
                entries.splice(entries.end(), entries, iter);
                entries.back().value.swap(value); // the replaced value is released on return
                return entries.back().value;
            }

            entries.push_back({ std::move(key), value });

            return value;
        }

        void ContextStorageImpl::Clear()
        {
            while (!entries.empty())
            {
                ReleaseNewest();
            }
        }

        ContextStorageImpl::Entries::const_iterator ContextStorageImpl::Find(std::string_view key) const
        {
            return std::ranges::find(entries, key, &Entry::key);
        }

        // Unlinks the newest entry before its value is released, so a destructor that calls back
        // into the context sees a consistent context.
        void ContextStorageImpl::ReleaseNewest()
        {
            const auto value = std::move(entries.back().value);
            entries.pop_back();
        }

        ContextStorageDecorator::ContextStorageDecorator(std::unique_ptr<ContextStorage> contextStorage)
            : contextStorage{ std::move(contextStorage) }
        {}

        bool ContextStorageDecorator::Contains(std::string_view key) const
        {
            return contextStorage->Contains(key);
        }

        std::shared_ptr<void> ContextStorageDecorator::Get(std::string_view key) const
        {
            return contextStorage->Get(key);
        }

        void ContextStorageDecorator::Erase(std::string_view key)
        {
            contextStorage->Erase(key);
        }

        std::shared_ptr<void> ContextStorageDecorator::Assign(std::string key, std::shared_ptr<void> value)
        {
            return contextStorage->Assign(std::move(key), value);
        }

        void ContextStorageDecorator::Clear()
        {
            contextStorage->Clear();
        }

        ThreadSafeContextStorage::ThreadSafeContextStorage(std::unique_ptr<ContextStorage> contextStorage)
            : ContextStorageDecorator{ std::move(contextStorage) }
        {}

        bool ThreadSafeContextStorage::Contains(std::string_view key) const
        {
            std::scoped_lock lock{ mutex };
            return ContextStorageDecorator::Contains(key);
        }

        std::shared_ptr<void> ThreadSafeContextStorage::Get(std::string_view key) const
        {
            std::scoped_lock lock{ mutex };
            return ContextStorageDecorator::Get(key);
        }

        void ThreadSafeContextStorage::Erase(std::string_view key)
        {
            std::scoped_lock lock{ mutex };
            ContextStorageDecorator::Erase(key);
        }

        std::shared_ptr<void> ThreadSafeContextStorage::Assign(std::string key, std::shared_ptr<void> value)
        {
            std::scoped_lock lock{ mutex };
            return ContextStorageDecorator::Assign(std::move(key), value);
        }

        void ThreadSafeContextStorage::Clear()
        {
            std::scoped_lock lock{ mutex };
            ContextStorageDecorator::Clear();
        }
    }

    std::unique_ptr<detail::ContextStorage> ContextStorageFactoryImpl::Create() const
    {
        return std::make_unique<detail::ContextStorageImpl>();
    }

    std::unique_ptr<detail::ContextStorage> ContextStorageFactoryImpl::Create(detail::ContextStorage& parent) const
    {
        return std::make_unique<detail::ContextStorageImpl>(parent);
    }

    std::unique_ptr<detail::ContextStorage> ContextStorageFactoryThreadSafe::Create() const
    {
        return std::make_unique<detail::ThreadSafeContextStorage>(ContextStorageFactoryImpl::Create());
    }

    std::unique_ptr<detail::ContextStorage> ContextStorageFactoryThreadSafe::Create(detail::ContextStorage& parent) const
    {
        return std::make_unique<detail::ThreadSafeContextStorage>(ContextStorageFactoryImpl::Create(parent));
    }

    Context::Context(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : contextStorageFactory{ std::move(contextStorageFactory) }
    {}

    Context::Context(const Context* parent)
        : contextStorageFactory{ parent->contextStorageFactory }
        , storage{ contextStorageFactory->Create(*parent->storage) }
    {}

    Context::~Context()
    {
        if (storage)
        {
            storage->Clear();
        }
    }

    bool Context::Contains(std::string_view key) const
    {
        return storage->Contains(key);
    }

    void Context::Clear(std::string_view key)
    {
        storage->Erase(key);
    }
}
