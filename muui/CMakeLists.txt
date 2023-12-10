add_subdirectory(muslots)
add_subdirectory(3rdparty)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
endif()

add_library(muui STATIC)

target_sources(${PROJECT_NAME}
PRIVATE
    abstracttexture.h
    assetpath.cc
    assetpath.h
    buffer.cc
    buffer.h
    fontcache.cc
    fontcache.h
    font.cc
    font.h
    gl.h
    ioutil.cc
    ioutil.h
    lazytexture.cc
    lazytexture.h
    log.h
    miniui.cc
    miniui.h
    noncopyable.h
    painter.cc
    painter.h
    palette.cc
    palette.h
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
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    muslots
    glm
    stb
    fmt::fmt
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(${PROJECT_NAME}
    PUBLIC
        android
        log
        EGL
        GLESv3
    )
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ASSETSDIR=""
    )
else()
    target_link_libraries(${PROJECT_NAME}
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
    )
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    )
endif()

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
