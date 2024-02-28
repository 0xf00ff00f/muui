#pragma once

#include "noncopyable.h"
#include "pixeltype.h"
#include "pixmap.h"
#include "util.h"

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace muui
{

class TextureAtlasPage : private NonCopyable
{
public:
    TextureAtlasPage(int width, int height, PixelType pixelType);
    ~TextureAtlasPage();

    PixelType pixelType() const { return m_pixmap.pixelType; }
    const Pixmap &pixmap() const { return m_pixmap; }

    std::optional<RectF> insert(const Pixmap &pixmap);

private:
    Pixmap m_pixmap;
    struct Node;
    std::unique_ptr<Node> m_tree;
};

} // namespace muui
