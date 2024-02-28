#include "system.h"

#include "fontcache.h"
#include "pixmapcache.h"
#include "shadermanager.h"
#include "textureatlas.h"

#include <SDL.h>

namespace muui::sys
{

namespace
{

struct System
{
    System()
        : m_shaderManager(std::make_unique<ShaderManager>())
        , m_textureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize))
        , m_fontCache(std::make_unique<FontCache>(m_textureAtlas.get()))
        , m_pixmapCache(std::make_unique<PixmapCache>(m_textureAtlas.get()))
    {
    }

    ShaderManager *shaderManager() { return m_shaderManager.get(); }
    FontCache *fontCache() { return m_fontCache.get(); }
    PixmapCache *pixmapCache() { return m_pixmapCache.get(); }

    static constexpr auto TextureAtlasPageSize = 1024;

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<TextureAtlas> m_textureAtlas;
    std::unique_ptr<FontCache> m_fontCache;
    std::unique_ptr<PixmapCache> m_pixmapCache;
} *s_system = nullptr;

} // namespace

bool initialize()
{
    assert(s_system == nullptr);
    s_system = new System;
    return true;
}

void shutdown()
{
    assert(s_system != nullptr);
    delete s_system;
    s_system = nullptr;
}

ShaderManager *shaderManager()
{
    return s_system->shaderManager();
}

FontCache *fontCache()
{
    return s_system->fontCache();
}

PixmapCache *pixmapCache()
{
    return s_system->pixmapCache();
}

} // namespace muui::sys
