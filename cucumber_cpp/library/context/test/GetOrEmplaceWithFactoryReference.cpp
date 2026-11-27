#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrEmplaceWithFactoryReference(cucumber_cpp::library::context::Context& context)
    {
        int value{ 42 };
        static_cast<void>(context.GetOrEmplaceWith<int>([&value]() -> int& {
            return value;
        }));
    }
}
