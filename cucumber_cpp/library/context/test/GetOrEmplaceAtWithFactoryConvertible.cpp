#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrEmplaceAtWithFactoryConvertible(cucumber_cpp::library::context::Context& context)
    {
        static_cast<void>(context.GetOrEmplaceAtWith<int>("key", [] {
            return short{ 42 };
        }));
    }
}
