add_subdirectory(muslots)
add_subdirectory(assets)
add_subdirectory(3rdparty)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
  find_package(OpenGL REQUIRED)
endif()

add_library(muui STATIC)

target_sources(
  ${PROJECT_NAME}
  PRIVATE abstracttexture.h
          buffer.cc
          buffer.h
          fontcache.cc
          fontcache.h
          font.cc
          font.h
          gl.h
          ioutil.cc
          ioutil.h
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
          diskfs.h
          diskfs.cc
          resourcefs.h
          resourcefs.cc)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC muslots glm stb fmt::fmt
  PRIVATE cmrc-base embed-assets)

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
