add_subdirectory(3rdparty)

add_executable(test
    testwindow.cc
    testwindow.h
    main.cc
)

target_link_libraries(test
PUBLIC
    glfw
    muui
)

target_compile_definitions(test
PRIVATE
    ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
