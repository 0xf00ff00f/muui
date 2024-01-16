#include <muui/item.h>

#include <catch2/catch_test_macros.hpp>

using namespace muui;

TEST_CASE("Column layout", "[layouts]")
{
    Column column;
    REQUIRE(column.childCount() == 0);

    // add a rectangle
    auto *r1 = column.appendChild<Rectangle>(80, 20);
    REQUIRE(r1->size() == Size{80, 20});
    REQUIRE(column.childCount() == 1);
    REQUIRE(column.size() == Size{80, 20});

    // add another rectangle
    auto *r2 = column.appendChild<Rectangle>(100, 10);
    REQUIRE(r2->size() == Size{100, 10});
    REQUIRE(column.childCount() == 2);
    REQUIRE(column.size() == Size{100, 30});

    // change size of first rectangle
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(column.size() == Size{150, 40});

    // change spacing
    column.setSpacing(10);
    REQUIRE(column.size() == Size{150, 50});

    // change alignment of second rectangle
    REQUIRE(column.childAt(1) == r2);
    REQUIRE(r2->containerAlignment() == (Alignment::VCenter | Alignment::Left));
    REQUIRE(column.childRect(1).min == glm::vec2{0.0f, 40.0f});

    r2->setContainerAlignment(Alignment::VCenter | Alignment::HCenter);
    REQUIRE(column.childRect(1).min == glm::vec2{25.0f, 40.0f});

    r2->setContainerAlignment(Alignment::VCenter | Alignment::Right);
    REQUIRE(column.childRect(1).min == glm::vec2{50.0f, 40.0f});

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
    auto *r1 = row.appendChild<Rectangle>(80, 20);
    REQUIRE(r1->size() == Size{80, 20});
    REQUIRE(row.childCount() == 1);
    REQUIRE(row.size() == Size{80, 20});

    // add another rectangle
    auto *r2 = row.appendChild<Rectangle>(100, 10);
    REQUIRE(r2->size() == Size{100, 10});
    REQUIRE(row.childCount() == 2);
    REQUIRE(row.size() == Size{180, 20});

    // change size of first rectangle
    r1->setSize(150, 30);
    REQUIRE(r1->size() == Size{150, 30});
    REQUIRE(row.size() == Size{250, 30});

    // change spacing
    row.setSpacing(10);
    REQUIRE(row.size() == Size{260, 30});

    // change alignment of second rectangle
    REQUIRE(row.childAt(1) == r2);
    REQUIRE(r2->containerAlignment() == (Alignment::VCenter | Alignment::Left));
    REQUIRE(row.childRect(1).min == glm::vec2{160.0f, 10.0f});

    r2->setContainerAlignment(Alignment::Top | Alignment::Left);
    REQUIRE(row.childRect(1).min == glm::vec2{160.0f, 0.0f});

    r2->setContainerAlignment(Alignment::Bottom | Alignment::Left);
    REQUIRE(row.childRect(1).min == glm::vec2{160.0f, 20.0f});

    // remove second rectangle
    row.takeChildAt(1);
    REQUIRE(row.childCount() == 1);
    REQUIRE(row.size() == Size{150, 30});
}
