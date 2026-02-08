macro(AddDemo)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCES)

  cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})

  add_executable(${DEMO_NAME} ${DEMO_SOURCES})
  target_link_libraries(${DEMO_NAME} PUBLIC muui)
  target_compile_definitions(
    ${DEMO_NAME} PUBLIC ASSETSDIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
  if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set_target_properties(
      ${DEMO_NAME} PROPERTIES COMPILE_FLAGS "-sUSE_SDL=2"
                              LINK_FLAGS "-sUSE_SDL=2 -sFULL_ES3=1")
  endif()
endmacro()

adddemo(NAME test-spritebatcher SOURCES test-spritebatcher.cc)
adddemo(NAME test-painter SOURCES test-painter.cc)
adddemo(NAME test-container SOURCES test-container.cc)
adddemo(NAME test-leaderboard SOURCES test-leaderboard.cc)
adddemo(NAME test-effect SOURCES test-effect.cc)
adddemo(NAME test-gradient SOURCES test-gradient.cc)
adddemo(NAME test-dropshadow SOURCES test-dropshadow.cc)
adddemo(NAME test-item-gradient SOURCES test-item-gradient.cc)
adddemo(NAME test-scrollarea SOURCES test-scrollarea.cc)
adddemo(NAME test-item-rotation SOURCES test-item-rotation.cc)
