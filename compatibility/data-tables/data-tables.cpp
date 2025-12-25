#include "cucumber/messages/pickle_table.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

STEP(R"(the following table is transposed:)")
{
    auto transposedTable = context.Emplace<cucumber::messages::pickle_table>();
    transposedTable->rows.reserve(this->dataTable->rows[0].cells.size());
    for (std::size_t colIdx = 0; colIdx < this->dataTable->rows[0].cells.size(); ++colIdx)
        transposedTable->rows.emplace_back().cells.resize(this->dataTable->rows.size());

    for (std::size_t rowIdx = 0; rowIdx < this->dataTable->rows.size(); ++rowIdx)
        for (std::size_t colIdx = 0; colIdx < this->dataTable->rows[rowIdx].cells.size(); ++colIdx)
            transposedTable->rows[colIdx].cells[rowIdx] = this->dataTable->rows[rowIdx].cells[colIdx];
}

STEP(R"(it should be:)")
{
    const auto& actualTable = context.Get<cucumber::messages::pickle_table>();
    const auto& expectedTalbe = dataTable;

    const auto rows = actualTable.rows.size();
    ASSERT_THAT(rows, testing::Eq(expectedTalbe->rows.size()));

    for (auto rowIdx = 0; rowIdx < rows; ++rowIdx)
    {
        const auto columns = expectedTalbe->rows[rowIdx].cells.size();
        ASSERT_THAT(columns, testing::Eq(actualTable.rows[rowIdx].cells.size()));
        for (auto colIdx = 0; colIdx < columns; ++colIdx)
        {
            const auto& expectedCell = expectedTalbe->rows[rowIdx].cells[colIdx];
            const auto& actualCell = actualTable.rows[rowIdx].cells[colIdx];
            EXPECT_THAT(expectedCell.value, testing::StrEq(actualCell.value)) << "at row " << rowIdx << " column " << colIdx;
        }
    }
}
