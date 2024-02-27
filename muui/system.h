#pragma once

namespace muui
{
class FontCache;
class PixmapCache;
class ShaderManager;
} // namespace muui

namespace muui::sys
{

ShaderManager *shaderManager();
FontCache *fontCache();
PixmapCache *pixmapCache();

bool initialize();
void shutdown();

} // namespace muui::sys
