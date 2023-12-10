#include "palette.h"

namespace
{

glm::vec3 rgbToColor(unsigned color)
{
    const auto r = static_cast<float>((color >> 16) & 0xff);
    const auto g = static_cast<float>((color >> 8) & 0xff);
    const auto b = static_cast<float>(color & 0xff);
    return (1.0f / 255.0f) * glm::vec3(r, g, b);
}

} // namespace

Palette g_palette;

void initializePalette()
{
    g_palette.background = glm::vec3(0.75);
    g_palette.text = rgbToColor(0x04568e);
    g_palette.heading = rgbToColor(0xfdfeff);
    g_palette.title = rgbToColor(0xe00000);
    g_palette.darkText = rgbToColor(0x040a18);
    g_palette.shape = rgbToColor(0x04568e);
}
