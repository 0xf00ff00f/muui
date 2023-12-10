#pragma once

#include <glm/glm.hpp>

struct Palette
{
    glm::vec3 background;
    glm::vec3 text;
    glm::vec3 heading;
    glm::vec3 title;
    glm::vec3 darkText;
    glm::vec3 shape;
};

extern Palette g_palette;

void initializePalette();
