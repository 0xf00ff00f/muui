#include "system.h"

#include "log.h"
#include "fontcache.h"
#include "pixmapcache.h"
#include "shadermanager.h"
#include "textureatlas.h"

namespace muui
{

System *System::s_instance = nullptr;

namespace
{
constexpr auto TextureAtlasPageSize = 1024;
}

bool System::initialize()
{
#if !defined(__ANDROID__)
    if (glewInit() != GLEW_OK)
    {
        log_error("Failed to initialize GLEW");
        return false;
    }
#endif
    return true;
}

void System::shutdown()
{
    delete s_instance;
    s_instance = nullptr;
}

System::System()
    : m_shaderManager(std::make_unique<ShaderManager>())
    , m_fontTextureAtlas(
          std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::Grayscale))
    , m_pixmapTextureAtlas(std::make_unique<TextureAtlas>(TextureAtlasPageSize, TextureAtlasPageSize, PixelType::RGBA))
    , m_fontCache(std::make_unique<FontCache>(m_fontTextureAtlas.get()))
    , m_pixmapCache(std::make_unique<PixmapCache>(m_pixmapTextureAtlas.get()))
{
}

System::~System() = default;

} // namespace muui
