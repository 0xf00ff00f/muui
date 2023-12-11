#include "system.h"

#include "diskfs.h"
#include "fontcache.h"
#include "log.h"
#include "pixmapcache.h"
#include "resourcefs.h"
#include "shadermanager.h"
#include "textureatlas.h"

CMRC_DECLARE(assets);

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
    , m_diskFS(std::make_unique<DiskFS>())
    , m_resourceFS(std::make_unique<ResourceFS>(cmrc::assets::get_filesystem()))
{
}

System::~System() = default;

VFS *System::diskFS() const
{
    return m_diskFS.get();
}

VFS *System::resourceFS() const
{
    return m_resourceFS.get();
}

} // namespace muui
