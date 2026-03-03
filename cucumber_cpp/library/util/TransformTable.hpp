#ifndef UTIL_TRANSFORM_TABLE_HPP
#define UTIL_TRANSFORM_TABLE_HPP

#include "cucumber/messages/pickle_table.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<Table> TransformTable(const std::optional<cucumber::messages::pickle_table>& pickleTable);
    std::optional<cucumber::messages::pickle_table> TransformTable(const std::optional<Table>& table);
}

#endif
