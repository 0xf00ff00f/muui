#pragma once

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include <glad/gles2.h>
#endif
