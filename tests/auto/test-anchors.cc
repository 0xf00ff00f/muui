#include <muui/item.h>

#include <catch2/catch_test_macros.hpp>

using namespace muui;

TEST_CASE("Anchors", "[anchors]")
{
    Rectangle parent{400, 300};
    REQUIRE(parent.childCount() == 0);

    // add a rectangle
    parent.appendChild(std::make_unique<Rectangle>(100.0f, 50.0f));
    REQUIRE(parent.childCount() == 1);
    REQUIRE(parent.childRect(0).min == glm::vec2{0.0f});

    auto *child = parent.childAt(0);

    child->setLeft(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 0.0f});

    child->setLeft(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{200.0f, 0.0f});

    child->setHorizontalCenter(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{50.0f, 0.0f});

    child->setHorizontalCenter(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{150.0f, 0.0f});

    child->setRight(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{0.0f, 0.0f});

    child->setRight(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 0.0f});

    child->setTop(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 100.0f});

    child->setTop(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 150.0f});

    child->setVerticalCenter(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 75.0f});

    child->setVerticalCenter(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 125.0f});

    child->setBottom(Length::pixels(100.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 50.0f});

    child->setBottom(Length::percent(50.0f));
    REQUIRE(parent.childRect(0).min == glm::vec2{100.0f, 100.0f});
}
