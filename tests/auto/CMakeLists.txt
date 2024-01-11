add_executable(test-layouts test-layouts.cc)
target_link_libraries(test-layouts muui Catch2::Catch2WithMain)

add_executable(test-anchors test-anchors.cc)
target_link_libraries(test-anchors muui Catch2::Catch2WithMain)
