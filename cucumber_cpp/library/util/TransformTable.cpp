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
            tableRow.cells.reserve(pickleTableRow->cells.size());

            for (const auto& cell : pickleTableRow->cells)
                tableRow.cells.emplace_back(cell->value);
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
            const auto& pickleTableRow = pickleTable.rows.emplace_back(std::make_shared<cucumber::messages::PickleTableRow>());
            pickleTableRow->cells.reserve(tableRow.cells.size());

            for (const auto& cell : tableRow.cells)
                pickleTableRow->cells.emplace_back(std::make_shared<cucumber::messages::PickleTableCell>(cucumber::messages::PickleTableCell{ .value = cell.value }));
        }

        return pickleTable;
    }
}
