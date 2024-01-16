#include <muui/item.h>

#include <catch2/catch_test_macros.hpp>

using namespace muui;

TEST_CASE("Column layout", "[layouts]")
{
    Column column;
    REQUIRE(column.childCount() == 0);

    // add a rectangle
    {
        auto *r1 = column.appendChild<Rectangle>(80, 20);
        REQUIRE(r1->size() == Size{80, 20});
    }
    REQUIRE(column.childCount() == 1);
    REQUIRE(column.size() == Size{80, 20});

    // add another rectangle
    {
        auto *r2 = column.appendChild<Rectangle>(100, 10);
        REQUIRE(r2->size() == Size{100, 10});
    }
    REQUIRE(column.childCount() == 2);
    REQUIRE(column.size() == Size{100, 30});

    // change size of first rectangle
    auto *r1 = dynamic_cast<Rectangle *>(column.childAt(0));
    REQUIRE(r1 != nullptr);
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(column.size() == Size{150, 40});

    // change spacing
    column.setSpacing(10);
    REQUIRE(column.size() == Size{150, 50});

    // remove second rectangle
    column.takeChildAt(1);
    REQUIRE(column.childCount() == 1);
    REQUIRE(column.size() == Size{150, 30});
}

TEST_CASE("Row layout", "[layouts]")
{
    Row row;
    REQUIRE(row.childCount() == 0);

    // add a rectangle
    {
        auto *r1 = row.appendChild<Rectangle>(80, 20);
        REQUIRE(r1->size() == Size{80, 20});
    }
    REQUIRE(row.childCount() == 1);
    REQUIRE(row.size() == Size{80, 20});

    // add another rectangle
    {
        auto *r2 = row.appendChild<Rectangle>(100, 10);
        REQUIRE(r2->size() == Size{100, 10});
    }
    REQUIRE(row.childCount() == 2);
    REQUIRE(row.size() == Size{180, 20});

    // change size of first rectangle
    auto *r1 = dynamic_cast<Rectangle *>(row.childAt(0));
    REQUIRE(r1 != nullptr);
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(row.size() == Size{250, 30});

    // change spacing
    row.setSpacing(10);
    REQUIRE(row.size() == Size{260, 30});

    // remove second rectangle
    row.takeChildAt(1);
    REQUIRE(row.childCount() == 1);
    REQUIRE(row.size() == Size{150, 30});
}
