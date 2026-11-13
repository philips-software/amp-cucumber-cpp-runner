#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    struct Base
    {
        virtual ~Base() = default;
    };

    struct Derived : Base
    {};

    [[maybe_unused]] void GetOrEmplaceAsWithFactoryReference(cucumber_cpp::library::context::Context& context)
    {
        Derived value;
        static_cast<void>(context.GetOrEmplaceAsWith<Base, Derived>([&value]() -> Derived& {
            return value;
        }));
    }
}
