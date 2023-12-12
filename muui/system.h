#pragma once

#include <memory>

namespace muui
{
class Painter;
class FontCache;
class PixmapCache;
class ShaderManager;
class TextureAtlas;

class System
{
public:
    static System *instance()
    {
        if (!s_instance)
            s_instance = new System();
        return s_instance;
    }

    static bool initialize();
    static void shutdown();

    ShaderManager *shaderManager() const { return m_shaderManager.get(); }
    FontCache *fontCache() const { return m_fontCache.get(); }
    PixmapCache *pixmapCache() const { return m_pixmapCache.get(); }

private:
    System();
    ~System();

    static System *s_instance;

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<TextureAtlas> m_fontTextureAtlas;
    std::unique_ptr<TextureAtlas> m_pixmapTextureAtlas;
    std::unique_ptr<FontCache> m_fontCache;
    std::unique_ptr<PixmapCache> m_pixmapCache;
};

ShaderManager *getShaderManager();
FontCache *getFontCache();
PixmapCache *getPixmapCache();

} // namespace muui
