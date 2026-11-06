#ifndef CUCUMBER_CPP_CONTEXT_HPP
#define CUCUMBER_CPP_CONTEXT_HPP

// IWYU pragma: private, include "cucumber_cpp/Steps.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace cucumber_cpp::library::context
{
    namespace detail
    {
        struct ContextStorage
        {
            struct KeyNotFound : std::out_of_range
            {
                using std::out_of_range::out_of_range;
            };

            virtual ~ContextStorage() = default;

            [[nodiscard]] virtual bool Contains(const std::string& key) const = 0;

            [[nodiscard]] virtual std::shared_ptr<void> Get(const std::string& key) const = 0;

            virtual void Erase(const std::string& key) = 0;

            virtual std::shared_ptr<void> Assign(std::string key, std::shared_ptr<void> value) = 0;

            virtual void Clear() = 0;
        };

        struct ContextStorageImpl : ContextStorage
        {
            ContextStorageImpl() = default;

            explicit ContextStorageImpl(ContextStorage& parent);

            ContextStorageImpl(const ContextStorageImpl&) = delete;
            ContextStorageImpl& operator=(const ContextStorageImpl&) = delete;
            ~ContextStorageImpl() override;

            [[nodiscard]] bool Contains(const std::string& key) const override;
            [[nodiscard]] std::shared_ptr<void> Get(const std::string& key) const override;
            void Erase(const std::string& key) override;
            std::shared_ptr<void> Assign(std::string key, std::shared_ptr<void> value) override;

            void Clear() override;

        private:
            struct Entry
            {
                const std::string* key;
                std::shared_ptr<void> value;
            };

            ContextStorage* parent{ nullptr };

            std::list<Entry> orderedEntries;
            std::unordered_map<std::string, std::list<Entry>::iterator> contextMap;
        };

        struct ContextStorageDecorator : ContextStorage
        {
            explicit ContextStorageDecorator(std::unique_ptr<ContextStorage> contextStorage);

            [[nodiscard]] bool Contains(const std::string& key) const override;
            [[nodiscard]] std::shared_ptr<void> Get(const std::string& key) const override;
            void Erase(const std::string& key) override;
            std::shared_ptr<void> Assign(std::string key, std::shared_ptr<void> value) override;

            void Clear() override;

        private:
            std::unique_ptr<ContextStorage> contextStorage;
        };

        struct ThreadSafeContextStorage : ContextStorageDecorator
        {
            explicit ThreadSafeContextStorage(std::unique_ptr<ContextStorage> contextStorage);

            [[nodiscard]] bool Contains(const std::string& key) const override;
            [[nodiscard]] std::shared_ptr<void> Get(const std::string& key) const override;
            void Erase(const std::string& key) override;
            std::shared_ptr<void> Assign(std::string key, std::shared_ptr<void> value) override;

            void Clear() override;

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

        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create() const override;
        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create(detail::ContextStorage& parent) const override;
    };

    struct ContextStorageFactoryThreadSafe : ContextStorageFactoryImpl
    {
        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create() const override;
        [[nodiscard]] std::unique_ptr<detail::ContextStorage> Create(detail::ContextStorage& parent) const override;
    };

    // Releases local values in reverse insertion order; overwrites count as new insertions.
    // External shared owners may delay actual destruction beyond the Context's lifetime.
    struct Context
    {
        explicit Context(std::shared_ptr<ContextStorageFactory> contextStorageFactory);
        explicit Context(const Context* parent);

        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        Context(Context&&) = default;
        Context& operator=(Context&&) = default;

        template<class T>
        void SetShared(const std::shared_ptr<T>& value);
        template<class T>
        void SetShared(std::string key, const std::shared_ptr<T>& value);

        template<class T, class... Args>
        std::shared_ptr<T> Emplace(Args&&... args);
        template<class T, class U, class... Args>
        std::shared_ptr<T> EmplaceAs(Args&&... args);
        template<class T, class... Args>
        std::shared_ptr<T> EmplaceAt(std::string key, Args&&... args);

        template<class T>
        void InsertRef(T& ref);
        template<class T, class U>
        void InsertRefAs(U& ref);
        template<class T>
        void InsertRefAt(std::string key, T& ref);

        template<class T>
        void Insert(const T& value);
        template<class T, class U>
        void InsertAs(const U& value);
        template<class T>
        void InsertAt(std::string key, const T& value);

        template<class T>
        [[nodiscard]] bool Contains() const;
        [[nodiscard]] bool Contains(const std::string& key) const;

        template<class T>
        void Clear();
        void Clear(const std::string& key);

        template<class T>
        [[nodiscard]] std::shared_ptr<T> GetShared() const;
        template<class T>
        [[nodiscard]] std::shared_ptr<T> GetShared(const std::string& key) const;

        template<class T>
        [[nodiscard]] T& Get() const;
        template<class T>
        [[nodiscard]] T& Get(const std::string& key) const;

    private:
        std::shared_ptr<ContextStorageFactory> contextStorageFactory;
        std::unique_ptr<detail::ContextStorage> storage{ contextStorageFactory->Create() };
    };

    template<class T>
    void Context::SetShared(const std::shared_ptr<T>& value)
    {
        SetShared(typeid(T).name(), value);
    }

    template<class T>
    void Context::SetShared(std::string key, const std::shared_ptr<T>& value)
    {
        storage->Assign(std::move(key), value);
    }

    template<class T, class... Args>
    std::shared_ptr<T> Context::Emplace(Args&&... args)
    {
        return EmplaceAt<T, Args...>(typeid(T).name(), std::forward<Args>(args)...);
    }

    template<class T, class U, class... Args>
    std::shared_ptr<T> Context::EmplaceAs(Args&&... args)
    {
        return EmplaceAt<U, Args...>(typeid(T).name(), std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    std::shared_ptr<T> Context::EmplaceAt(std::string key, Args&&... args)
    {
        return std::static_pointer_cast<T>(storage->Assign(std::move(key), std::make_shared<T>(std::forward<Args>(args)...)));
    }

    template<class T>
    void Context::InsertRef(T& ref)
    {
        InsertRefAs<T, T>(ref);
    }

    template<class T, class U>
    void Context::InsertRefAs(U& ref)
    {
        InsertRefAt<U>(typeid(T).name(), ref);
    }

    template<class T>
    void Context::InsertRefAt(std::string key, T& ref)
    {
        storage->Assign(std::move(key), std::shared_ptr<T>(&ref, [](auto) { /* lifetime managed by caller */ }));
    }

    template<class T>
    void Context::Insert(const T& value)
    {
        InsertAs<T, T>(value);
    }

    template<class T, class U>
    void Context::InsertAs(const U& value)
    {
        InsertAt<U>(typeid(T).name(), value);
    }

    template<class T>
    void Context::InsertAt(std::string key, const T& value)
    {
        storage->Assign(std::move(key), std::make_shared<T>(value));
    }

    template<class T>
    [[nodiscard]] bool Context::Contains() const
    {
        return Contains(typeid(T).name());
    }

    template<class T>
    void Context::Clear()
    {
        Clear(typeid(T).name());
    }

    template<class T>
    [[nodiscard]] std::shared_ptr<T> Context::GetShared() const
    {
        return GetShared<T>(typeid(T).name());
    }

    template<class T>
    [[nodiscard]] std::shared_ptr<T> Context::GetShared(const std::string& key) const
    {
        return std::static_pointer_cast<T>(storage->Get(key));
    }

    template<class T>
    [[nodiscard]] T& Context::Get() const
    {
        return Get<T>(typeid(T).name());
    }

    template<class T>
    [[nodiscard]] T& Context::Get(const std::string& key) const
    {
        return *GetShared<T>(key);
    }
}

#endif
