#include <muui/item.h>

#include <catch2/catch_test_macros.hpp>

using namespace muui;

TEST_CASE("Column layout", "[layouts]")
{
    Column column;
    REQUIRE(column.count() == 0);

    // add a rectangle
    {
        auto r1 = std::make_unique<muui::Rectangle>(80, 20);
        REQUIRE(r1->size() == Size{80, 20});
        column.append(std::move(r1));
    }
    REQUIRE(column.count() == 1);
    REQUIRE(column.size() == Size{80, 20});

    // add another rectangle
    {
        auto r2 = std::make_unique<muui::Rectangle>(100, 10);
        REQUIRE(r2->size() == Size{100, 10});
        column.append(std::move(r2));
    }
    REQUIRE(column.count() == 2);
    REQUIRE(column.size() == Size{100, 30});

    // change size of first rectangle
    auto *r1 = dynamic_cast<Rectangle *>(column.at(0));
    REQUIRE(r1 != nullptr);
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(column.size() == Size{150, 40});

    // change spacing
    column.setSpacing(10);
    REQUIRE(column.size() == Size{150, 50});

    // remove second rectangle
    column.takeAt(1);
    REQUIRE(column.count() == 1);
    REQUIRE(column.size() == Size{150, 30});
}

TEST_CASE("Row layout", "[layouts]")
{
    Row row;
    REQUIRE(row.count() == 0);

    // add a rectangle
    {
        auto r1 = std::make_unique<muui::Rectangle>(80, 20);
        REQUIRE(r1->size() == Size{80, 20});
        row.append(std::move(r1));
    }
    REQUIRE(row.count() == 1);
    REQUIRE(row.size() == Size{80, 20});

    // add another rectangle
    {
        auto r2 = std::make_unique<muui::Rectangle>(100, 10);
        REQUIRE(r2->size() == Size{100, 10});
        row.append(std::move(r2));
    }
    REQUIRE(row.count() == 2);
    REQUIRE(row.size() == Size{180, 20});

    // change size of first rectangle
    auto *r1 = dynamic_cast<Rectangle *>(row.at(0));
    REQUIRE(r1 != nullptr);
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(row.size() == Size{250, 30});

    // change spacing
    row.setSpacing(10);
    REQUIRE(row.size() == Size{260, 30});

    // remove second rectangle
    row.takeAt(1);
    REQUIRE(row.count() == 1);
    REQUIRE(row.size() == Size{150, 30});
}
