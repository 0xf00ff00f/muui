#include "panic.h"

#include <muui/application.h>
#include <muui/font.h>
#include <muui/item.h>
#include <muui/painter.h>
#include <muui/textureatlas.h>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class LayoutTest : public muui::Application
{
protected:
    bool initialize() override;
    void resize(int width, int height) override;
    void render() const override;

private:
    std::unique_ptr<muui::Painter> m_painter;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Item> m_rootItem;
};

bool LayoutTest::initialize()
{
    m_painter = std::make_unique<muui::Painter>();

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(ASSETSDIR "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    auto addLabel = [this](muui::Item *parent, std::u32string_view text, muui::Alignment alignment) {
        auto *label = parent->appendChild<muui::Label>(m_font.get(), text);
        label->setContainerAlignment(alignment);
        label->fillBackground = true;
        label->backgroundBrush = glm::vec4{1, 0, 0, 1};
        label->brush = glm::vec4{1, 1, 1, 1};
    };

    auto root = std::make_unique<muui::Column>();
    root->setSpacing(8);

    {
        auto *column = root->appendChild<muui::Column>();
        column->setMinimumWidth(400);
        column->shape = muui::Item::Shape::RoundedRectangle;
        column->fillBackground = true;
        column->backgroundBrush = glm::vec4{0.75, 0.75, 0.75, 1};
        column->setMargins(muui::Margins{8, 8, 8, 8});
        column->cornerRadius = 8.0f;
        column->setSpacing(4);
        addLabel(column, U"label 1"sv, muui::Alignment::VCenter | muui::Alignment::Left);
        addLabel(column, U"label 2"sv, muui::Alignment::VCenter | muui::Alignment::HCenter);
        addLabel(column, U"label 3"sv, muui::Alignment::VCenter | muui::Alignment::Right);
    }

    {
        auto *row = root->appendChild<muui::Row>();
        row->setMinimumHeight(150);
        row->shape = muui::Item::Shape::RoundedRectangle;
        row->fillBackground = true;
        row->backgroundBrush = glm::vec4{0.75, 0.75, 0.75, 1};
        row->setMargins(muui::Margins{8, 8, 8, 8});
        row->cornerRadius = 8.0f;
        row->setSpacing(4);
        addLabel(row, U"label 1"sv, muui::Alignment::Top | muui::Alignment::Left);
        addLabel(row, U"label 2"sv, muui::Alignment::VCenter | muui::Alignment::Left);
        addLabel(row, U"label 3"sv, muui::Alignment::Bottom | muui::Alignment::Left);
    }

    m_rootItem = std::move(root);

    return true;
}

void LayoutTest::resize(int width, int height)
{
    m_painter->setWindowSize(width, height);
}

void LayoutTest::render() const
{
    glClearColor(0.25, 0.25, 0.25, 1);
    glViewport(0, 0, m_painter->windowWidth(), m_painter->windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    m_painter->begin();
    m_rootItem->render(m_painter.get(), {8, 8});
    m_painter->end();
}

int main(int argc, char *argv[])
{
    LayoutTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
