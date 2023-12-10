project(assets)

set(ASSET_FILES
    shaders/circle.frag
    shaders/circle.vert
    shaders/flat.frag
    shaders/flat.vert
    shaders/roundedrect.frag
    shaders/roundedrect.vert
    shaders/text.frag
    shaders/text.vert
)

cmrc_add_resource_library(
    embed-assets
    NAMESPACE
    assets
    WHENCE
    ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX
    assets
    ${ASSET_FILES})
