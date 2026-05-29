#ifndef UTIL_ID_GENERATOR_HPP
#define UTIL_ID_GENERATOR_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace cucumber_cpp::library::util
{
#if defined(ENABLE_PARALLEL_SUPPORT)

    class ThreadSafeIdGenerator : public cucumber::gherkin::id_generator_base
    {
    public:
        std::string next_id() override
        {
            return std::to_string(id_counter_.fetch_add(1, std::memory_order_relaxed));
        }

    private:
        std::atomic<std::size_t> id_counter_{ 0 };
    };

#endif

    inline cucumber::gherkin::id_generator_ptr MakeIdGenerator()
    {
#if defined(ENABLE_PARALLEL_SUPPORT)
        return std::make_shared<ThreadSafeIdGenerator>();
#else
        return std::make_shared<cucumber::gherkin::id_generator>();
#endif
    }
}

#endif
