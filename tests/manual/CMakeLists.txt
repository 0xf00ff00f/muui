add_executable(test-spritebatcher test-spritebatcher.cc)
target_link_libraries(test-spritebatcher PUBLIC muui)
target_compile_definitions(test-spritebatcher PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-painter test-painter.cc)
target_link_libraries(test-painter PUBLIC muui)
target_compile_definitions(test-painter PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-container test-container.cc)
target_link_libraries(test-container PUBLIC muui)
target_compile_definitions(test-container PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-leaderboard test-leaderboard.cc)
target_link_libraries(test-leaderboard PUBLIC muui)
target_compile_definitions(test-leaderboard PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-effect test-effect.cc)
target_link_libraries(test-effect PUBLIC muui)
target_compile_definitions(test-effect PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-gradient test-gradient.cc)
target_link_libraries(test-gradient PUBLIC muui)
target_compile_definitions(test-gradient PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-dropshadow test-dropshadow.cc)
target_link_libraries(test-dropshadow PUBLIC muui)
target_compile_definitions(test-dropshadow PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-item-gradient test-item-gradient.cc)
target_link_libraries(test-item-gradient PUBLIC muui)
target_compile_definitions(test-item-gradient PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-scrollarea test-scrollarea.cc)
target_link_libraries(test-scrollarea PUBLIC muui)
target_compile_definitions(test-scrollarea PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-item-rotation test-item-rotation.cc)
target_link_libraries(test-item-rotation PUBLIC muui)
target_compile_definitions(test-item-rotation PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
