#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    [[maybe_unused]] void GetOrElseFactoryConvertible(const cucumber_cpp::library::context::Context& context)
    {
        static_cast<void>(context.GetOrElse<int>([] {
            return short{ 42 };
        }));
    }
}
