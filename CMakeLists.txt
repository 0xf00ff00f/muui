cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(muui)

option(MUUI_TESTS "Build tests" ON)

include(CMakeRC)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(muui)

if(MUUI_TESTS)
  add_subdirectory(tests)
endif()
