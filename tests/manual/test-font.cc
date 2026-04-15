#include "panic.h"

#include <muui/application.h>
#include <muui/font.h>

#include <fmt/format.h>

#include <stb_image_write.h>

static const std::filesystem::path AssetsPath{ASSETSDIR};

class FontApplication : public muui::Application
{
public:
    bool initialize() override;
    void resize(int width, int height) override;
    void render() const override;
};

bool FontApplication::initialize()
{
    muui::TextureAtlas textureAtlas{512, 512};

    muui::Font font{&textureAtlas};
    if (!font.load(AssetsPath / "OpenSans_Bold.ttf", 300, 20))
        panic("Failed to load font\n");

    for (size_t i = 0; i < 10; ++i)
        auto *glyph = font.glyph(L'0' + i);

    for (std::size_t i = 0; i < textureAtlas.pageCount(); ++i)
    {
        const auto &page = textureAtlas.page(i);
        const auto &pixmap = page.pixmap();
        assert(pixmap.pixelType == PixelType::RGBA);
        stbi_write_png(fmt::format("page-{}.png", i).c_str(), pixmap.width, pixmap.height, 4, pixmap.pixels.data(),
                       pixmap.width * sizeof(uint32_t));
    }

    return true;
}

void FontApplication::resize(int, int) {}

void FontApplication::render() const {}

int main(int argc, char *argv[])
{
    FontApplication{}.createWindow(400, 400, "hello");
}
