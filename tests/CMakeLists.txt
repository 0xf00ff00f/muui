add_subdirectory(3rdparty)

add_library(test-common STATIC testwindow.cc testwindow.h)
target_link_libraries(test-common PUBLIC glfw muui)
target_compile_definitions(
  test-common PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

add_executable(test-painter test-painter.cc)
target_link_libraries(test-painter PUBLIC test-common)

add_executable(test-container test-container.cc)
target_link_libraries(test-container PUBLIC test-common)

add_executable(test-leaderboard test-leaderboard.cc)
target_link_libraries(test-leaderboard PUBLIC test-common)

add_executable(test-effect test-effect.cc)
target_link_libraries(test-effect PUBLIC test-common)
