#ifndef ENGINE_TABLE_HPP
#define ENGINE_TABLE_HPP

#include "cucumber_cpp/library/engine/StringTo.hpp"
#include <source_location>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    struct TableValue
    {
        template<class T>
        T As(std::source_location sourceLocation = std::source_location::current()) const
        {
            return StringTo<T>(value, sourceLocation);
        }

        explicit TableValue(std::string value)
            : value(std::move(value))
        {}

    private:
        std::string value;
    };

    using Table = std::vector<std::vector<TableValue>>;
}

#endif
