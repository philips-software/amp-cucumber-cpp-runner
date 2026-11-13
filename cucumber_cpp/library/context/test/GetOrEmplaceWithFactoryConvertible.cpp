#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrEmplaceWithFactoryConvertible(cucumber_cpp::library::context::Context& context)
    {
        static_cast<void>(context.GetOrEmplaceWith<int>([] {
            return short{ 42 };
        }));
    }
}
