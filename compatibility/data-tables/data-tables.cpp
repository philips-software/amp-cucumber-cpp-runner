#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <gmock/gmock.h>
#include <span>
#include <vector>

STEP(R"(the following table is transposed:)")
{
    std::vector<cucumber::messages::pickle_table_row> transposedTable;
    transposedTable.reserve(this->table->front().cells.size());
    for (std::size_t colIdx = 0; colIdx < this->table->front().cells.size(); ++colIdx)
        transposedTable.emplace_back().cells.resize(this->table->size());

    for (std::size_t rowIdx = 0; rowIdx < this->table->size(); ++rowIdx)
        for (std::size_t colIdx = 0; colIdx < this->table->begin()[rowIdx].cells.size(); ++colIdx)
            transposedTable[colIdx].cells[rowIdx] = this->table->begin()[rowIdx].cells[colIdx];

    context.Insert(transposedTable);
}

STEP(R"(it should be:)")
{
    std::span<const cucumber::messages::pickle_table_row> expected = context.Get<std::vector<cucumber::messages::pickle_table_row>>();
    const auto& actual = *table;
    const auto rows = expected.size();
    ASSERT_THAT(rows, testing::Eq(actual.size()));
    for (auto rowIdx = 0; rowIdx < rows; ++rowIdx)
    {
        const auto columns = expected[rowIdx].cells.size();
        ASSERT_THAT(columns, testing::Eq(actual[rowIdx].cells.size()));
        for (auto colIdx = 0; colIdx < columns; ++colIdx)
        {
            const auto& expectedCell = expected[rowIdx].cells[colIdx];
            const auto& actualCell = actual[rowIdx].cells[colIdx];
            EXPECT_THAT(expectedCell.value, testing::StrEq(actualCell.value)) << "at row " << rowIdx << " column " << colIdx;
        }
    }
}

// import assert from 'node:assert'
// import { DataTable, When, Then } from '@cucumber/fake-cucumber'

// When('the following table is transposed:', function (table: DataTable) {
//   this.transposed = table.transpose()
// })

// Then('it should be:', function (expected: DataTable) {
//   assert.deepStrictEqual(this.transposed.raw(), expected.raw())
// }
