#include "cucumber_cpp/library/context/Context.hpp"

namespace
{
    struct Base
    {
        virtual ~Base() = default;
    };

    struct Derived : Base
    {};

    struct ConvertibleToDerived
    {
        operator Derived() const;
    };

    [[maybe_unused]] void GetOrEmplaceAsWithFactoryConvertible(cucumber_cpp::library::context::Context& context)
    {
        static_cast<void>(context.GetOrEmplaceAsWith<Base, Derived>([] {
            return ConvertibleToDerived{};
        }));
    }
}
