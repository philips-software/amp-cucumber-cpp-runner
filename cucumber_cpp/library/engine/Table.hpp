#ifndef ENGINE_TABLE_HPP
#define ENGINE_TABLE_HPP

#include "cucumber_cpp/library/engine/StringTo.hpp"
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct TableValue
    {
        template<class T>
        T As() const
        {
            return StringTo<T>(value);
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
