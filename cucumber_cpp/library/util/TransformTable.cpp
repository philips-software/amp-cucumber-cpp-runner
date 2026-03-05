#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/pickle_table_cell.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<Table> TransformTable(const std::optional<cucumber::messages::pickle_table>& pickleTable)
    {
        if (!pickleTable.has_value())
            return std::nullopt;

        Table table;

        for (const auto& pickleTableRow : pickleTable->rows)
        {
            TableRow& tableRow = table.rows.emplace_back();
            tableRow.cells.reserve(pickleTableRow.cells.size());

            for (const auto& cell : pickleTableRow.cells)
                tableRow.cells.emplace_back(cell.value);
        }

        return table;
    }

    std::optional<cucumber::messages::pickle_table> TransformTable(const std::optional<Table>& table)
    {
        if (!table.has_value())
            return std::nullopt;

        cucumber::messages::pickle_table pickleTable;

        for (const auto& tableRow : table->rows)
        {
            auto& pickleTableRow = pickleTable.rows.emplace_back();
            pickleTableRow.cells.reserve(tableRow.cells.size());

            for (const auto& cell : tableRow.cells)
                pickleTableRow.cells.emplace_back(cucumber::messages::pickle_table_cell{ .value = cell.value });
        }

        return pickleTable;
    }
}
