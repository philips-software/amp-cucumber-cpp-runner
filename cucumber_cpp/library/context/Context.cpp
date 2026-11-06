#include "cucumber_cpp/library/context/Context.hpp"
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
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

        bool ContextStorageImpl::Contains(const std::string& key) const
        {
            if (contextMap.contains(key))
            {
                return true;
            }

            if (parent != nullptr)
            {
                return parent->Contains(key);
            }

            return false;
        }

        std::shared_ptr<void> ContextStorageImpl::Get(const std::string& key) const
        {
            if (auto iter = contextMap.find(key); iter != contextMap.end())
            {
                return iter->second->value;
            }

            if (parent != nullptr)
            {
                return parent->Get(key);
            }

            std::cerr << "key not found: \"" << key << "\"\n";

            throw KeyNotFound{ key };
        }

        void ContextStorageImpl::Erase(const std::string& key)
        {
            if (auto iter = contextMap.find(key); iter != contextMap.end())
            {
                auto entry = iter->second;
                contextMap.erase(iter); // removes the key object, don't use entry->key after this
                orderedEntries.erase(entry);
            }
        }

        std::shared_ptr<void> ContextStorageImpl::Assign(std::string key, std::shared_ptr<void> value)
        {
            auto [iter, inserted] = contextMap.try_emplace(std::move(key));
            if (!inserted)
            {
                auto entry = iter->second;
                entry->value.swap(value);
                orderedEntries.splice(orderedEntries.end(), orderedEntries, entry);
                return entry->value;
            }

            try
            {
                iter->second = orderedEntries.insert(orderedEntries.end(), Entry{ &iter->first, value });
            }
            catch (...)
            {
                contextMap.erase(iter);
                throw;
            }
            return value;
        }

        void ContextStorageImpl::Clear()
        {
            while (!orderedEntries.empty())
                Erase(*orderedEntries.back().key);
        }

        ContextStorageDecorator::ContextStorageDecorator(std::unique_ptr<ContextStorage> contextStorage)
            : contextStorage{ std::move(contextStorage) }
        {}

        bool ContextStorageDecorator::Contains(const std::string& key) const
        {
            return contextStorage->Contains(key);
        }

        std::shared_ptr<void> ContextStorageDecorator::Get(const std::string& key) const
        {
            return contextStorage->Get(key);
        }

        void ContextStorageDecorator::Erase(const std::string& key)
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

        bool ThreadSafeContextStorage::Contains(const std::string& key) const
        {
            std::scoped_lock lock{ mutex };
            return ContextStorageDecorator::Contains(key);
        }

        std::shared_ptr<void> ThreadSafeContextStorage::Get(const std::string& key) const
        {
            std::scoped_lock lock{ mutex };
            return ContextStorageDecorator::Get(key);
        }

        void ThreadSafeContextStorage::Erase(const std::string& key)
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

    bool Context::Contains(const std::string& key) const
    {
        return storage->Contains(key);
    }

    void Context::Clear(const std::string& key)
    {
        storage->Erase(key);
    }
}
