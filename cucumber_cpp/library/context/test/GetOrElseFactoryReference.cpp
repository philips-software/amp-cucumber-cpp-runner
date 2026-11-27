#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrElseFactoryReference(const cucumber_cpp::library::context::Context& context)
    {
        int value{ 42 };
        static_cast<void>(context.GetOrElse<int>([&value]() -> int& {
            return value;
        }));
    }
}
