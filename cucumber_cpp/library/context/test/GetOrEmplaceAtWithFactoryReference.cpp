#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrEmplaceAtWithFactoryReference(cucumber_cpp::library::context::Context& context)
    {
        int value{ 42 };
        static_cast<void>(context.GetOrEmplaceAtWith<int>("key", [&value]() -> int& {
            return value;
        }));
    }
}
