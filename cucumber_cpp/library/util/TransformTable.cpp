#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber/messages/PickleTable.hpp"
#include "cucumber/messages/PickleTableCell.hpp"
#include "cucumber/messages/PickleTableRow.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include <memory>
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<Table> TransformTable(const std::optional<cucumber::messages::PickleTable>& pickleTable)
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

    std::optional<cucumber::messages::PickleTable> TransformTable(const std::optional<Table>& table)
    {
        if (!table.has_value())
            return std::nullopt;

        cucumber::messages::PickleTable pickleTable;

        for (const auto& tableRow : table->rows)
        {
            cucumber::messages::PickleTableRow& pickleTableRow = pickleTable.rows.emplace_back();
            pickleTableRow.cells.reserve(tableRow.cells.size());

            for (const auto& cell : tableRow.cells)
            {
                cucumber::messages::PickleTableCell pickleTableCell;
                pickleTableCell.value = cell.value;
                pickleTableRow.cells.push_back(pickleTableCell);
            }
        }

        return pickleTable;
    }
}
