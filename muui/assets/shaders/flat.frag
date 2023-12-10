#version 300 es

precision highp float;

in vec4 vs_color;
out vec4 fragColor;

void main(void)
{
    fragColor = vs_color;
}
