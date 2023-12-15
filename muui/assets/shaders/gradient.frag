#version 300 es

precision highp float;

in vec2 vs_position;
out vec4 fragColor;

#include "lineargradient.inc.frag"

void main(void)
{
    fragColor = gradientColor(vs_position);
}
