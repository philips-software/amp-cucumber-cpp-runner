#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrElseAtFactoryConvertible(const cucumber_cpp::library::context::Context& context)
    {
        static_cast<void>(context.GetOrElseAt<int>("key", [] {
            return short{ 42 };
        }));
    }
}
