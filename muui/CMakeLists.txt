add_subdirectory(muslots)
add_subdirectory(assets)
add_subdirectory(3rdparty)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
  find_package(OpenGL REQUIRED)
endif()

add_library(muui STATIC)

set(MUUI_SOURCES
    abstracttexture.h
    buffer.cc
    buffer.h
    vertexarray.cc
    vertexarray.h
    fontcache.cc
    fontcache.h
    font.cc
    font.h
    gl.h
    item.cc
    item.h
    lazytexture.cc
    lazytexture.h
    log.h
    noncopyable.h
    painter.cc
    painter.h
    pixeltype.h
    pixmapcache.cc
    pixmapcache.h
    pixmap.cc
    pixmap.h
    screen.cc
    screen.h
    shadermanager.cc
    shadermanager.h
    shaderprogram.cc
    shaderprogram.h
    spritebatcher.cc
    spritebatcher.h
    system.cc
    system.h
    textureatlas.cc
    textureatlas.h
    textureatlaspage.cc
    textureatlaspage.h
    texture.cc
    texture.h
    touchevent.h
    tweening.h
    uiinput.h
    util.h
    valueanimation.h
    vfs.h
    resourcefs.h
    resourcefs.cc
    file.h
    file.cc
    framebuffer.h
    framebuffer.cc
    shadereffect.h
    shadereffect.cc
    gradienttexture.h
    gradienttexture.cc
    mesh.h
    dropshadow.h
    dropshadow.cc)

if(MUUI_USE_SDL2)
  list(APPEND MUUI_SOURCES sdlfs.h sdlfs.cc)
else()
  list(APPEND MUUI_SOURCES diskfs.h diskfs.cc)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${MUUI_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC muslots glm stb fmt::fmt
  PRIVATE cmrc-base embed-assets)

if(MUUI_USE_SDL2)
  target_compile_definitions(${PROJECT_NAME} PUBLIC MUUI_USE_SDL2)
  target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
  target_link_libraries(${PROJECT_NAME} PUBLIC android log EGL GLESv3)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETSDIR="")
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC glew OpenGL::GL)
  target_compile_definitions(
    ${PROJECT_NAME} PRIVATE ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
endif()

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
