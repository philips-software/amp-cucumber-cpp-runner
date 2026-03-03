#ifndef UTIL_TABLE_HPP
#define UTIL_TABLE_HPP

#include <string>
#include <vector>

namespace cucumber_cpp::library::util
{
    struct TableCell
    {
        std::string value;
    };

    struct TableRow
    {
        std::vector<TableCell> cells;
    };

    struct Table
    {
        std::vector<TableRow> rows;
    };
}

#endif
