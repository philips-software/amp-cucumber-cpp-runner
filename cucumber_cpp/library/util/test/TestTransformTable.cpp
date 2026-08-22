#include "cucumber/messages/PickleTable.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include <gmock/gmock.h>
#include <optional>

namespace cucumber_cpp::library::util
{
    using testing::Eq;
    using testing::IsFalse;
    using testing::IsTrue;
    using testing::SizeIs;

    TEST(TransformTable, returns_nullopt_for_empty_input)
    {
        EXPECT_THAT(TransformTable(std::optional<Table>{}).has_value(), IsFalse());
        EXPECT_THAT(TransformTable(std::optional<cucumber::messages::PickleTable>{}).has_value(), IsFalse());
    }

    TEST(TransformTable, roundtrips_rows_and_cells)
    {
        Table table;
        auto& row = table.rows.emplace_back();
        row.cells.push_back(TableCell{ .value = "a" });
        row.cells.push_back(TableCell{ .value = "b" });

        const std::optional<cucumber::messages::PickleTable> pickle = TransformTable(std::optional<Table>{ table });

        ASSERT_THAT(pickle.has_value(), IsTrue());
        ASSERT_THAT(pickle->rows, SizeIs(1));
        ASSERT_THAT(pickle->rows[0]->cells, SizeIs(2));
        EXPECT_THAT(pickle->rows[0]->cells[0]->value, Eq("a"));
        EXPECT_THAT(pickle->rows[0]->cells[1]->value, Eq("b"));

        const std::optional<Table> back = TransformTable(pickle);

        ASSERT_THAT(back.has_value(), IsTrue());
        ASSERT_THAT(back->rows, SizeIs(1));
        ASSERT_THAT(back->rows[0].cells, SizeIs(2));
        EXPECT_THAT(back->rows[0].cells[0].value, Eq("a"));
        EXPECT_THAT(back->rows[0].cells[1].value, Eq("b"));
    }
}
